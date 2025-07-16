#!/usr/bin/env python3
"""
MELKENS Advanced Scenario Runner - End-to-End Demonstration
Shows complete fault injection framework functionality
"""

import asyncio
import json
import time
from pathlib import Path
from scenario_runner import AdvancedScenarioRunner

async def demonstrate_advanced_scenarios():
    """Comprehensive demonstration of the advanced scenario framework"""
    
    print("🎯 MELKENS Advanced Scenario Runner - Live Demonstration")
    print("=" * 70)
    print("This demonstration shows the complete fault injection framework")
    print("including scenario loading, fault injection, and comprehensive reporting.")
    print()
    
    # Initialize the runner
    runner = AdvancedScenarioRunner()
    
    # Step 1: Load example scenarios
    print("📁 STEP 1: Loading Example Scenarios")
    print("-" * 40)
    
    json_count = runner.scenario_manager.load_scenarios_from_json("scenarios/example_scenarios.json")
    csv_count = runner.scenario_manager.load_scenarios_from_csv("scenarios/example_scenarios.csv")
    
    print(f"✅ Loaded {json_count} scenarios from JSON")
    print(f"✅ Loaded {csv_count} scenarios from CSV")
    print(f"✅ Total scenarios available: {len(runner.scenario_manager.scenarios)}")
    print()
    
    # Step 2: Display available scenarios
    print("📋 STEP 2: Available Test Scenarios")
    print("-" * 40)
    
    scenarios = runner.scenario_manager.list_scenarios()
    fault_types = {}
    
    for scenario in scenarios[:10]:  # Show first 10
        fault_type = scenario['fault_type']
        if fault_type not in fault_types:
            fault_types[fault_type] = []
        fault_types[fault_type].append(scenario['name'])
    
    for fault_type, names in fault_types.items():
        print(f"🔧 {fault_type.upper()}: {len(names)} scenarios")
        for name in names[:2]:  # Show first 2 of each type
            print(f"   • {name}")
        if len(names) > 2:
            print(f"   • ... and {len(names) - 2} more")
    
    print()
    
    # Step 3: Run selected test scenarios
    print("🧪 STEP 3: Running Test Scenarios")
    print("-" * 40)
    
    # Select a few representative scenarios to test
    test_scenarios = [
        "basic_acc_stuck",
        "high_gyro_noise", 
        "packet_loss_critical"
    ]
    
    results = []
    
    for scenario_id in test_scenarios:
        if scenario_id in runner.scenario_manager.scenarios:
            print(f"Running: {scenario_id}")
            start_time = time.time()
            
            try:
                result = await runner.run_scenario(scenario_id, "A")
                elapsed = time.time() - start_time
                
                print(f"  ✅ Completed in {elapsed:.1f}s")
                print(f"  📊 Route completion: {result.completion_percentage:.1f}%")
                print(f"  🔍 Fault detected: {result.fault_detected}")
                print(f"  🔄 Recovery achieved: {result.recovery_achieved}")
                print()
                
                results.append(result)
                
            except Exception as e:
                print(f"  ❌ Test failed: {e}")
                print()
    
    # Step 4: Generate comprehensive report
    print("📊 STEP 4: Comprehensive Analysis Report")
    print("-" * 40)
    
    if results:
        report = runner.generate_scenario_report()
        
        print(f"📈 Test Summary:")
        print(f"   • Total tests run: {report['total_tests']}")
        print(f"   • Tests passed: {report['summary']['tests_passed']}")
        print(f"   • Tests failed: {report['summary']['tests_failed']}")
        print(f"   • Faults detected: {report['summary']['faults_detected']}")
        print(f"   • Recoveries achieved: {report['summary']['recoveries_achieved']}")
        print(f"   • Average completion rate: {report['summary']['avg_completion_rate']:.1f}%")
        
        if 'avg_detection_time' in report['summary'] and report['summary']['avg_detection_time']:
            print(f"   • Average detection time: {report['summary']['avg_detection_time']:.1f}s")
        
        print()
        
        # Show recommendations
        if report.get('recommendations'):
            print("💡 System Recommendations:")
            for i, rec in enumerate(report['recommendations'], 1):
                print(f"   {i}. {rec}")
            print()
    
    # Step 5: Export data for analysis
    print("💾 STEP 5: Data Export")
    print("-" * 40)
    
    # Export to CSV
    csv_file = "logs/demonstration_results.csv"
    runner.export_results_csv(csv_file)
    if Path(csv_file).exists():
        print(f"✅ Results exported to: {csv_file}")
    
    # Save comprehensive report
    report_file = "logs/demonstration_report.json"
    if results:
        Path("logs").mkdir(exist_ok=True)
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2, default=str)
        print(f"✅ Detailed report saved to: {report_file}")
    
    print()
    
    # Step 6: Show fault injection capabilities
    print("⚙️  STEP 6: Fault Injection Capabilities")
    print("-" * 40)
    
    fault_info = {
        "stuck_value": "Sensor reading frozen at specific value",
        "axis_loss": "Complete sensor axis failure",
        "bias_drift": "Gradual bias accumulation over time", 
        "noise_injection": "Random Gaussian noise addition",
        "packet_loss": "Random data packet loss",
        "scale_error": "Incorrect sensor scaling factor",
        "periodic_glitch": "Periodic signal disturbances",
        "saturation": "Value clipping to min/max limits"
    }
    
    severities = ["low", "medium", "high", "critical"]
    
    print("🔧 Available Fault Types:")
    for fault_type, description in fault_info.items():
        print(f"   • {fault_type}: {description}")
    
    print(f"\n📏 Severity Levels: {', '.join(severities)}")
    print(f"🎯 Target Options: accelerometer, gyroscope, magnetometer")
    print(f"📐 Axis Options: x, y, z, all")
    print()
    
    # Summary
    print("=" * 70)
    print("🎉 DEMONSTRATION COMPLETE")
    print("=" * 70)
    print("The MELKENS Advanced Scenario Runner provides:")
    print("✅ 8 different fault injection types")
    print("✅ Comprehensive system response monitoring") 
    print("✅ Real-time fault detection and recovery assessment")
    print("✅ Detailed performance metrics and analysis")
    print("✅ JSON and CSV scenario definition formats")
    print("✅ Automated report generation with recommendations")
    print("✅ Export capabilities for further analysis")
    print()
    print("🚀 The system is ready for production validation testing!")
    print()
    
    return len(results)

if __name__ == "__main__":
    try:
        result_count = asyncio.run(demonstrate_advanced_scenarios())
        print(f"✅ Demonstration completed successfully with {result_count} test results")
    except KeyboardInterrupt:
        print("\n⏹️  Demonstration interrupted by user")
    except Exception as e:
        print(f"\n❌ Demonstration failed: {e}")
        import traceback
        traceback.print_exc()