#!/usr/bin/env python3
"""
MELKENS Robot Web Simulator Backend
Hardware-in-the-loop (HIL) Integration with IMU Board
"""

import asyncio
import json
import logging
import os
import os
import time
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Union, Any

import structlog
import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException, File, UploadFile
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse, FileResponse
from pydantic import BaseModel

from imu_manager import IMUManager
from data_logger import DataLogger
from robot_simulator import RobotSimulator

# New import for advanced scenarios
from scenario_runner import AdvancedScenarioRunner, FaultScenario, FaultType, FaultSeverity

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

# Mount static files - handle both local development and Docker deployment
frontend_path = Path(__file__).parent.parent / "frontend"
if not frontend_path.exists():
    # For Docker deployment, frontend is copied to /app/frontend
    frontend_path = Path("/app/frontend")

app.mount("/static", StaticFiles(directory=str(frontend_path)), name="static")

# Global state
imu_manager = IMUManager()
data_logger = DataLogger()
robot_simulator = RobotSimulator()
connected_websockets: List[WebSocket] = []

# New: Advanced scenario runner
scenario_runner = AdvancedScenarioRunner(data_logger=data_logger)

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

# ========================
# Advanced Scenario API Endpoints
# ========================

class ScenarioRequest(BaseModel):
    id: str
    name: str
    description: str
    fault_type: str
    severity: str
    target_sensor: str
    target_axis: str
    start_time: float
    duration: float
    parameters: Dict[str, Any] = {}

class TestRunRequest(BaseModel):
    scenario_id: str
    route_name: str = "A"

@app.post("/api/scenarios/upload")
async def upload_scenarios(file: UploadFile = File(...)):
    """Upload scenario file (JSON or CSV)"""
    try:
        # Save uploaded file temporarily
        temp_file = Path(f"temp_{file.filename}")
        content = await file.read()
        temp_file.write_bytes(content)
        
        # Load scenarios based on file type
        if file.filename.endswith('.json'):
            count = scenario_runner.scenario_manager.load_scenarios_from_json(str(temp_file))
        elif file.filename.endswith('.csv'):
            count = scenario_runner.scenario_manager.load_scenarios_from_csv(str(temp_file))
        else:
            raise HTTPException(status_code=400, detail="Unsupported file type. Use JSON or CSV.")
        
        # Clean up
        temp_file.unlink()
        
        return {
            "status": "success",
            "message": f"Loaded {count} scenarios from {file.filename}",
            "scenarios_loaded": count
        }
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.get("/api/scenarios")
async def list_scenarios():
    """List all available fault injection scenarios"""
    try:
        scenarios = scenario_runner.scenario_manager.list_scenarios()
        return {
            "status": "success",
            "scenarios": scenarios,
            "count": len(scenarios)
        }
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.post("/api/scenarios")
async def create_scenario(scenario_request: ScenarioRequest):
    """Create a new fault injection scenario"""
    try:
        scenario = FaultScenario(
            id=scenario_request.id,
            name=scenario_request.name,
            description=scenario_request.description,
            fault_type=FaultType(scenario_request.fault_type),
            severity=FaultSeverity(scenario_request.severity),
            target_sensor=scenario_request.target_sensor,
            target_axis=scenario_request.target_axis,
            start_time=scenario_request.start_time,
            duration=scenario_request.duration,
            parameters=scenario_request.parameters
        )
        
        scenario_runner.scenario_manager.add_scenario(scenario)
        
        return {
            "status": "success",
            "message": f"Scenario '{scenario.name}' created successfully",
            "scenario": scenario.to_dict()
        }
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.get("/api/scenarios/{scenario_id}")
async def get_scenario(scenario_id: str):
    """Get details of a specific scenario"""
    try:
        scenario = scenario_runner.scenario_manager.get_scenario(scenario_id)
        if not scenario:
            raise HTTPException(status_code=404, detail="Scenario not found")
        
        return {
            "status": "success",
            "scenario": scenario.to_dict()
        }
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.post("/api/scenarios/{scenario_id}/run")
async def run_scenario_test(scenario_id: str, request: TestRunRequest = None):
    """Run a fault injection scenario test"""
    try:
        route_name = request.route_name if request else "A"
        
        # Set the robot route
        robot_simulator.set_route(route_name)
        
        # Run the scenario
        result = await scenario_runner.run_scenario(scenario_id, route_name)
        
        return {
            "status": "success",
            "message": f"Scenario test completed",
            "result": {
                "scenario_id": result.scenario_id,
                "route_completed": result.route_completed,
                "completion_percentage": result.completion_percentage,
                "fault_detected": result.fault_detected,
                "detection_time": result.detection_time,
                "recovery_achieved": result.recovery_achieved,
                "recovery_time": result.recovery_time,
                "performance_metrics": result.performance_metrics,
                "navigation_error_count": len(result.navigation_errors)
            }
        }
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.get("/api/scenarios/results")
async def get_test_results():
    """Get all test results"""
    try:
        results = scenario_runner.scenario_manager.test_results
        
        return {
            "status": "success",
            "results": [
                {
                    "scenario_id": r.scenario_id,
                    "start_time": r.start_time.isoformat(),
                    "end_time": r.end_time.isoformat(),
                    "route_completed": r.route_completed,
                    "completion_percentage": r.completion_percentage,
                    "fault_detected": r.fault_detected,
                    "detection_time": r.detection_time,
                    "recovery_achieved": r.recovery_achieved,
                    "recovery_time": r.recovery_time,
                    "navigation_error_count": len(r.navigation_errors),
                    "performance_metrics": r.performance_metrics
                }
                for r in results
            ],
            "count": len(results)
        }
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.get("/api/scenarios/report")
async def generate_scenario_report(scenario_ids: Optional[str] = None):
    """Generate comprehensive scenario test report"""
    try:
        scenario_id_list = scenario_ids.split(',') if scenario_ids else None
        report = scenario_runner.generate_scenario_report(scenario_id_list)
        
        return {
            "status": "success",
            "report": report
        }
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.get("/api/scenarios/export/csv")
async def export_results_csv():
    """Export test results to CSV file"""
    try:
        output_file = "logs/scenario_results.csv"
        scenario_runner.export_results_csv(output_file)
        
        return FileResponse(
            path=output_file,
            filename="scenario_results.csv",
            media_type="text/csv"
        )
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

@app.get("/api/scenarios/fault-types")
async def get_fault_types():
    """Get available fault types and their descriptions"""
    try:
        fault_types = {
            "stuck_value": {
                "name": "Stuck Value",
                "description": "Sensor value freezes at current reading",
                "parameters": ["stuck_value (optional)"]
            },
            "axis_loss": {
                "name": "Axis Loss",
                "description": "Complete failure of sensor axis",
                "parameters": []
            },
            "bias_drift": {
                "name": "Bias Drift",
                "description": "Gradual bias introduction over time",
                "parameters": ["bias_rate (units/second)"]
            },
            "noise_injection": {
                "name": "Noise Injection",
                "description": "Add random noise to sensor readings",
                "parameters": ["noise_level (standard deviation)"]
            },
            "packet_loss": {
                "name": "Packet Loss",
                "description": "Random loss of data packets",
                "parameters": ["loss_rate (0.0-1.0)"]
            },
            "scale_error": {
                "name": "Scale Error",
                "description": "Incorrect scaling factor applied",
                "parameters": ["scale_factor (multiplier)"]
            },
            "periodic_glitch": {
                "name": "Periodic Glitch",
                "description": "Periodic disturbances in data",
                "parameters": ["frequency (Hz)", "amplitude"]
            },
            "saturation": {
                "name": "Saturation",
                "description": "Values clipped to min/max limits",
                "parameters": ["min_value", "max_value"]
            }
        }
        
        severities = {
            "low": "Minor impact on system performance",
            "medium": "Moderate impact, system should handle gracefully",
            "high": "Significant impact, may cause navigation issues",
            "critical": "Severe impact, system failure expected"
        }
        
        return {
            "status": "success",
            "fault_types": fault_types,
            "severities": severities
        }
    
    except Exception as e:
        return {"status": "error", "message": str(e)}

# ========================
# Enhanced WebSocket for Scenario Monitoring
# ========================

@app.websocket("/ws/scenarios")
async def websocket_scenario_endpoint(websocket: WebSocket):
    """WebSocket endpoint for real-time scenario monitoring"""
    await websocket.accept()
    
    try:
        while True:
            # Send scenario status if test is running
            if scenario_runner.is_running:
                status_data = {
                    "type": "scenario_status",
                    "data": {
                        "test_id": scenario_runner.current_test_id,
                        "is_running": scenario_runner.is_running,
                        "active_faults": len(scenario_runner.scenario_manager.fault_injector.active_faults),
                        "route_progress": scenario_runner.system_monitor.route_progress,
                        "fault_detections": len(scenario_runner.system_monitor.fault_detections),
                        "navigation_errors": len(scenario_runner.system_monitor.navigation_errors),
                        "performance_metrics": scenario_runner.system_monitor.get_performance_metrics()
                    }
                }
                await websocket.send_text(json.dumps(status_data))
            
            await asyncio.sleep(0.1)  # 10Hz update rate
    
    except Exception as e:
        print(f"WebSocket error: {e}")

# ========================
# Integration with existing IMU processing
# ========================

async def process_imu_data_with_faults():
    """Enhanced IMU data processing with fault injection"""
    while True:
        try:
            # Get current IMU data
            imu_data = imu_manager.get_current_data()
            
            # Apply fault injection if scenario is running
            if scenario_runner.is_running:
                # Apply faults to IMU data
                modified_data = scenario_runner.scenario_manager.fault_injector.apply_faults(imu_data)
                
                # Check for packet loss
                if not scenario_runner.scenario_manager.fault_injector.should_drop_packet():
                    # Update system monitor
                    scenario_runner.system_monitor.update_imu_state(
                        modified_data, 
                        scenario_runner.scenario_manager.fault_injector
                    )
                    
                    # Use modified data for robot simulation
                    robot_simulator.update_imu_data(modified_data)
                else:
                    # Skip this update due to packet loss
                    pass
            else:
                # Normal operation without faults
                robot_simulator.update_imu_data(imu_data)
            
            # Update scenario monitor with robot state
            if scenario_runner.is_running:
                robot_data = {
                    'x': robot_simulator.position[0],
                    'y': robot_simulator.position[1],
                    'step_progress': robot_simulator.get_route_progress(),
                    'status': 'running'
                }
                scenario_runner.system_monitor.update_robot_state(robot_data)
            
            await asyncio.sleep(0.02)  # 50Hz
            
        except Exception as e:
            print(f"Error in enhanced IMU processing: {e}")
            await asyncio.sleep(0.1)

# Root endpoint - serve main frontend page
@app.get("/")
async def read_root():
    """Serve the main frontend page"""
    try:
        frontend_file = frontend_path / "index.html"
        if frontend_file.exists():
            with open(frontend_file, 'r', encoding='utf-8') as f:
                content = f.read()
            return HTMLResponse(content=content)
        else:
            return HTMLResponse(content="<h1>MELKENS HIL Simulator</h1><p>Frontend not found</p>", status_code=404)
    except Exception as e:
        return HTMLResponse(content=f"<h1>Error</h1><p>{str(e)}</p>", status_code=500)

# Health check endpoint for Railway/Docker
@app.get("/api/health")
async def health_check():
    """Health check endpoint for deployment monitoring"""
    return {
        "status": "healthy",
        "timestamp": datetime.now().isoformat(),
        "service": "MELKENS HIL Simulator",
        "version": "1.0"
    }

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8000))
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=port,
        reload=False,  # Disable reload in production
        log_level="info"
    )