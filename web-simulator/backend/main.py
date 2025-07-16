#!/usr/bin/env python3
"""
MELKENS Robot Web Simulator Backend
Hardware-in-the-loop (HIL) Integration with IMU Board
"""

import asyncio
import json
import logging
import os
import time
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Union

import structlog
import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse
from pydantic import BaseModel

from imu_manager import IMUManager
from data_logger import DataLogger
from robot_simulator import RobotSimulator

# Configure logging
structlog.configure(
    processors=[
        structlog.stdlib.filter_by_level,
        structlog.stdlib.add_logger_name,
        structlog.stdlib.add_log_level,
        structlog.stdlib.PositionalArgumentsFormatter(),
        structlog.processors.TimeStamper(fmt="iso"),
        structlog.processors.StackInfoRenderer(),
        structlog.processors.format_exc_info,
        structlog.processors.UnicodeDecoder(),
        structlog.processors.JSONRenderer()
    ],
    context_class=dict,
    logger_factory=structlog.stdlib.LoggerFactory(),
    wrapper_class=structlog.stdlib.BoundLogger,
    cache_logger_on_first_use=True,
)

logger = structlog.get_logger()

# Create logs directory
os.makedirs("../../logs", exist_ok=True)

app = FastAPI(title="MELKENS Robot HIL Simulator", version="1.0.0")

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Mount static files
app.mount("/static", StaticFiles(directory="../frontend"), name="static")

# Global state
imu_manager = None
data_logger = None
robot_simulator = None
connected_websockets: List[WebSocket] = []

# Pydantic models
class SystemConfig(BaseModel):
    imu_mode: str = "simulated"  # "simulated" or "hardware"
    serial_port: str = "/dev/ttyUSB0"
    baud_rate: int = 115200
    update_rate_ms: int = 50

class IMUData(BaseModel):
    timestamp: float
    accelerometer: Dict[str, float]
    gyroscope: Dict[str, float]
    magnetometer: Dict[str, float]
    quaternion: Dict[str, float]
    euler_angles: Dict[str, float]
    position: Dict[str, float]
    hardware_connected: bool
    error_message: Optional[str] = None

class FaultInjectionConfig(BaseModel):
    enabled: bool = False
    fault_type: str = "none"  # "drift", "stuck_axis", "missing_packets", "noise"
    axis: str = "x"  # "x", "y", "z"
    severity: float = 1.0
    duration_ms: int = 1000

# Global configuration
system_config = SystemConfig()
fault_config = FaultInjectionConfig()

@app.on_event("startup")
async def startup():
    """Initialize the HIL system on startup"""
    global imu_manager, data_logger, robot_simulator
    
    logger.info("Starting MELKENS HIL Simulator...")
    
    # Initialize components
    imu_manager = IMUManager(system_config.serial_port, system_config.baud_rate)
    data_logger = DataLogger("../../logs/hil_imu_test.csv")
    robot_simulator = RobotSimulator()
    
    # Start IMU manager
    await imu_manager.start()
    
    # Perform self-test
    test_result = await imu_manager.self_test()
    if test_result["success"]:
        logger.info("IMU self-test passed", details=test_result)
    else:
        logger.warning("IMU self-test failed", details=test_result)
    
    logger.info("HIL Simulator started successfully")

@app.on_event("shutdown")
async def shutdown():
    """Cleanup on shutdown"""
    global imu_manager, data_logger
    
    logger.info("Shutting down HIL Simulator...")
    
    if imu_manager:
        await imu_manager.stop()
    
    if data_logger:
        data_logger.close()
    
    logger.info("HIL Simulator stopped")

@app.get("/", response_class=HTMLResponse)
async def get_index():
    """Serve the main dashboard"""
    with open("../frontend/index.html", "r") as f:
        return HTMLResponse(content=f.read())

@app.get("/api/config")
async def get_config():
    """Get current system configuration"""
    return {
        "system": system_config.dict(),
        "fault_injection": fault_config.dict()
    }

@app.post("/api/config/system")
async def update_system_config(config: SystemConfig):
    """Update system configuration"""
    global system_config, imu_manager
    
    logger.info("Updating system configuration", new_config=config.dict())
    
    # Update configuration
    old_mode = system_config.imu_mode
    system_config = config
    
    # If mode changed, reconfigure IMU manager
    if old_mode != config.imu_mode and imu_manager:
        await imu_manager.set_mode(config.imu_mode)
        if config.imu_mode == "hardware":
            await imu_manager.configure_serial(config.serial_port, config.baud_rate)
    
    return {"status": "success", "config": system_config.dict()}

@app.post("/api/config/fault-injection")
async def update_fault_config(config: FaultInjectionConfig):
    """Update fault injection configuration"""
    global fault_config, imu_manager
    
    logger.info("Updating fault injection configuration", new_config=config.dict())
    
    fault_config = config
    
    if imu_manager:
        await imu_manager.configure_fault_injection(config.dict())
    
    return {"status": "success", "config": fault_config.dict()}

@app.get("/api/imu/status")
async def get_imu_status():
    """Get current IMU status"""
    if not imu_manager:
        raise HTTPException(status_code=500, detail="IMU manager not initialized")
    
    status = await imu_manager.get_status()
    return status

@app.get("/api/imu/data")
async def get_imu_data():
    """Get latest IMU data"""
    if not imu_manager:
        raise HTTPException(status_code=500, detail="IMU manager not initialized")
    
    data = await imu_manager.get_latest_data()
    return data

@app.post("/api/imu/self-test")
async def run_self_test():
    """Run IMU self-test"""
    if not imu_manager:
        raise HTTPException(status_code=500, detail="IMU manager not initialized")
    
    result = await imu_manager.self_test()
    return result

@app.post("/api/robot/route/{route_id}")
async def set_robot_route(route_id: str):
    """Set robot route (A, B, C, D)"""
    if not robot_simulator:
        raise HTTPException(status_code=500, detail="Robot simulator not initialized")
    
    success = robot_simulator.set_route(route_id)
    if success:
        return {"status": "success", "route": route_id}
    else:
        raise HTTPException(status_code=400, detail=f"Invalid route ID: {route_id}")

@app.get("/api/logs")
async def get_logs():
    """Get recent log entries"""
    if not data_logger:
        raise HTTPException(status_code=500, detail="Data logger not initialized")
    
    logs = data_logger.get_recent_logs(limit=100)
    return {"logs": logs}

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket endpoint for real-time data streaming"""
    await websocket.accept()
    connected_websockets.append(websocket)
    
    logger.info("WebSocket connected", client=websocket.client)
    
    try:
        while True:
            # Send real-time IMU data
            if imu_manager:
                imu_data = await imu_manager.get_latest_data()
                await websocket.send_json({
                    "type": "imu_data",
                    "data": imu_data
                })
            
            # Send robot position data
            if robot_simulator:
                robot_data = robot_simulator.get_position()
                await websocket.send_json({
                    "type": "robot_position",
                    "data": robot_data
                })
            
            # Wait for next update cycle
            await asyncio.sleep(system_config.update_rate_ms / 1000.0)
            
    except WebSocketDisconnect:
        logger.info("WebSocket disconnected", client=websocket.client)
        connected_websockets.remove(websocket)
    except Exception as e:
        logger.error("WebSocket error", error=str(e))
        if websocket in connected_websockets:
            connected_websockets.remove(websocket)

async def broadcast_to_websockets(message: dict):
    """Broadcast message to all connected WebSocket clients"""
    if connected_websockets:
        disconnected = []
        for websocket in connected_websockets:
            try:
                await websocket.send_json(message)
            except:
                disconnected.append(websocket)
        
        # Remove disconnected websockets
        for ws in disconnected:
            connected_websockets.remove(ws)

if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8000,
        reload=True,
        log_level="info"
    )