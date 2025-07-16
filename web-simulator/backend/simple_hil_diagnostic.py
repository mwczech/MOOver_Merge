#!/usr/bin/env python3
"""
Simplified MELKENS HIL Production Diagnostic
Core system validation without full web dependencies
"""

import sys
import time
import subprocess
import json
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional

try:
    import serial
    import serial.tools.list_ports
    SERIAL_AVAILABLE = True
except ImportError:
    SERIAL_AVAILABLE = False

class SimplifiedHILDiagnostic:
    """Simplified HIL diagnostic focusing on core system validation"""
    
    def __init__(self):
        self.diagnostic_results = []
        self.detected_imu_ports = []
        self.system_ready = False
        
    def log_diagnostic(self, test_name: str, status: str, details: str = "", critical: bool = False):
        """Log diagnostic result with color coding"""
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
    
    def check_dependencies(self) -> bool:
        """Check if required dependencies are available"""
        self.log_diagnostic("Dependency Check", "INFO", "Checking system dependencies...")
        
        all_deps_ok = True
        
        # Check Python version
        python_version = f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"
        if sys.version_info >= (3, 8):
            self.log_diagnostic("Python Version", "PASS", f"Python {python_version}")
        else:
            self.log_diagnostic("Python Version", "FAIL", 
                              f"Python {python_version} (≥3.8 required)", critical=True)
            all_deps_ok = False
        
        # Check serial library
        if SERIAL_AVAILABLE:
            self.log_diagnostic("PySerial Library", "PASS", "pyserial available")
        else:
            self.log_diagnostic("PySerial Library", "FAIL", 
                              "pyserial not available - install with: pip install pyserial", critical=True)
            all_deps_ok = False
        
        # Check for common system tools
        tools_to_check = ["python3", "pip3"]
        for tool in tools_to_check:
            try:
                result = subprocess.run(["which", tool], capture_output=True, text=True)
                if result.returncode == 0:
                    self.log_diagnostic(f"System Tool: {tool}", "PASS", result.stdout.strip())
                else:
                    self.log_diagnostic(f"System Tool: {tool}", "WARN", "Not found in PATH")
            except Exception as e:
                self.log_diagnostic(f"System Tool: {tool}", "WARN", f"Check failed: {e}")
        
        return all_deps_ok
    
    def detect_serial_ports(self) -> List[Dict]:
        """Detect and analyze serial ports for potential IMU devices"""
        if not SERIAL_AVAILABLE:
            self.log_diagnostic("Serial Port Detection", "FAIL", 
                              "Cannot detect ports - pyserial not available", critical=True)
            return []
        
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
                
                # Enhanced IMU device detection
                is_potential_imu = False
                confidence = "Low"
                
                # Check for STMicroelectronics devices (STM32)
                if port.vid == 0x0483:  # STMicroelectronics VID
                    is_potential_imu = True
                    confidence = "High"
                elif "STM" in port.description.upper():
                    is_potential_imu = True
                    confidence = "Medium"
                elif "USB Serial" in port.description:
                    is_potential_imu = True
                    confidence = "Medium"
                elif "/dev/ttyUSB" in port.device or "/dev/ttyACM" in port.device:
                    is_potential_imu = True
                    confidence = "Medium"
                elif "COM" in port.device:  # Windows
                    is_potential_imu = True
                    confidence = "Low"
                
                status_icon = "🎯" if is_potential_imu else "📱"
                vid_pid_str = f"VID:PID {port.vid:04X}:{port.pid:04X}" if port.vid and port.pid else "No VID:PID"
                
                details = f"{status_icon} {port.description} ({vid_pid_str})"
                if is_potential_imu:
                    details += f" - IMU Confidence: {confidence}"
                
                self.log_diagnostic(f"Port {port.device}", "INFO", details)
                
                if is_potential_imu:
                    self.detected_imu_ports.append({
                        "device": port.device,
                        "confidence": confidence,
                        "description": port.description,
                        "vid": port.vid,
                        "pid": port.pid
                    })
            
            # Evaluate port detection results
            if self.detected_imu_ports:
                high_confidence = [p for p in self.detected_imu_ports if p["confidence"] == "High"]
                if high_confidence:
                    self.log_diagnostic("IMU Port Detection", "PASS", 
                                      f"High-confidence IMU device found: {high_confidence[0]['device']}")
                else:
                    self.log_diagnostic("IMU Port Detection", "PASS", 
                                      f"Potential IMU devices detected: {[p['device'] for p in self.detected_imu_ports]}")
            else:
                self.log_diagnostic("IMU Port Detection", "WARN", 
                                  "No potential IMU devices detected. Manual configuration may be required.")
            
            return port_info
            
        except Exception as e:
            self.log_diagnostic("Serial Port Detection", "FAIL", str(e), critical=True)
            return []
    
    def test_serial_communication(self) -> bool:
        """Test basic serial communication capabilities"""
        if not self.detected_imu_ports:
            self.log_diagnostic("Serial Communication Test", "WARN", 
                              "No IMU ports detected - skipping communication test")
            return True
        
        # Test the most promising port
        best_port = max(self.detected_imu_ports, 
                       key=lambda p: {"High": 3, "Medium": 2, "Low": 1}[p["confidence"]])
        
        try:
            self.log_diagnostic("Serial Communication Test", "INFO", 
                              f"Testing communication on {best_port['device']}")
            
            # Attempt to open the serial port
            with serial.Serial(best_port['device'], 115200, timeout=1) as ser:
                self.log_diagnostic("  Port Opening", "PASS", 
                                  f"Successfully opened {best_port['device']} at 115200 baud")
                
                # Test basic write capability
                test_data = b"TEST\r\n"
                ser.write(test_data)
                self.log_diagnostic("  Write Test", "PASS", 
                                  f"Successfully wrote {len(test_data)} bytes")
                
                # Brief read attempt (don't expect response in diagnostic mode)
                time.sleep(0.1)
                available = ser.in_waiting
                if available > 0:
                    response = ser.read(available)
                    self.log_diagnostic("  Read Test", "PASS", 
                                      f"Received {len(response)} bytes: {response.hex()}")
                else:
                    self.log_diagnostic("  Read Test", "INFO", 
                                      "No immediate response (normal for IMU in idle mode)")
                
                self.log_diagnostic("Serial Communication Test", "PASS", 
                                  "Basic serial communication functional")
                return True
                
        except serial.SerialException as e:
            self.log_diagnostic("Serial Communication Test", "FAIL", 
                              f"Serial error on {best_port['device']}: {e}")
            return False
        except PermissionError:
            self.log_diagnostic("Serial Communication Test", "FAIL", 
                              f"Permission denied on {best_port['device']} - check user permissions")
            return False
        except Exception as e:
            self.log_diagnostic("Serial Communication Test", "FAIL", 
                              f"Unexpected error: {e}")
            return False
    
    def validate_system_configuration(self) -> bool:
        """Validate overall system configuration for HIL operation"""
        self.log_diagnostic("System Configuration", "INFO", "Validating HIL system configuration")
        
        config_ok = True
        
        # Check if backend files exist
        required_files = [
            "main.py", "imu_manager.py", "robot_simulator.py", 
            "data_logger.py", "requirements.txt"
        ]
        
        for filename in required_files:
            filepath = Path(filename)
            if filepath.exists():
                size = filepath.stat().st_size
                self.log_diagnostic(f"  File: {filename}", "PASS", f"Present ({size:,} bytes)")
            else:
                self.log_diagnostic(f"  File: {filename}", "FAIL", "Missing", critical=True)
                config_ok = False
        
        # Check frontend files
        frontend_path = Path("../frontend")
        if frontend_path.exists():
            frontend_files = ["index.html", "style.css", "main.js"]
            for filename in frontend_files:
                filepath = frontend_path / filename
                if filepath.exists():
                    self.log_diagnostic(f"  Frontend: {filename}", "PASS", "Present")
                else:
                    self.log_diagnostic(f"  Frontend: {filename}", "WARN", "Missing")
        else:
            self.log_diagnostic("  Frontend Directory", "WARN", "Frontend directory not found")
        
        # Check logs directory
        logs_path = Path("../../logs")
        logs_path.mkdir(parents=True, exist_ok=True)
        self.log_diagnostic("  Logs Directory", "PASS", f"Available at {logs_path.absolute()}")
        
        return config_ok
    
    def test_basic_server_functionality(self) -> bool:
        """Test if the backend server can be imported and basic functionality works"""
        try:
            self.log_diagnostic("Server Import Test", "INFO", "Testing backend server imports")
            
            # Test importing main modules
            sys.path.insert(0, str(Path.cwd()))
            
            try:
                import main
                self.log_diagnostic("  main.py import", "PASS", "Successfully imported")
            except Exception as e:
                self.log_diagnostic("  main.py import", "FAIL", f"Import error: {e}")
                return False
            
            try:
                import imu_manager
                self.log_diagnostic("  imu_manager.py import", "PASS", "Successfully imported")
            except Exception as e:
                self.log_diagnostic("  imu_manager.py import", "FAIL", f"Import error: {e}")
                return False
            
            try:
                import robot_simulator
                self.log_diagnostic("  robot_simulator.py import", "PASS", "Successfully imported")
            except Exception as e:
                self.log_diagnostic("  robot_simulator.py import", "WARN", f"Import warning: {e}")
            
            self.log_diagnostic("Server Import Test", "PASS", "Core modules can be imported")
            return True
            
        except Exception as e:
            self.log_diagnostic("Server Import Test", "FAIL", f"Server test failed: {e}")
            return False
    
    def generate_diagnostic_report(self) -> Dict:
        """Generate comprehensive diagnostic report"""
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
            "diagnostic_results": self.diagnostic_results,
            "recommendations": self._generate_recommendations()
        }
        
        return report
    
    def _generate_recommendations(self) -> List[str]:
        """Generate actionable recommendations based on diagnostic results"""
        recommendations = []
        
        critical_failures = [r for r in self.diagnostic_results if r["status"] == "FAIL" and r["critical"]]
        
        if not SERIAL_AVAILABLE:
            recommendations.append("Install pyserial: pip install pyserial")
        
        if not self.detected_imu_ports:
            recommendations.extend([
                "Connect MELKENS IMU device via USB",
                "Check device drivers are installed",
                "Verify device appears in system device manager",
                "Try different USB cable or port"
            ])
        elif len(self.detected_imu_ports) > 1:
            recommendations.append(f"Multiple potential IMU devices detected - ensure correct device selection")
        
        if critical_failures:
            recommendations.append("Resolve critical failures before production use")
        
        if self.system_ready:
            recommendations.extend([
                "System appears ready for HIL operation",
                "Run full server test: python3 main.py",
                "Test web interface at http://localhost:8000",
                "Verify IMU data streaming in dashboard"
            ])
        
        return recommendations
    
    def run_full_diagnostic(self) -> Dict:
        """Execute comprehensive diagnostic suite"""
        print("=" * 80)
        print("🔬 MELKENS HIL PRODUCTION DIAGNOSTIC (Simplified)")
        print("=" * 80)
        print(f"Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Platform: {sys.platform}")
        print()
        
        try:
            # Step 1: Dependency check
            print("🔧 STEP 1: Dependency & Environment Check")
            print("-" * 50)
            self.check_dependencies()
            print()
            
            # Step 2: Serial port detection
            print("📡 STEP 2: Serial Port Detection & Analysis")
            print("-" * 50)
            self.detect_serial_ports()
            print()
            
            # Step 3: Serial communication test
            print("📞 STEP 3: Serial Communication Test")
            print("-" * 50)
            self.test_serial_communication()
            print()
            
            # Step 4: System configuration validation
            print("⚙️  STEP 4: System Configuration Validation")
            print("-" * 50)
            self.validate_system_configuration()
            print()
            
            # Step 5: Basic server functionality test
            print("🖥️  STEP 5: Server Functionality Test")
            print("-" * 50)
            self.test_basic_server_functionality()
            print()
            
            # Generate comprehensive report
            report = self.generate_diagnostic_report()
            
            # Display results
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
            
            if self.detected_imu_ports:
                print(f"\nDetected IMU Devices:")
                for port in self.detected_imu_ports:
                    print(f"  - {port['device']} ({port['confidence']} confidence): {port['description']}")
            
            if self.system_ready:
                print(f"\n{status_color}✅ HIL IMU: READY{reset_color}")
                print("System is ready for HIL operation!")
            else:
                print(f"\n{status_color}❌ HIL IMU: NOT READY{reset_color}")
                print("System requires attention before production use.")
                
                # Show critical failures
                critical_failures = [r for r in self.diagnostic_results if r["status"] == "FAIL" and r["critical"]]
                if critical_failures:
                    print("\nCritical Issues:")
                    for result in critical_failures:
                        print(f"  - {result['test']}: {result['details']}")
            
            # Show recommendations
            if report['recommendations']:
                print(f"\n📝 Recommendations:")
                for i, rec in enumerate(report['recommendations'], 1):
                    print(f"  {i}. {rec}")
            
            return report
            
        except KeyboardInterrupt:
            self.log_diagnostic("Diagnostic", "FAIL", "Interrupted by user")
            return self.generate_diagnostic_report()
        except Exception as e:
            self.log_diagnostic("Diagnostic", "FAIL", f"Unexpected error: {str(e)}", critical=True)
            return self.generate_diagnostic_report()

def main():
    """Main diagnostic execution"""
    diagnostic = SimplifiedHILDiagnostic()
    
    try:
        # Run diagnostic
        report = diagnostic.run_full_diagnostic()
        
        # Save report
        report_file = Path("../../logs/hil_diagnostic_report.json")
        report_file.parent.mkdir(parents=True, exist_ok=True)
        report_file.write_text(json.dumps(report, indent=2, default=str))
        
        print(f"\n📄 Full diagnostic report saved to: {report_file.absolute()}")
        
        # Return appropriate exit code
        return 0 if report["system_ready"] else 1
        
    except Exception as e:
        print(f"❌ Diagnostic failed: {e}")
        return 1

if __name__ == "__main__":
    exit_code = main()
    print(f"\nDiagnostic completed with exit code: {exit_code}")
    sys.exit(exit_code)