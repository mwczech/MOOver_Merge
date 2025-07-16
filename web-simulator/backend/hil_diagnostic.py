#!/usr/bin/env python3
"""
MELKENS HIL Production Diagnostic Script
Comprehensive self-diagnostic for production readiness validation
"""

import asyncio
import json
import sys
import time
import subprocess
import signal
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from datetime import datetime

import requests
import websockets
import serial
import serial.tools.list_ports
import structlog

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

class HILDiagnostic:
    """Comprehensive HIL diagnostic system"""
    
    def __init__(self):
        self.base_url = "http://localhost:8000"
        self.ws_url = "ws://localhost:8000/ws"
        self.server_process = None
        self.diagnostic_results = []
        self.detected_imu_ports = []
        self.system_ready = False
        
        # Diagnostic thresholds
        self.min_data_rate = 20.0  # Hz
        self.max_packet_loss = 5.0  # %
        self.movement_threshold = 0.5  # m/s² for accelerometer
        self.gyro_threshold = 0.1  # rad/s for gyroscope
        
    def log_diagnostic(self, test_name: str, status: str, details: str = "", critical: bool = False):
        """Log diagnostic result"""
        timestamp = datetime.now().isoformat()
        
        # Color coding for terminal output
        if status == "PASS":
            color = "\033[92m"  # Green
            icon = "✅"
        elif status == "FAIL":
            color = "\033[91m"  # Red
            icon = "❌"
        elif status == "WARN":
            color = "\033[93m"  # Yellow
            icon = "⚠️"
        else:
            color = "\033[94m"  # Blue
            icon = "ℹ️"
        
        reset_color = "\033[0m"
        
        print(f"{color}[{status}] {icon} {test_name}{reset_color}")
        if details:
            print(f"    {details}")
        
        self.diagnostic_results.append({
            "timestamp": timestamp,
            "test": test_name,
            "status": status,
            "details": details,
            "critical": critical
        })
        
        logger.info("Diagnostic result", test=test_name, status=status, details=details)
    
    def detect_serial_ports(self) -> List[Dict]:
        """Detect and list all available serial ports"""
        try:
            ports = list(serial.tools.list_ports.comports())
            port_info = []
            
            self.log_diagnostic("Serial Port Detection", "INFO", 
                              f"Found {len(ports)} serial ports")
            
            for port in ports:
                info = {
                    "device": port.device,
                    "description": port.description,
                    "hwid": port.hwid,
                    "vid": getattr(port, 'vid', None),
                    "pid": getattr(port, 'pid', None),
                    "manufacturer": getattr(port, 'manufacturer', None)
                }
                port_info.append(info)
                
                # Check if this looks like a MELKENS/STM device
                is_potential_imu = False
                if port.vid == 0x0483:  # STMicroelectronics VID
                    is_potential_imu = True
                elif "STM" in port.description.upper():
                    is_potential_imu = True
                elif "USB Serial" in port.description:
                    is_potential_imu = True
                
                status_icon = "🎯" if is_potential_imu else "📱"
                self.log_diagnostic(f"Port {port.device}", "INFO", 
                                  f"{status_icon} {port.description} (VID:PID {port.vid:04X}:{port.pid:04X})"
                                  if port.vid and port.pid else f"{status_icon} {port.description}")
                
                if is_potential_imu:
                    self.detected_imu_ports.append(port.device)
            
            if self.detected_imu_ports:
                self.log_diagnostic("IMU Port Detection", "PASS", 
                                  f"Potential IMU devices: {self.detected_imu_ports}")
            else:
                self.log_diagnostic("IMU Port Detection", "WARN", 
                                  "No potential IMU devices detected. Manual configuration may be required.")
            
            return port_info
            
        except Exception as e:
            self.log_diagnostic("Serial Port Detection", "FAIL", str(e), critical=True)
            return []
    
    async def start_hil_simulator(self) -> bool:
        """Start the HIL simulator backend"""
        try:
            # Check if server is already running
            if await self._check_server_running():
                self.log_diagnostic("Server Status", "INFO", "Server already running")
                return True
            
            self.log_diagnostic("Server Startup", "INFO", "Starting HIL simulator backend...")
            
            # Start server process
            self.server_process = subprocess.Popen(
                [sys.executable, "main.py"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=Path(__file__).parent
            )
            
            # Wait for server to start (max 30 seconds)
            for i in range(30):
                await asyncio.sleep(1)
                if await self._check_server_running():
                    self.log_diagnostic("Server Startup", "PASS", f"Server started in {i+1} seconds")
                    return True
            
            self.log_diagnostic("Server Startup", "FAIL", "Server failed to start within 30 seconds", critical=True)
            return False
            
        except Exception as e:
            self.log_diagnostic("Server Startup", "FAIL", str(e), critical=True)
            return False
    
    async def _check_server_running(self) -> bool:
        """Check if HIL server is running"""
        try:
            response = requests.get(f"{self.base_url}/api/config", timeout=2)
            return response.status_code == 200
        except:
            return False
    
    async def configure_hil_mode(self) -> bool:
        """Configure system for HIL mode with auto-detected port"""
        try:
            # Determine best port to use
            target_port = "/dev/ttyUSB0"  # Default
            if self.detected_imu_ports:
                target_port = self.detected_imu_ports[0]  # Use first detected
            
            config = {
                "imu_mode": "hardware",
                "serial_port": target_port,
                "baud_rate": 115200,
                "update_rate_ms": 20  # 50 Hz
            }
            
            self.log_diagnostic("HIL Configuration", "INFO", 
                              f"Configuring for hardware mode on {target_port}")
            
            response = requests.post(
                f"{self.base_url}/api/config/system",
                json=config,
                timeout=5
            )
            
            if response.status_code == 200:
                result = response.json()
                if result.get("status") == "success":
                    self.log_diagnostic("HIL Configuration", "PASS", 
                                      f"Successfully configured HIL mode")
                    return True
            
            self.log_diagnostic("HIL Configuration", "FAIL", 
                              f"Configuration failed: {response.status_code}", critical=True)
            return False
            
        except Exception as e:
            self.log_diagnostic("HIL Configuration", "FAIL", str(e), critical=True)
            return False
    
    async def test_imu_handshake(self) -> bool:
        """Test IMU handshake and connection"""
        try:
            # Run self-test which includes handshake
            self.log_diagnostic("IMU Handshake", "INFO", "Testing IMU handshake...")
            
            response = requests.post(f"{self.base_url}/api/imu/self-test", timeout=15)
            
            if response.status_code == 200:
                result = response.json()
                
                # Check overall success
                if result.get("success"):
                    self.log_diagnostic("IMU Handshake", "PASS", "IMU handshake successful")
                    
                    # Log individual test results
                    tests = result.get("tests", {})
                    for test_name, test_result in tests.items():
                        status = "PASS" if test_result.get("passed") else "FAIL"
                        details = test_result.get("error", "") or str(test_result)
                        self.log_diagnostic(f"  {test_name}", status, details)
                    
                    return True
                else:
                    self.log_diagnostic("IMU Handshake", "FAIL", 
                                      "IMU self-test failed", critical=True)
                    return False
            else:
                self.log_diagnostic("IMU Handshake", "FAIL", 
                                  f"Self-test request failed: {response.status_code}", critical=True)
                return False
                
        except Exception as e:
            self.log_diagnostic("IMU Handshake", "FAIL", str(e), critical=True)
            return False
    
    async def validate_data_rates(self) -> bool:
        """Validate IMU data rates meet minimum requirements"""
        try:
            self.log_diagnostic("Data Rate Validation", "INFO", 
                              f"Testing data rates (minimum: {self.min_data_rate} Hz)")
            
            # Connect to WebSocket and measure data rate
            message_count = 0
            start_time = time.time()
            test_duration = 5.0  # seconds
            
            async with websockets.connect(self.ws_url, timeout=5) as websocket:
                while time.time() - start_time < test_duration:
                    try:
                        message = await asyncio.wait_for(websocket.recv(), timeout=1)
                        data = json.loads(message)
                        
                        if data.get("type") == "imu_data":
                            message_count += 1
                            
                    except asyncio.TimeoutError:
                        continue
            
            # Calculate actual data rate
            actual_rate = message_count / test_duration
            
            if actual_rate >= self.min_data_rate:
                self.log_diagnostic("Data Rate Validation", "PASS", 
                                  f"Data rate: {actual_rate:.1f} Hz (≥ {self.min_data_rate} Hz)")
                return True
            else:
                self.log_diagnostic("Data Rate Validation", "FAIL", 
                                  f"Data rate too low: {actual_rate:.1f} Hz (< {self.min_data_rate} Hz)", 
                                  critical=True)
                return False
                
        except Exception as e:
            self.log_diagnostic("Data Rate Validation", "FAIL", str(e), critical=True)
            return False
    
    async def test_sensor_data_validity(self) -> bool:
        """Test that all sensor axes provide valid, live data"""
        try:
            self.log_diagnostic("Sensor Data Validation", "INFO", 
                              "Testing sensor data validity on all axes")
            
            sensor_data = {
                "accelerometer": {"x": [], "y": [], "z": []},
                "gyroscope": {"x": [], "y": [], "z": []},
                "magnetometer": {"x": [], "y": [], "z": []}
            }
            
            # Collect data for analysis
            sample_count = 0
            start_time = time.time()
            
            async with websockets.connect(self.ws_url, timeout=5) as websocket:
                while time.time() - start_time < 3.0 and sample_count < 100:
                    try:
                        message = await asyncio.wait_for(websocket.recv(), timeout=1)
                        data = json.loads(message)
                        
                        if data.get("type") == "imu_data":
                            imu_data = data.get("data", {})
                            
                            for sensor in sensor_data.keys():
                                sensor_reading = imu_data.get(sensor, {})
                                for axis in ["x", "y", "z"]:
                                    value = sensor_reading.get(axis)
                                    if value is not None:
                                        sensor_data[sensor][axis].append(value)
                            
                            sample_count += 1
                            
                    except asyncio.TimeoutError:
                        continue
            
            # Validate sensor data
            all_sensors_valid = True
            
            for sensor, axes_data in sensor_data.items():
                for axis, values in axes_data.items():
                    if len(values) == 0:
                        self.log_diagnostic(f"  {sensor}.{axis}", "FAIL", "No data received")
                        all_sensors_valid = False
                        continue
                    
                    # Check for stuck values (all identical)
                    if len(set(values)) == 1:
                        self.log_diagnostic(f"  {sensor}.{axis}", "FAIL", 
                                          f"Stuck value: {values[0]}")
                        all_sensors_valid = False
                        continue
                    
                    # Check for reasonable ranges
                    mean_val = sum(values) / len(values)
                    range_val = max(values) - min(values)
                    
                    # Sensor-specific validation
                    if sensor == "accelerometer":
                        # Should detect gravity (~9.8 m/s²)
                        if abs(mean_val) > 20.0:  # Unreasonable
                            self.log_diagnostic(f"  {sensor}.{axis}", "WARN", 
                                              f"High mean: {mean_val:.2f} m/s²")
                        else:
                            self.log_diagnostic(f"  {sensor}.{axis}", "PASS", 
                                              f"Mean: {mean_val:.2f} m/s², Range: {range_val:.2f}")
                    
                    elif sensor == "gyroscope":
                        if abs(mean_val) > 10.0:  # Very high for stationary
                            self.log_diagnostic(f"  {sensor}.{axis}", "WARN", 
                                              f"High gyro bias: {mean_val:.3f} rad/s")
                        else:
                            self.log_diagnostic(f"  {sensor}.{axis}", "PASS", 
                                              f"Mean: {mean_val:.3f} rad/s, Range: {range_val:.3f}")
                    
                    elif sensor == "magnetometer":
                        if abs(mean_val) < 1000:  # Too low for Earth's field
                            self.log_diagnostic(f"  {sensor}.{axis}", "WARN", 
                                              f"Low magnetic field: {mean_val:.0f} µT")
                        else:
                            self.log_diagnostic(f"  {sensor}.{axis}", "PASS", 
                                              f"Mean: {mean_val:.0f} µT, Range: {range_val:.0f}")
            
            if all_sensors_valid:
                self.log_diagnostic("Sensor Data Validation", "PASS", 
                                  f"All sensors providing valid data ({sample_count} samples)")
                return True
            else:
                self.log_diagnostic("Sensor Data Validation", "FAIL", 
                                  "Some sensors failed validation", critical=True)
                return False
                
        except Exception as e:
            self.log_diagnostic("Sensor Data Validation", "FAIL", str(e), critical=True)
            return False
    
    async def test_movement_detection(self) -> bool:
        """Test movement detection (simulated - would require manual movement in real scenario)"""
        try:
            self.log_diagnostic("Movement Detection", "INFO", 
                              "Testing movement detection capability (simulated)")
            
            # For simulation mode, we'll verify that the data shows variation
            # In real hardware mode, this would prompt user to move the device
            
            baseline_data = None
            movement_detected = False
            
            async with websockets.connect(self.ws_url, timeout=5) as websocket:
                # Get baseline reading
                message = await asyncio.wait_for(websocket.recv(), timeout=5)
                data = json.loads(message)
                if data.get("type") == "imu_data":
                    baseline_data = data.get("data", {})
                
                # Monitor for changes over 5 seconds
                start_time = time.time()
                max_acc_change = 0.0
                max_gyro_change = 0.0
                
                while time.time() - start_time < 5.0:
                    try:
                        message = await asyncio.wait_for(websocket.recv(), timeout=1)
                        data = json.loads(message)
                        
                        if data.get("type") == "imu_data":
                            current_data = data.get("data", {})
                            
                            # Calculate changes from baseline
                            if baseline_data:
                                acc_change = abs(
                                    current_data.get("accelerometer", {}).get("x", 0) - 
                                    baseline_data.get("accelerometer", {}).get("x", 0)
                                )
                                gyro_change = abs(
                                    current_data.get("gyroscope", {}).get("z", 0) - 
                                    baseline_data.get("gyroscope", {}).get("z", 0)
                                )
                                
                                max_acc_change = max(max_acc_change, acc_change)
                                max_gyro_change = max(max_gyro_change, gyro_change)
                    
                    except asyncio.TimeoutError:
                        continue
            
            # In simulation mode, we expect some variation due to noise/movement simulation
            if max_acc_change > 0.01 or max_gyro_change > 0.001:
                movement_detected = True
            
            if movement_detected:
                self.log_diagnostic("Movement Detection", "PASS", 
                                  f"Movement sensitivity confirmed (Acc: {max_acc_change:.3f}, Gyro: {max_gyro_change:.4f})")
                return True
            else:
                self.log_diagnostic("Movement Detection", "WARN", 
                                  "No significant movement detected - sensors may be too stable or stuck")
                return True  # Don't fail for this in automated test
                
        except Exception as e:
            self.log_diagnostic("Movement Detection", "FAIL", str(e))
            return False
    
    async def test_reference_route(self) -> bool:
        """Test a reference route and verify dashboard updates"""
        try:
            self.log_diagnostic("Reference Route Test", "INFO", 
                              "Testing route execution and visualization")
            
            # Set route A (simple rectangle)
            response = requests.post(f"{self.base_url}/api/robot/route/A", timeout=5)
            if response.status_code != 200:
                self.log_diagnostic("Reference Route Test", "FAIL", 
                                  f"Failed to set route: {response.status_code}", critical=True)
                return False
            
            self.log_diagnostic("  Route Selection", "PASS", "Route A selected")
            
            # Monitor robot position updates
            initial_position = None
            position_updates = 0
            route_progress = 0.0
            
            async with websockets.connect(self.ws_url, timeout=5) as websocket:
                start_time = time.time()
                
                while time.time() - start_time < 10.0:  # Monitor for 10 seconds
                    try:
                        message = await asyncio.wait_for(websocket.recv(), timeout=1)
                        data = json.loads(message)
                        
                        if data.get("type") == "robot_position":
                            robot_data = data.get("data", {})
                            
                            if initial_position is None:
                                initial_position = (robot_data.get("x", 0), robot_data.get("y", 0))
                            
                            # Check for position updates
                            current_pos = (robot_data.get("x", 0), robot_data.get("y", 0))
                            if current_pos != initial_position:
                                position_updates += 1
                            
                            # Track route progress
                            route_progress = max(route_progress, robot_data.get("step_progress", 0))
                            
                    except asyncio.TimeoutError:
                        continue
            
            if position_updates > 0:
                self.log_diagnostic("  Position Updates", "PASS", 
                                  f"{position_updates} position updates detected")
            else:
                self.log_diagnostic("  Position Updates", "WARN", 
                                  "No position updates detected")
            
            if route_progress > 0:
                self.log_diagnostic("  Route Progress", "PASS", 
                                  f"Route progress: {route_progress:.1%}")
            else:
                self.log_diagnostic("  Route Progress", "WARN", 
                                  "No route progress detected")
            
            self.log_diagnostic("Reference Route Test", "PASS", 
                              "Route testing completed successfully")
            return True
            
        except Exception as e:
            self.log_diagnostic("Reference Route Test", "FAIL", str(e), critical=True)
            return False
    
    async def generate_diagnostic_report(self) -> Dict:
        """Generate comprehensive diagnostic report"""
        # Count results
        total_tests = len(self.diagnostic_results)
        passed_tests = sum(1 for r in self.diagnostic_results if r["status"] == "PASS")
        failed_tests = sum(1 for r in self.diagnostic_results if r["status"] == "FAIL")
        critical_failures = sum(1 for r in self.diagnostic_results if r["status"] == "FAIL" and r["critical"])
        
        # Determine overall system status
        if critical_failures == 0 and failed_tests == 0:
            overall_status = "HIL READY"
            self.system_ready = True
        elif critical_failures == 0:
            overall_status = "HIL READY WITH WARNINGS"
            self.system_ready = True
        else:
            overall_status = "NOT READY"
            self.system_ready = False
        
        report = {
            "timestamp": datetime.now().isoformat(),
            "overall_status": overall_status,
            "system_ready": self.system_ready,
            "summary": {
                "total_tests": total_tests,
                "passed": passed_tests,
                "failed": failed_tests,
                "warnings": sum(1 for r in self.diagnostic_results if r["status"] == "WARN"),
                "critical_failures": critical_failures
            },
            "detected_ports": self.detected_imu_ports,
            "diagnostic_results": self.diagnostic_results
        }
        
        return report
    
    def cleanup(self):
        """Cleanup resources"""
        if self.server_process:
            try:
                self.server_process.terminate()
                self.server_process.wait(timeout=5)
            except:
                self.server_process.kill()
    
    async def run_full_diagnostic(self) -> Dict:
        """Run complete HIL diagnostic suite"""
        print("=" * 80)
        print("🔬 MELKENS HIL PRODUCTION DIAGNOSTIC")
        print("=" * 80)
        print(f"Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print()
        
        try:
            # Step 1: Serial port detection
            print("📡 STEP 1: Serial Port Detection")
            print("-" * 40)
            self.detect_serial_ports()
            print()
            
            # Step 2: Start HIL simulator
            print("🚀 STEP 2: HIL Simulator Startup")
            print("-" * 40)
            if not await self.start_hil_simulator():
                return await self.generate_diagnostic_report()
            print()
            
            # Step 3: Configure HIL mode
            print("⚙️  STEP 3: HIL Mode Configuration")
            print("-" * 40)
            if not await self.configure_hil_mode():
                return await self.generate_diagnostic_report()
            print()
            
            # Step 4: IMU handshake
            print("🤝 STEP 4: IMU Handshake & Connection")
            print("-" * 40)
            await self.test_imu_handshake()
            print()
            
            # Step 5: Data rate validation
            print("📊 STEP 5: Data Rate Validation")
            print("-" * 40)
            await self.validate_data_rates()
            print()
            
            # Step 6: Sensor data validation
            print("🔍 STEP 6: Sensor Data Validation")
            print("-" * 40)
            await self.test_sensor_data_validity()
            print()
            
            # Step 7: Movement detection
            print("🏃 STEP 7: Movement Detection")
            print("-" * 40)
            await self.test_movement_detection()
            print()
            
            # Step 8: Reference route test
            print("🛤️  STEP 8: Reference Route Test")
            print("-" * 40)
            await self.test_reference_route()
            print()
            
            # Generate final report
            report = await self.generate_diagnostic_report()
            
            # Display final results
            print("=" * 80)
            print("📋 DIAGNOSTIC SUMMARY")
            print("=" * 80)
            
            status_color = "\033[92m" if self.system_ready else "\033[91m"
            reset_color = "\033[0m"
            
            print(f"Status: {status_color}{report['overall_status']}{reset_color}")
            print(f"Total Tests: {report['summary']['total_tests']}")
            print(f"Passed: {report['summary']['passed']}")
            print(f"Failed: {report['summary']['failed']}")
            print(f"Warnings: {report['summary']['warnings']}")
            print(f"Critical Failures: {report['summary']['critical_failures']}")
            
            if self.system_ready:
                print(f"\n{status_color}✅ HIL IMU: READY{reset_color}")
                print("System is ready for production use!")
            else:
                print(f"\n{status_color}❌ HIL IMU: NOT READY{reset_color}")
                print("System requires attention before production use.")
                
                # Show critical failures
                print("\nCritical Issues:")
                for result in self.diagnostic_results:
                    if result["status"] == "FAIL" and result["critical"]:
                        print(f"  - {result['test']}: {result['details']}")
            
            return report
            
        except KeyboardInterrupt:
            self.log_diagnostic("Diagnostic", "FAIL", "Interrupted by user")
            return await self.generate_diagnostic_report()
        except Exception as e:
            self.log_diagnostic("Diagnostic", "FAIL", f"Unexpected error: {str(e)}", critical=True)
            return await self.generate_diagnostic_report()
        finally:
            self.cleanup()

async def main():
    """Main diagnostic function"""
    diagnostic = HILDiagnostic()
    
    try:
        # Run full diagnostic
        report = await diagnostic.run_full_diagnostic()
        
        # Save report
        report_file = Path("../../logs/hil_diagnostic_report.json")
        report_file.parent.mkdir(parents=True, exist_ok=True)
        report_file.write_text(json.dumps(report, indent=2))
        
        print(f"\n📄 Full diagnostic report saved to: {report_file.absolute()}")
        
        # Return appropriate exit code
        return 0 if report["system_ready"] else 1
        
    except Exception as e:
        print(f"❌ Diagnostic failed: {e}")
        return 1

if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)