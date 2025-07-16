#!/usr/bin/env python3
"""
MELKENS HIL System Test Script
Comprehensive testing of the HIL simulator functionality
"""

import asyncio
import json
import sys
import time
import subprocess
from pathlib import Path
from typing import Dict, List

import requests
import websockets
import serial.tools.list_ports

class HILSystemTester:
    """Comprehensive HIL system tester"""
    
    def __init__(self, base_url: str = "http://localhost:8000"):
        self.base_url = base_url
        self.ws_url = base_url.replace("http://", "ws://") + "/ws"
        self.test_results = []
        
    def log_result(self, test_name: str, passed: bool, details: str = ""):
        """Log test result"""
        status = "PASS" if passed else "FAIL"
        print(f"[{status}] {test_name}")
        if details:
            print(f"    {details}")
        
        self.test_results.append({
            "test": test_name,
            "passed": passed,
            "details": details,
            "timestamp": time.time()
        })
    
    def test_server_connectivity(self) -> bool:
        """Test if server is running and accessible"""
        try:
            response = requests.get(f"{self.base_url}/api/config", timeout=5)
            if response.status_code == 200:
                self.log_result("Server Connectivity", True, f"Status: {response.status_code}")
                return True
            else:
                self.log_result("Server Connectivity", False, f"Status: {response.status_code}")
                return False
        except Exception as e:
            self.log_result("Server Connectivity", False, str(e))
            return False
    
    def test_api_endpoints(self) -> bool:
        """Test all API endpoints"""
        endpoints = [
            ("GET", "/api/config"),
            ("GET", "/api/imu/status"),
            ("GET", "/api/imu/data"),
            ("GET", "/api/logs"),
        ]
        
        all_passed = True
        for method, endpoint in endpoints:
            try:
                if method == "GET":
                    response = requests.get(f"{self.base_url}{endpoint}", timeout=5)
                elif method == "POST":
                    response = requests.post(f"{self.base_url}{endpoint}", timeout=5)
                
                passed = response.status_code in [200, 201]
                self.log_result(f"API {method} {endpoint}", passed, f"Status: {response.status_code}")
                if not passed:
                    all_passed = False
                    
            except Exception as e:
                self.log_result(f"API {method} {endpoint}", False, str(e))
                all_passed = False
        
        return all_passed
    
    def test_configuration_update(self) -> bool:
        """Test system configuration updates"""
        try:
            # Test simulated mode configuration
            config = {
                "imu_mode": "simulated",
                "serial_port": "/dev/ttyUSB0",
                "baud_rate": 115200,
                "update_rate_ms": 100
            }
            
            response = requests.post(
                f"{self.base_url}/api/config/system",
                json=config,
                timeout=5
            )
            
            passed = response.status_code == 200
            if passed:
                result = response.json()
                passed = result.get("status") == "success"
            
            self.log_result("Configuration Update", passed, 
                          f"Response: {response.status_code}")
            return passed
            
        except Exception as e:
            self.log_result("Configuration Update", False, str(e))
            return False
    
    def test_fault_injection(self) -> bool:
        """Test fault injection configuration"""
        try:
            fault_config = {
                "enabled": True,
                "fault_type": "noise",
                "axis": "x",
                "severity": 2.0
            }
            
            response = requests.post(
                f"{self.base_url}/api/config/fault-injection",
                json=fault_config,
                timeout=5
            )
            
            passed = response.status_code == 200
            self.log_result("Fault Injection", passed, 
                          f"Response: {response.status_code}")
            
            # Disable fault injection
            fault_config["enabled"] = False
            requests.post(
                f"{self.base_url}/api/config/fault-injection",
                json=fault_config,
                timeout=5
            )
            
            return passed
            
        except Exception as e:
            self.log_result("Fault Injection", False, str(e))
            return False
    
    def test_route_management(self) -> bool:
        """Test route setting and management"""
        routes = ["A", "B", "C", "D"]
        all_passed = True
        
        for route_id in routes:
            try:
                response = requests.post(
                    f"{self.base_url}/api/robot/route/{route_id}",
                    timeout=5
                )
                
                passed = response.status_code == 200
                self.log_result(f"Route {route_id}", passed, 
                              f"Response: {response.status_code}")
                if not passed:
                    all_passed = False
                    
            except Exception as e:
                self.log_result(f"Route {route_id}", False, str(e))
                all_passed = False
        
        return all_passed
    
    def test_self_test(self) -> bool:
        """Test IMU self-test functionality"""
        try:
            response = requests.post(f"{self.base_url}/api/imu/self-test", timeout=10)
            
            if response.status_code == 200:
                result = response.json()
                passed = result.get("success", False)
                test_count = len(result.get("tests", {}))
                self.log_result("IMU Self-Test", passed, 
                              f"Tests run: {test_count}")
                return passed
            else:
                self.log_result("IMU Self-Test", False, 
                              f"Status: {response.status_code}")
                return False
                
        except Exception as e:
            self.log_result("IMU Self-Test", False, str(e))
            return False
    
    async def test_websocket_connection(self) -> bool:
        """Test WebSocket real-time communication"""
        try:
            async with websockets.connect(self.ws_url, timeout=5) as websocket:
                # Wait for data
                message = await asyncio.wait_for(websocket.recv(), timeout=10)
                data = json.loads(message)
                
                # Check message format
                has_type = "type" in data
                has_data = "data" in data
                
                passed = has_type and has_data
                self.log_result("WebSocket Connection", passed, 
                              f"Message type: {data.get('type', 'unknown')}")
                return passed
                
        except Exception as e:
            self.log_result("WebSocket Connection", False, str(e))
            return False
    
    async def test_realtime_data(self) -> bool:
        """Test real-time data streaming"""
        try:
            async with websockets.connect(self.ws_url, timeout=5) as websocket:
                messages_received = 0
                imu_data_received = False
                robot_data_received = False
                
                # Collect messages for 5 seconds
                start_time = time.time()
                while time.time() - start_time < 5:
                    try:
                        message = await asyncio.wait_for(websocket.recv(), timeout=1)
                        data = json.loads(message)
                        messages_received += 1
                        
                        if data.get("type") == "imu_data":
                            imu_data_received = True
                        elif data.get("type") == "robot_position":
                            robot_data_received = True
                            
                    except asyncio.TimeoutError:
                        continue
                
                passed = messages_received > 0 and imu_data_received
                self.log_result("Real-time Data", passed, 
                              f"Messages: {messages_received}, IMU: {imu_data_received}, Robot: {robot_data_received}")
                return passed
                
        except Exception as e:
            self.log_result("Real-time Data", False, str(e))
            return False
    
    def test_serial_ports(self) -> bool:
        """Test serial port detection"""
        try:
            ports = list(serial.tools.list_ports.comports())
            port_count = len(ports)
            
            # Log available ports
            port_names = [port.device for port in ports]
            
            passed = True  # Always pass - this is informational
            self.log_result("Serial Port Detection", passed, 
                          f"Found {port_count} ports: {port_names}")
            return passed
            
        except Exception as e:
            self.log_result("Serial Port Detection", False, str(e))
            return False
    
    def test_file_structure(self) -> bool:
        """Test required file structure"""
        required_files = [
            "main.py",
            "imu_manager.py", 
            "robot_simulator.py",
            "data_logger.py",
            "requirements.txt",
            "../frontend/index.html",
            "../frontend/style.css",
            "../frontend/main.js",
            "../../docs/HIL_IMU_GUIDE.md"
        ]
        
        missing_files = []
        for file_path in required_files:
            if not Path(file_path).exists():
                missing_files.append(file_path)
        
        passed = len(missing_files) == 0
        details = f"Missing files: {missing_files}" if missing_files else "All files present"
        self.log_result("File Structure", passed, details)
        return passed
    
    def test_logs_directory(self) -> bool:
        """Test logs directory and permissions"""
        try:
            logs_dir = Path("../../logs")
            
            # Create if doesn't exist
            logs_dir.mkdir(parents=True, exist_ok=True)
            
            # Test write permissions
            test_file = logs_dir / "test_write.tmp"
            test_file.write_text("test")
            test_file.unlink()
            
            passed = True
            self.log_result("Logs Directory", passed, f"Path: {logs_dir.absolute()}")
            return passed
            
        except Exception as e:
            self.log_result("Logs Directory", False, str(e))
            return False
    
    def generate_report(self) -> Dict:
        """Generate comprehensive test report"""
        total_tests = len(self.test_results)
        passed_tests = sum(1 for result in self.test_results if result["passed"])
        failed_tests = total_tests - passed_tests
        
        report = {
            "summary": {
                "total_tests": total_tests,
                "passed": passed_tests,
                "failed": failed_tests,
                "success_rate": (passed_tests / total_tests * 100) if total_tests > 0 else 0
            },
            "results": self.test_results,
            "timestamp": time.time()
        }
        
        return report
    
    async def run_all_tests(self) -> Dict:
        """Run comprehensive test suite"""
        print("=" * 60)
        print("MELKENS HIL System Test Suite")
        print("=" * 60)
        
        # Basic connectivity tests
        print("\n🔌 Basic Connectivity Tests")
        self.test_server_connectivity()
        self.test_api_endpoints()
        
        # Configuration tests
        print("\n⚙️  Configuration Tests")
        self.test_configuration_update()
        self.test_fault_injection()
        self.test_route_management()
        
        # Functionality tests
        print("\n🧪 Functionality Tests") 
        self.test_self_test()
        await self.test_websocket_connection()
        await self.test_realtime_data()
        
        # System tests
        print("\n💻 System Tests")
        self.test_serial_ports()
        self.test_file_structure()
        self.test_logs_directory()
        
        # Generate report
        report = self.generate_report()
        
        print("\n" + "=" * 60)
        print("TEST SUMMARY")
        print("=" * 60)
        print(f"Total Tests: {report['summary']['total_tests']}")
        print(f"Passed: {report['summary']['passed']}")
        print(f"Failed: {report['summary']['failed']}")
        print(f"Success Rate: {report['summary']['success_rate']:.1f}%")
        
        if report['summary']['failed'] > 0:
            print("\n❌ FAILED TESTS:")
            for result in self.test_results:
                if not result["passed"]:
                    print(f"  - {result['test']}: {result['details']}")
        else:
            print("\n✅ ALL TESTS PASSED!")
        
        return report

def check_server_running():
    """Check if the HIL server is running"""
    try:
        response = requests.get("http://localhost:8000/api/config", timeout=2)
        return response.status_code == 200
    except:
        return False

async def main():
    """Main test function"""
    if len(sys.argv) > 1 and sys.argv[1] == "--start-server":
        # Start server if requested
        print("Starting HIL server...")
        subprocess.Popen([sys.executable, "main.py"], 
                        stdout=subprocess.DEVNULL, 
                        stderr=subprocess.DEVNULL)
        
        # Wait for server to start
        for i in range(30):  # Wait up to 30 seconds
            if check_server_running():
                print("Server started successfully!")
                break
            time.sleep(1)
        else:
            print("❌ Failed to start server")
            return 1
    
    # Check if server is running
    if not check_server_running():
        print("❌ HIL server is not running!")
        print("Please start the server first:")
        print("  python main.py")
        print("\nOr run with --start-server to auto-start:")
        print("  python test_system.py --start-server")
        return 1
    
    # Run tests
    tester = HILSystemTester()
    report = await tester.run_all_tests()
    
    # Save report
    report_file = Path("../../logs/test_report.json")
    report_file.parent.mkdir(parents=True, exist_ok=True)
    report_file.write_text(json.dumps(report, indent=2))
    print(f"\n📄 Test report saved to: {report_file.absolute()}")
    
    # Return appropriate exit code
    return 0 if report['summary']['failed'] == 0 else 1

if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)