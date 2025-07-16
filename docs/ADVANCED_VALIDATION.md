# MELKENS Advanced Validation & Fault Injection Framework

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Getting Started](#getting-started)
4. [Scenario Definition](#scenario-definition)
5. [Running Tests](#running-tests)
6. [Analysis & Reporting](#analysis--reporting)
7. [Best Practices](#best-practices)
8. [Example Test Scenarios](#example-test-scenarios)
9. [API Reference](#api-reference)
10. [Troubleshooting](#troubleshooting)

## Overview

The MELKENS Advanced Validation & Fault Injection Framework provides comprehensive testing capabilities for IMU-based navigation systems. It enables systematic validation of system robustness through controlled fault injection, real-time monitoring, and detailed analysis reporting.

### Key Features

- **8 Fault Types**: Stuck values, axis loss, bias drift, noise injection, packet loss, scale errors, periodic glitches, and saturation
- **Live Fault Injection**: Real-time application of faults during system operation
- **System Response Monitoring**: Track navigation performance, fault detection, and recovery
- **Flexible Scenario Definition**: JSON and CSV formats for scenario management
- **Comprehensive Reporting**: Detailed analysis with recommendations
- **Web Interface**: Real-time monitoring and control dashboard
- **Export Capabilities**: CSV and JSON data export for further analysis

### Use Cases

- **System Validation**: Verify navigation system robustness before deployment
- **Fault Tolerance Testing**: Assess system behavior under various failure conditions
- **Algorithm Development**: Test and improve fault detection and recovery algorithms
- **Compliance Testing**: Meet safety and reliability standards
- **Research & Development**: Investigate system behavior under controlled conditions

## System Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Dashboard │    │  Scenario       │    │  Test Results   │
│                 │    │  Manager        │    │  & Reports      │
│ • Real-time UI  │    │                 │    │                 │
│ • Controls      │◄──►│ • JSON/CSV      │◄──►│ • Performance   │
│ • Monitoring    │    │ • Validation    │    │ • Analysis      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │  Fault Injection│
                    │  Engine         │
                    │                 │
                    │ • Live faults   │
                    │ • Multi-type    │
                    │ • Timed control │
                    └─────────────────┘
                                 │
                    ┌─────────────────┐
                    │  System Monitor │
                    │                 │
                    │ • IMU data      │
                    │ • Navigation    │
                    │ • Performance   │
                    └─────────────────┘
```

## Getting Started

### Prerequisites

- Python 3.8+
- MELKENS HIL system configured
- Required Python packages (see requirements.txt)

### Installation

1. **Navigate to the backend directory**:
   ```bash
   cd web-simulator/backend
   ```

2. **Install dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

3. **Verify installation**:
   ```bash
   python3 test_scenarios.py
   ```

### Quick Start

1. **Start the HIL system**:
   ```bash
   python3 main.py
   ```

2. **Access the web interface**:
   Open http://localhost:8000 in your browser

3. **Load example scenarios**:
   ```bash
   curl -X POST http://localhost:8000/api/scenarios/upload \
        -F "file=@scenarios/example_scenarios.json"
   ```

4. **Run a test scenario**:
   ```bash
   curl -X POST http://localhost:8000/api/scenarios/basic_acc_stuck/run \
        -H "Content-Type: application/json" \
        -d '{"route_name": "A"}'
   ```

5. **View results**:
   ```bash
   curl http://localhost:8000/api/scenarios/results
   ```

## Scenario Definition

### Scenario Structure

Each fault injection scenario is defined with the following parameters:

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| `id` | string | Unique scenario identifier | `"acc_stuck_x"` |
| `name` | string | Human-readable name | `"Accelerometer X-Axis Stuck"` |
| `description` | string | Detailed description | `"X-axis freezes at 2.5 m/s² for 20s"` |
| `fault_type` | enum | Type of fault to inject | `"stuck_value"` |
| `severity` | enum | Impact severity level | `"medium"` |
| `target_sensor` | string | Target sensor | `"accelerometer"` |
| `target_axis` | string | Target axis (x/y/z/all) | `"x"` |
| `start_time` | float | Fault start time (seconds) | `10.0` |
| `duration` | float | Fault duration (seconds) | `30.0` |
| `parameters` | object | Fault-specific parameters | `{"stuck_value": 2.5}` |

### Fault Types

#### 1. Stuck Value (`stuck_value`)
Freezes sensor reading at a specific value.

**Parameters**:
- `stuck_value` (optional): Value to freeze at (defaults to current reading)

**Example**:
```json
{
  "fault_type": "stuck_value",
  "parameters": {"stuck_value": 2.5}
}
```

#### 2. Axis Loss (`axis_loss`)
Complete failure of sensor axis (reads zero).

**Parameters**: None

**Example**:
```json
{
  "fault_type": "axis_loss",
  "parameters": {}
}
```

#### 3. Bias Drift (`bias_drift`)
Gradual introduction of bias over time.

**Parameters**:
- `bias_rate`: Rate of bias increase (units/second)

**Example**:
```json
{
  "fault_type": "bias_drift",
  "parameters": {"bias_rate": 0.1}
}
```

#### 4. Noise Injection (`noise_injection`)
Adds random Gaussian noise to sensor readings.

**Parameters**:
- `noise_level`: Standard deviation of noise

**Example**:
```json
{
  "fault_type": "noise_injection",
  "parameters": {"noise_level": 0.5}
}
```

#### 5. Packet Loss (`packet_loss`)
Random loss of data packets.

**Parameters**:
- `loss_rate`: Probability of packet loss (0.0-1.0)

**Example**:
```json
{
  "fault_type": "packet_loss",
  "parameters": {"loss_rate": 0.2}
}
```

#### 6. Scale Error (`scale_error`)
Incorrect scaling factor applied to readings.

**Parameters**:
- `scale_factor`: Multiplier applied to values

**Example**:
```json
{
  "fault_type": "scale_error",
  "parameters": {"scale_factor": 1.5}
}
```

#### 7. Periodic Glitch (`periodic_glitch`)
Periodic disturbances in sensor data.

**Parameters**:
- `frequency`: Disturbance frequency (Hz)
- `amplitude`: Disturbance amplitude

**Example**:
```json
{
  "fault_type": "periodic_glitch",
  "parameters": {"frequency": 5.0, "amplitude": 1.0}
}
```

#### 8. Saturation (`saturation`)
Values clipped to minimum/maximum limits.

**Parameters**:
- `min_value`: Minimum allowed value
- `max_value`: Maximum allowed value

**Example**:
```json
{
  "fault_type": "saturation",
  "parameters": {"min_value": -8.0, "max_value": 8.0}
}
```

### JSON Format

```json
{
  "scenarios": [
    {
      "id": "example_scenario",
      "name": "Example Fault Scenario",
      "description": "Demonstrates basic fault injection",
      "fault_type": "noise_injection",
      "severity": "medium",
      "target_sensor": "accelerometer",
      "target_axis": "x",
      "start_time": 10.0,
      "duration": 30.0,
      "parameters": {
        "noise_level": 0.5
      }
    }
  ]
}
```

### CSV Format

```csv
id,name,description,fault_type,severity,target_sensor,target_axis,start_time,duration,parameters
example_scenario,Example Fault Scenario,Demonstrates basic fault injection,noise_injection,medium,accelerometer,x,10.0,30.0,"{""noise_level"": 0.5}"
```

## Running Tests

### Web Interface

1. **Access the Dashboard**: Navigate to http://localhost:8000
2. **Upload Scenarios**: Use the file upload interface to load JSON/CSV scenarios
3. **Select Scenario**: Choose from available scenarios
4. **Configure Test**: Set route and parameters
5. **Run Test**: Execute the fault injection test
6. **Monitor Progress**: View real-time test progress and metrics
7. **Review Results**: Analyze test outcomes and reports

### API Interface

#### Upload Scenarios
```bash
curl -X POST http://localhost:8000/api/scenarios/upload \
     -F "file=@my_scenarios.json"
```

#### List Available Scenarios
```bash
curl http://localhost:8000/api/scenarios
```

#### Run a Scenario Test
```bash
curl -X POST http://localhost:8000/api/scenarios/{scenario_id}/run \
     -H "Content-Type: application/json" \
     -d '{"route_name": "A"}'
```

#### Get Test Results
```bash
curl http://localhost:8000/api/scenarios/results
```

#### Generate Report
```bash
curl http://localhost:8000/api/scenarios/report
```

#### Export Results to CSV
```bash
curl http://localhost:8000/api/scenarios/export/csv \
     -o test_results.csv
```

### Command Line Interface

#### Run Comprehensive Tests
```bash
python3 test_scenarios.py
```

#### Load and Run Scenarios Programmatically
```python
from scenario_runner import AdvancedScenarioRunner

# Initialize runner
runner = AdvancedScenarioRunner()

# Load scenarios
runner.scenario_manager.load_scenarios_from_json("my_scenarios.json")

# Run a test
result = await runner.run_scenario("my_scenario_id", "A")

# Generate report
report = runner.generate_scenario_report()
```

## Analysis & Reporting

### Test Results

Each test execution produces a comprehensive result set:

```python
{
  "scenario_id": "acc_stuck_x",
  "start_time": "2024-01-15T10:30:00",
  "end_time": "2024-01-15T10:32:30", 
  "route_completed": True,
  "completion_percentage": 95.5,
  "fault_detected": True,
  "detection_time": 12.3,
  "recovery_achieved": True,
  "recovery_time": 8.7,
  "navigation_errors": [...],
  "performance_metrics": {
    "route_completion_percentage": 95.5,
    "total_distance_traveled": 48.2,
    "navigation_error_count": 2,
    "fault_detection_count": 1,
    "test_duration": 150.0,
    "avg_velocity": 0.32,
    "velocity_std": 0.045,
    "navigation_stability_score": 0.85
  }
}
```

### Performance Metrics

| Metric | Description | Good Range |
|--------|-------------|------------|
| `route_completion_percentage` | Percentage of route completed | > 90% |
| `fault_detection_count` | Number of faults detected | ≥ 1 for injected faults |
| `detection_time` | Time to detect fault (seconds) | < 5.0s |
| `recovery_achieved` | System recovered from fault | True |
| `recovery_time` | Time to recover (seconds) | < 10.0s |
| `navigation_stability_score` | Navigation consistency (0-1) | > 0.7 |
| `navigation_error_count` | Number of navigation errors | < 3 |

### Report Generation

#### Summary Report
```json
{
  "generated_at": "2024-01-15T11:00:00",
  "total_tests": 15,
  "summary": {
    "tests_passed": 12,
    "tests_failed": 3,
    "faults_detected": 14,
    "recoveries_achieved": 11,
    "avg_completion_rate": 87.3,
    "avg_detection_time": 4.2
  },
  "recommendations": [
    "System shows good fault tolerance",
    "Fault detection needs improvement for noise-type faults",
    "Recovery mechanisms working well"
  ]
}
```

#### Data Export

**CSV Export**: Contains all test results in tabular format for analysis in Excel, R, Python, or MATLAB.

**JSON Export**: Complete raw data including detailed sensor readings, fault injection timeline, and system responses.

## Best Practices

### Test Design

1. **Start Simple**: Begin with single-fault scenarios before complex multi-fault tests
2. **Systematic Coverage**: Test each fault type on each sensor and axis
3. **Severity Progression**: Test low, medium, high, and critical severity levels
4. **Timing Variation**: Vary fault injection timing relative to route execution
5. **Duration Testing**: Test both short bursts and extended fault periods

### Scenario Planning

1. **Realistic Faults**: Base scenarios on real-world failure modes
2. **Environmental Conditions**: Consider temperature, vibration, magnetic interference
3. **Mission Phases**: Test faults during different mission phases (startup, navigation, parking)
4. **Fault Combinations**: Test multiple simultaneous faults
5. **Edge Cases**: Test boundary conditions and extreme values

### Test Execution

1. **Baseline Testing**: Establish baseline performance without faults
2. **Controlled Environment**: Ensure consistent test conditions
3. **Repeatable Tests**: Run critical scenarios multiple times
4. **Documentation**: Record test conditions and observations
5. **Incremental Testing**: Build test complexity gradually

### Data Analysis

1. **Statistical Analysis**: Use multiple test runs for statistical validity
2. **Trend Analysis**: Look for patterns across fault types and severities
3. **Comparative Analysis**: Compare performance across different scenarios
4. **Threshold Setting**: Establish pass/fail criteria based on requirements
5. **Root Cause Analysis**: Investigate failure modes and system responses

### Safety Considerations

1. **Controlled Environment**: Perform tests in safe, controlled conditions
2. **Emergency Stops**: Implement emergency stop mechanisms
3. **Monitoring**: Continuously monitor system status during tests
4. **Fault Isolation**: Ensure test faults don't affect real operations
5. **Recovery Procedures**: Have clear procedures for test recovery

## Example Test Scenarios

### Basic Validation Suite

#### Scenario 1: Accelerometer Stuck Value
**Objective**: Test system response to frozen accelerometer reading
```json
{
  "id": "acc_stuck_basic",
  "name": "Basic Accelerometer Stuck Test",
  "description": "X-axis accelerometer freezes at 2.5 m/s² during navigation",
  "fault_type": "stuck_value",
  "severity": "medium",
  "target_sensor": "accelerometer",
  "target_axis": "x",
  "start_time": 10.0,
  "duration": 20.0,
  "parameters": {"stuck_value": 2.5}
}
```

**Expected Results**:
- Route completion: > 80%
- Fault detection: < 5 seconds
- Recovery: < 10 seconds

#### Scenario 2: High Gyroscope Noise
**Objective**: Test navigation stability under high sensor noise
```json
{
  "id": "gyro_noise_high",
  "name": "High Gyroscope Noise Test", 
  "description": "High-level noise injection on all gyroscope axes",
  "fault_type": "noise_injection",
  "severity": "high",
  "target_sensor": "gyroscope",
  "target_axis": "all",
  "start_time": 15.0,
  "duration": 30.0,
  "parameters": {"noise_level": 0.8}
}
```

**Expected Results**:
- Navigation stability: > 0.6
- Route completion: > 70%
- Error count: < 5

### Advanced Test Suite

#### Scenario 3: Critical Packet Loss
**Objective**: Test system behavior under severe data loss
```json
{
  "id": "packet_loss_critical",
  "name": "Critical Packet Loss Test",
  "description": "30% packet loss affecting all IMU data",
  "fault_type": "packet_loss",
  "severity": "critical", 
  "target_sensor": "accelerometer",
  "target_axis": "all",
  "start_time": 5.0,
  "duration": 45.0,
  "parameters": {"loss_rate": 0.3}
}
```

**Expected Results**:
- Route completion: > 60%
- Fault detection: < 3 seconds
- System should attempt recovery

#### Scenario 4: Magnetometer Bias Drift
**Objective**: Test long-term bias compensation
```json
{
  "id": "mag_bias_drift_long",
  "name": "Long-term Magnetometer Bias Drift",
  "description": "Gradual Z-axis bias drift over extended period",
  "fault_type": "bias_drift",
  "severity": "medium",
  "target_sensor": "magnetometer", 
  "target_axis": "z",
  "start_time": 5.0,
  "duration": 120.0,
  "parameters": {"bias_rate": 0.05}
}
```

**Expected Results**:
- Route completion: > 85%
- Navigation adapts to bias
- Heading accuracy maintained

### Stress Test Suite

#### Scenario 5: Multi-Sensor Failure
**Objective**: Test system resilience to multiple simultaneous failures
```json
{
  "scenarios": [
    {
      "id": "multi_fault_acc_stuck",
      "fault_type": "stuck_value",
      "target_sensor": "accelerometer",
      "target_axis": "x",
      "start_time": 10.0,
      "duration": 30.0
    },
    {
      "id": "multi_fault_gyro_noise", 
      "fault_type": "noise_injection",
      "target_sensor": "gyroscope",
      "target_axis": "all",
      "start_time": 15.0,
      "duration": 25.0,
      "parameters": {"noise_level": 0.6}
    }
  ]
}
```

**Expected Results**:
- Route completion: > 50%
- System demonstrates graceful degradation
- Critical errors handled appropriately

### Environmental Test Suite

#### Scenario 6: Temperature Simulation
**Objective**: Simulate temperature-induced sensor drift
```json
{
  "id": "temp_drift_simulation",
  "name": "Temperature Drift Simulation",
  "description": "Simulated temperature-induced bias on all sensors",
  "fault_type": "bias_drift",
  "severity": "low",
  "target_sensor": "accelerometer",
  "target_axis": "all",
  "start_time": 0.0,
  "duration": 180.0,
  "parameters": {"bias_rate": 0.002}
}
```

#### Scenario 7: Vibration Simulation
**Objective**: Test behavior under mechanical vibration
```json
{
  "id": "vibration_test",
  "name": "Mechanical Vibration Test",
  "description": "High-frequency vibration affecting accelerometer",
  "fault_type": "periodic_glitch",
  "severity": "medium",
  "target_sensor": "accelerometer", 
  "target_axis": "all",
  "start_time": 20.0,
  "duration": 30.0,
  "parameters": {"frequency": 25.0, "amplitude": 0.3}
}
```

### Compliance Test Suite

#### Scenario 8: Safety Critical Test
**Objective**: Verify behavior under safety-critical failures
```json
{
  "id": "safety_critical_test",
  "name": "Safety Critical Sensor Failure",
  "description": "Complete loss of primary navigation sensor",
  "fault_type": "axis_loss",
  "severity": "critical",
  "target_sensor": "magnetometer",
  "target_axis": "all", 
  "start_time": 25.0,
  "duration": 60.0,
  "parameters": {}
}
```

**Expected Results**:
- System enters safe mode
- Navigation stops or uses backup sensors
- Error condition clearly indicated

## API Reference

### Scenario Management

#### `GET /api/scenarios`
List all available fault injection scenarios.

**Response**:
```json
{
  "status": "success",
  "scenarios": [...],
  "count": 15
}
```

#### `POST /api/scenarios`
Create a new fault injection scenario.

**Request**:
```json
{
  "id": "new_scenario",
  "name": "New Test Scenario",
  "description": "...",
  "fault_type": "noise_injection",
  "severity": "medium",
  "target_sensor": "accelerometer",
  "target_axis": "x",
  "start_time": 10.0,
  "duration": 30.0,
  "parameters": {"noise_level": 0.5}
}
```

#### `GET /api/scenarios/{scenario_id}`
Get details of a specific scenario.

#### `POST /api/scenarios/upload`
Upload scenario file (JSON or CSV).

**Request**: Multipart form with file upload

### Test Execution

#### `POST /api/scenarios/{scenario_id}/run`
Run a fault injection scenario test.

**Request**:
```json
{
  "route_name": "A"
}
```

**Response**:
```json
{
  "status": "success", 
  "result": {
    "scenario_id": "test_scenario",
    "route_completed": true,
    "completion_percentage": 95.5,
    "fault_detected": true,
    "performance_metrics": {...}
  }
}
```

### Results & Reporting

#### `GET /api/scenarios/results`
Get all test results.

#### `GET /api/scenarios/report`
Generate comprehensive test report.

**Query Parameters**:
- `scenario_ids`: Comma-separated list of scenario IDs (optional)

#### `GET /api/scenarios/export/csv`
Export test results to CSV file.

### System Information

#### `GET /api/scenarios/fault-types`
Get available fault types and their descriptions.

**Response**:
```json
{
  "status": "success",
  "fault_types": {
    "stuck_value": {
      "name": "Stuck Value",
      "description": "Sensor value freezes at current reading",
      "parameters": ["stuck_value (optional)"]
    },
    ...
  },
  "severities": {
    "low": "Minor impact on system performance",
    "medium": "Moderate impact, system should handle gracefully",
    "high": "Significant impact, may cause navigation issues", 
    "critical": "Severe impact, system failure expected"
  }
}
```

### WebSocket Interface

#### `ws://localhost:8000/ws/scenarios`
Real-time scenario monitoring WebSocket.

**Message Format**:
```json
{
  "type": "scenario_status",
  "data": {
    "test_id": "test_acc_stuck_x_1642248600",
    "is_running": true,
    "active_faults": 1,
    "route_progress": 0.45,
    "fault_detections": 1,
    "navigation_errors": 0,
    "performance_metrics": {...}
  }
}
```

## Troubleshooting

### Common Issues

#### Test Execution Fails
**Symptoms**: Scenario test fails to start or complete
**Solutions**:
1. Check scenario definition syntax
2. Verify target sensor/axis exists
3. Ensure sufficient test duration
4. Check for conflicting fault parameters

#### No Fault Detection
**Symptoms**: Faults injected but not detected by system
**Solutions**:
1. Increase fault severity or duration
2. Check fault detection thresholds
3. Verify fault is actually being applied
4. Review fault detection algorithms

#### Poor Navigation Performance
**Symptoms**: Route completion rates consistently low
**Solutions**:
1. Start with lower severity faults
2. Check baseline performance without faults
3. Verify route configuration
4. Review system tuning parameters

#### Data Export Issues
**Symptoms**: CSV/JSON export fails or incomplete
**Solutions**:
1. Check file permissions
2. Ensure sufficient disk space
3. Verify data logger configuration
4. Check for special characters in data

### Debug Tools

#### Enable Debug Logging
```python
import logging
logging.basicConfig(level=logging.DEBUG)
```

#### Scenario Validation
```bash
python3 -c "
from scenario_runner import FaultScenario, FaultType, FaultSeverity
scenario = FaultScenario.from_dict(scenario_dict)
print('Scenario valid:', scenario.id)
"
```

#### Test Individual Components
```bash
python3 test_scenarios.py
```

### Performance Tuning

#### Optimize Test Duration
- Reduce test duration for development
- Use full duration for validation
- Balance test coverage vs. execution time

#### Memory Management
- Clear old test results periodically
- Limit number of concurrent tests
- Monitor system resource usage

#### Network Optimization
- Use local file uploads for large scenario sets
- Implement test result caching
- Optimize WebSocket update frequency

### Support Resources

- **Documentation**: This guide and HIL_IMU_GUIDE.md
- **Example Files**: scenarios/example_scenarios.json and .csv
- **Test Suite**: test_scenarios.py for component validation
- **API Documentation**: Interactive docs at http://localhost:8000/docs
- **Log Files**: Check logs/ directory for detailed execution logs

---

**Document Version**: 1.0  
**Last Updated**: January 2024  
**Compatible Framework**: MELKENS HIL v1.3+