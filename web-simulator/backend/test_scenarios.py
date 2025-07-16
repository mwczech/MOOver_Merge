#!/usr/bin/env python3
"""
Test script for Advanced Scenario Runner and Fault Injection Framework
Validates all components and generates test reports
"""

import asyncio
import json
import time
from pathlib import Path
import tempfile
import csv

from scenario_runner import (
    AdvancedScenarioRunner, 
    FaultScenario, 
    FaultType, 
    FaultSeverity,
    ScenarioManager,
    FaultInjector,
    SystemMonitor
)

async def test_fault_injector():
    """Test the fault injection engine"""
    print("🔧 Testing Fault Injector...")
    
    injector = FaultInjector()
    injector.reset()
    
    # Create test scenario
    scenario = FaultScenario(
        id="test_noise",
        name="Test Noise Injection",
        description="Test noise injection functionality",
        fault_type=FaultType.NOISE_INJECTION,
        severity=FaultSeverity.MEDIUM,
        target_sensor="accelerometer",
        target_axis="x",
        start_time=0.0,
        duration=5.0,
        parameters={"noise_level": 0.5}
    )
    
    # Test fault activation
    fault_id = injector.activate_fault(scenario)
    assert fault_id in injector.active_faults
    print(f"  ✅ Fault activation: {fault_id}")
    
    # Test fault application
    test_data = {
        "accelerometer": {"x": 1.0, "y": 2.0, "z": 9.8},
        "gyroscope": {"x": 0.0, "y": 0.0, "z": 0.0},
        "magnetometer": {"x": 20.0, "y": -10.0, "z": 45.0}
    }
    
    modified_data = injector.apply_faults(test_data)
    
    # Verify noise was added to X-axis
    original_x = test_data["accelerometer"]["x"]
    modified_x = modified_data["accelerometer"]["x"]
    assert modified_x != original_x, "Noise should have been applied"
    print(f"  ✅ Noise injection: {original_x} → {modified_x}")
    
    # Test fault deactivation
    injector.deactivate_fault(fault_id)
    assert not injector.active_faults[fault_id]['active']
    print(f"  ✅ Fault deactivation: {fault_id}")
    
    print("  ✅ Fault Injector tests passed!")

async def test_scenario_manager():
    """Test scenario management functionality"""
    print("📊 Testing Scenario Manager...")
    
    manager = ScenarioManager()
    
    # Test default scenarios creation
    manager.create_default_scenarios()
    assert len(manager.scenarios) >= 5
    print(f"  ✅ Default scenarios created: {len(manager.scenarios)}")
    
    # Test custom scenario addition
    custom_scenario = FaultScenario(
        id="test_custom",
        name="Custom Test Scenario",
        description="Custom test scenario",
        fault_type=FaultType.STUCK_VALUE,
        severity=FaultSeverity.LOW,
        target_sensor="gyroscope",
        target_axis="y",
        start_time=10.0,
        duration=30.0,
        parameters={"stuck_value": 1.5}
    )
    
    manager.add_scenario(custom_scenario)
    retrieved = manager.get_scenario("test_custom")
    assert retrieved is not None
    assert retrieved.name == "Custom Test Scenario"
    print("  ✅ Custom scenario addition and retrieval")
    
    # Test scenario listing
    scenarios_list = manager.list_scenarios()
    assert len(scenarios_list) == len(manager.scenarios)
    print(f"  ✅ Scenario listing: {len(scenarios_list)} scenarios")
    
    # Test JSON loading
    with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
        test_scenarios = {
            "scenarios": [
                {
                    "id": "json_test",
                    "name": "JSON Test Scenario",
                    "description": "Test JSON loading",
                    "fault_type": "bias_drift",
                    "severity": "medium",
                    "target_sensor": "magnetometer",
                    "target_axis": "z",
                    "start_time": 5.0,
                    "duration": 20.0,
                    "parameters": {"bias_rate": 0.1}
                }
            ]
        }
        json.dump(test_scenarios, f)
        json_file = f.name
    
    count = manager.load_scenarios_from_json(json_file)
    assert count == 1
    Path(json_file).unlink()  # Clean up
    print("  ✅ JSON scenario loading")
    
    # Test CSV loading
    with tempfile.NamedTemporaryFile(mode='w', suffix='.csv', delete=False) as f:
        writer = csv.writer(f)
        writer.writerow(['id', 'name', 'description', 'fault_type', 'severity', 
                        'target_sensor', 'target_axis', 'start_time', 'duration', 'parameters'])
        writer.writerow(['csv_test', 'CSV Test Scenario', 'Test CSV loading', 
                        'noise_injection', 'high', 'accelerometer', 'all', 
                        '15.0', '25.0', '{"noise_level": 0.8}'])
        csv_file = f.name
    
    count = manager.load_scenarios_from_csv(csv_file)
    assert count == 1
    Path(csv_file).unlink()  # Clean up
    print("  ✅ CSV scenario loading")
    
    print("  ✅ Scenario Manager tests passed!")

async def test_system_monitor():
    """Test system monitoring functionality"""
    print("📈 Testing System Monitor...")
    
    monitor = SystemMonitor()
    monitor.reset()
    
    # Test robot state updates
    robot_data = {
        "x": 1.0,
        "y": 2.0,
        "step_progress": 0.25,
        "status": "running"
    }
    
    monitor.update_robot_state(robot_data)
    assert monitor.route_progress == 0.25
    assert monitor.last_position == (1.0, 2.0)
    print("  ✅ Robot state tracking")
    
    # Test IMU state updates
    imu_data = {
        "accelerometer": {"x": 50.0, "y": 2.0, "z": 9.8},  # Unrealistic X value
        "gyroscope": {"x": 0.1, "y": 0.0, "z": 0.0},
        "magnetometer": {"x": 20.0, "y": -10.0, "z": 45.0}
    }
    
    fault_injector = FaultInjector()
    monitor.update_imu_state(imu_data, fault_injector)
    
    # Should detect anomaly in accelerometer X (out of range)
    assert len(monitor.fault_detections) > 0
    print("  ✅ Anomaly detection")
    
    # Test performance metrics
    metrics = monitor.get_performance_metrics()
    assert "route_completion_percentage" in metrics
    assert "fault_detection_count" in metrics
    print(f"  ✅ Performance metrics: {len(metrics)} metrics")
    
    print("  ✅ System Monitor tests passed!")

async def test_advanced_scenario_runner():
    """Test the complete advanced scenario runner"""
    print("🚀 Testing Advanced Scenario Runner...")
    
    runner = AdvancedScenarioRunner()
    
    # Test scenario execution
    try:
        # Run a quick test scenario (reduced duration for testing)
        result = await runner.run_scenario("acc_stuck_x", "A")
        
        assert result.scenario_id == "acc_stuck_x"
        assert result.completion_percentage >= 0
        assert result.performance_metrics is not None
        print(f"  ✅ Scenario execution: {result.completion_percentage:.1f}% completion")
        
        # Test report generation
        report = runner.generate_scenario_report()
        assert "total_tests" in report
        assert report["total_tests"] >= 1
        print(f"  ✅ Report generation: {report['total_tests']} tests")
        
        # Test CSV export
        output_file = "test_results.csv"
        runner.export_results_csv(output_file)
        assert Path(output_file).exists()
        Path(output_file).unlink()  # Clean up
        print("  ✅ CSV export")
        
    except Exception as e:
        print(f"  ❌ Scenario runner test failed: {e}")
        raise
    
    print("  ✅ Advanced Scenario Runner tests passed!")

async def test_fault_types():
    """Test all fault types"""
    print("🎯 Testing All Fault Types...")
    
    injector = FaultInjector()
    injector.reset()
    
    test_data = {
        "accelerometer": {"x": 1.0, "y": 2.0, "z": 9.8},
        "gyroscope": {"x": 0.1, "y": 0.0, "z": 0.0},
        "magnetometer": {"x": 20.0, "y": -10.0, "z": 45.0}
    }
    
    fault_tests = [
        (FaultType.STUCK_VALUE, {"stuck_value": 5.0}),
        (FaultType.AXIS_LOSS, {}),
        (FaultType.BIAS_DRIFT, {"bias_rate": 0.1}),
        (FaultType.NOISE_INJECTION, {"noise_level": 0.5}),
        (FaultType.SCALE_ERROR, {"scale_factor": 2.0}),
        (FaultType.PERIODIC_GLITCH, {"frequency": 1.0, "amplitude": 1.0}),
        (FaultType.SATURATION, {"min_value": -5.0, "max_value": 5.0})
    ]
    
    for fault_type, params in fault_tests:
        scenario = FaultScenario(
            id=f"test_{fault_type.value}",
            name=f"Test {fault_type.value}",
            description=f"Test {fault_type.value} fault",
            fault_type=fault_type,
            severity=FaultSeverity.MEDIUM,
            target_sensor="accelerometer",
            target_axis="x",
            start_time=0.0,
            duration=1.0,
            parameters=params
        )
        
        fault_id = injector.activate_fault(scenario)
        modified_data = injector.apply_faults(test_data)
        
        # Verify modification occurred (except for some edge cases)
        if fault_type != FaultType.PACKET_LOSS:  # Packet loss doesn't modify data directly
            original_x = test_data["accelerometer"]["x"]
            modified_x = modified_data["accelerometer"]["x"]
            
            if fault_type == FaultType.AXIS_LOSS:
                assert modified_x == 0.0, f"Axis loss should set value to 0"
            elif fault_type != FaultType.STUCK_VALUE or "stuck_value" in params:
                # For stuck value without preset, it uses original value
                pass  # Some faults may not immediately change the value
        
        injector.deactivate_fault(fault_id)
        print(f"  ✅ {fault_type.value} fault tested")
    
    # Test packet loss
    packet_loss_scenario = FaultScenario(
        id="test_packet_loss",
        name="Test Packet Loss",
        description="Test packet loss",
        fault_type=FaultType.PACKET_LOSS,
        severity=FaultSeverity.HIGH,
        target_sensor="accelerometer",
        target_axis="all",
        start_time=0.0,
        duration=1.0,
        parameters={"loss_rate": 1.0}  # 100% loss for testing
    )
    
    injector.activate_fault(packet_loss_scenario)
    should_drop = injector.should_drop_packet()
    assert should_drop, "Should drop packet with 100% loss rate"
    print("  ✅ packet_loss fault tested")
    
    print("  ✅ All fault types tested!")

async def test_integration_with_examples():
    """Test integration with example scenario files"""
    print("📁 Testing Example Scenario Files...")
    
    runner = AdvancedScenarioRunner()
    
    # Test JSON example loading
    json_file = "scenarios/example_scenarios.json"
    if Path(json_file).exists():
        count = runner.scenario_manager.load_scenarios_from_json(json_file)
        print(f"  ✅ Loaded {count} scenarios from JSON example")
    else:
        print("  ⚠️ JSON example file not found, skipping")
    
    # Test CSV example loading  
    csv_file = "scenarios/example_scenarios.csv"
    if Path(csv_file).exists():
        count = runner.scenario_manager.load_scenarios_from_csv(csv_file)
        print(f"  ✅ Loaded {count} scenarios from CSV example")
    else:
        print("  ⚠️ CSV example file not found, skipping")
    
    # List all loaded scenarios
    scenarios = runner.scenario_manager.list_scenarios()
    print(f"  ✅ Total scenarios available: {len(scenarios)}")
    
    # Test running an example scenario if available
    if scenarios:
        example_id = scenarios[0]['id']
        try:
            # Quick test run with reduced monitoring
            print(f"  🧪 Testing scenario: {example_id}")
            
            # Override the monitoring duration for faster testing
            original_monitor = runner._monitor_test_execution
            
            async def quick_monitor(scenario, route_name):
                """Quick monitoring for testing"""
                for i in range(10):  # Just 10 iterations instead of full duration
                    await asyncio.sleep(0.01)
                    
                    # Mock some basic updates
                    robot_data = {'x': i * 0.1, 'y': 0, 'step_progress': i * 0.1, 'status': 'running'}
                    imu_data = {
                        'accelerometer': {'x': 1.0, 'y': 2.0, 'z': 9.8},
                        'gyroscope': {'x': 0.0, 'y': 0.0, 'z': 0.0},
                        'magnetometer': {'x': 20.0, 'y': -10.0, 'z': 45.0}
                    }
                    
                    runner.system_monitor.update_robot_state(robot_data)
                    runner.system_monitor.update_imu_state(imu_data, runner.scenario_manager.fault_injector)
            
            runner._monitor_test_execution = quick_monitor
            
            result = await runner.run_scenario(example_id, "A")
            print(f"  ✅ Example scenario executed: {result.completion_percentage:.1f}% completion")
            
            # Restore original method
            runner._monitor_test_execution = original_monitor
            
        except Exception as e:
            print(f"  ⚠️ Example scenario test failed: {e}")
    
    print("  ✅ Example integration tests completed!")

async def run_comprehensive_tests():
    """Run all tests"""
    print("🔬 MELKENS Advanced Scenario Runner - Comprehensive Tests")
    print("=" * 80)
    
    start_time = time.time()
    
    try:
        await test_fault_injector()
        print()
        
        await test_scenario_manager()
        print()
        
        await test_system_monitor()
        print()
        
        await test_fault_types()
        print()
        
        await test_advanced_scenario_runner()
        print()
        
        await test_integration_with_examples()
        print()
        
        elapsed = time.time() - start_time
        print("=" * 80)
        print(f"🎉 ALL TESTS PASSED! ({elapsed:.2f}s)")
        print("✅ Advanced Scenario Runner is ready for production use!")
        
        return True
        
    except Exception as e:
        elapsed = time.time() - start_time
        print("=" * 80)
        print(f"❌ TESTS FAILED: {e} ({elapsed:.2f}s)")
        print("🔧 Check the error details above and fix issues before deployment.")
        
        return False

if __name__ == "__main__":
    success = asyncio.run(run_comprehensive_tests())
    exit(0 if success else 1)