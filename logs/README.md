# MELKENS HIL Data Logs

This directory contains data logs from the HIL (Hardware-in-the-Loop) simulator system.

## Log Files

### `hil_imu_test.csv`
Main data log containing timestamped IMU sensor data and robot state information.

**Columns:**
- `timestamp` - Unix timestamp (seconds since epoch)
- `iso_time` - ISO 8601 formatted timestamp
- `data_source` - "hardware" or "simulated"
- `imu_connected` - Boolean indicating hardware connection
- `fault_injection_enabled` - Boolean indicating fault injection status
- `fault_type` - Type of fault if enabled ("none", "drift", "stuck_axis", "noise", "missing_packets")
- `acc_x`, `acc_y`, `acc_z` - Accelerometer readings (m/s²)
- `gyro_x`, `gyro_y`, `gyro_z` - Gyroscope readings (rad/s)
- `mag_x`, `mag_y`, `mag_z` - Magnetometer readings (µT)
- `temperature` - IMU temperature (°C)
- `quat_w`, `quat_x`, `quat_y`, `quat_z` - Quaternion orientation
- `roll`, `pitch`, `yaw` - Euler angles (radians)
- `pos_x`, `pos_y`, `pos_z` - Position estimates (m)
- `vel_x`, `vel_y`, `vel_z` - Velocity estimates (m/s)
- `robot_route` - Current route ID ("A", "B", "C", "D", or "None")
- `robot_pos_x`, `robot_pos_y` - Robot position (m)
- `robot_angle` - Robot heading angle (radians)
- `motor_left_speed`, `motor_right_speed` - Motor speeds (-1500 to 1500)
- `packet_count` - Total packets received
- `error_count` - Total communication errors
- `data_rate` - Current data rate (Hz)

### `test_report.json`
Test results from the system validation script.

## Data Analysis

### Python Analysis Example

```python
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load data
df = pd.read_csv('hil_imu_test.csv')

# Convert timestamp to datetime
df['datetime'] = pd.to_datetime(df['timestamp'], unit='s')

# Basic statistics
print(f"Total samples: {len(df)}")
print(f"Duration: {(df['timestamp'].max() - df['timestamp'].min()):.1f} seconds")
print(f"Average data rate: {len(df) / (df['timestamp'].max() - df['timestamp'].min()):.1f} Hz")

# Plot accelerometer data
plt.figure(figsize=(12, 8))
plt.subplot(3, 1, 1)
plt.plot(df['datetime'], df['acc_x'], label='X', alpha=0.7)
plt.plot(df['datetime'], df['acc_y'], label='Y', alpha=0.7)
plt.plot(df['datetime'], df['acc_z'], label='Z', alpha=0.7)
plt.ylabel('Acceleration (m/s²)')
plt.legend()
plt.title('IMU Accelerometer Data')

# Plot gyroscope data
plt.subplot(3, 1, 2)
plt.plot(df['datetime'], df['gyro_x'], label='X', alpha=0.7)
plt.plot(df['datetime'], df['gyro_y'], label='Y', alpha=0.7)
plt.plot(df['datetime'], df['gyro_z'], label='Z', alpha=0.7)
plt.ylabel('Angular Velocity (rad/s)')
plt.legend()
plt.title('IMU Gyroscope Data')

# Plot robot path
plt.subplot(3, 1, 3)
plt.plot(df['robot_pos_x'], df['robot_pos_y'], 'b-', alpha=0.7)
plt.scatter(df['robot_pos_x'].iloc[0], df['robot_pos_y'].iloc[0], 
           c='green', s=100, label='Start')
plt.scatter(df['robot_pos_x'].iloc[-1], df['robot_pos_y'].iloc[-1], 
           c='red', s=100, label='End')
plt.xlabel('X Position (m)')
plt.ylabel('Y Position (m)')
plt.legend()
plt.title('Robot Path')
plt.axis('equal')

plt.tight_layout()
plt.show()

# Analysis by route
for route in df['robot_route'].unique():
    if route != 'None':
        route_data = df[df['robot_route'] == route]
        print(f"\nRoute {route}:")
        print(f"  Duration: {(route_data['timestamp'].max() - route_data['timestamp'].min()):.1f}s")
        print(f"  Distance: {np.sum(np.sqrt(np.diff(route_data['robot_pos_x'])**2 + 
                                          np.diff(route_data['robot_pos_y'])**2)):.2f}m")

# Error analysis
if df['error_count'].max() > 0:
    print(f"\nError Analysis:")
    print(f"  Total errors: {df['error_count'].max()}")
    print(f"  Error rate: {df['error_count'].max() / len(df) * 100:.2f}%")

# Fault injection analysis
fault_data = df[df['fault_injection_enabled'] == True]
if len(fault_data) > 0:
    print(f"\nFault Injection:")
    print(f"  Samples with faults: {len(fault_data)}")
    print(f"  Fault types: {fault_data['fault_type'].unique()}")
```

### MATLAB Analysis Example

```matlab
% Load data
data = readtable('hil_imu_test.csv');

% Convert timestamp to datetime
data.datetime = datetime(data.timestamp, 'ConvertFrom', 'posixtime');

% Plot IMU data
figure;
subplot(3,1,1);
plot(data.datetime, data.acc_x, data.datetime, data.acc_y, data.datetime, data.acc_z);
ylabel('Acceleration (m/s²)');
legend('X', 'Y', 'Z');
title('Accelerometer Data');

subplot(3,1,2);
plot(data.datetime, data.gyro_x, data.datetime, data.gyro_y, data.datetime, data.gyro_z);
ylabel('Angular Velocity (rad/s)');
legend('X', 'Y', 'Z');
title('Gyroscope Data');

subplot(3,1,3);
plot(data.robot_pos_x, data.robot_pos_y);
xlabel('X Position (m)');
ylabel('Y Position (m)');
title('Robot Path');
axis equal;

% Statistics
fprintf('Total samples: %d\n', height(data));
fprintf('Duration: %.1f seconds\n', seconds(data.datetime(end) - data.datetime(1)));
fprintf('Average data rate: %.1f Hz\n', height(data) / seconds(data.datetime(end) - data.datetime(1)));
```

## File Management

### Automatic Cleanup
The system automatically manages log file sizes. Large files are rotated to prevent disk space issues.

### Manual Cleanup
To manually clean old logs:
```bash
# Remove logs older than 7 days
find . -name "*.csv" -mtime +7 -delete

# Compress old logs
gzip hil_imu_test_*.csv
```

### Backup
For important test data:
```bash
# Create dated backup
cp hil_imu_test.csv "hil_imu_test_$(date +%Y%m%d_%H%M%S).csv"

# Archive to separate location
tar -czf backup_$(date +%Y%m%d).tar.gz *.csv *.json
```

## Data Quality

### Expected Ranges
- **Accelerometer**: ±16g (±156.8 m/s²)
- **Gyroscope**: ±2000°/s (±34.9 rad/s)
- **Magnetometer**: ±50 gauss (±5000 µT)
- **Temperature**: 0°C to 60°C
- **Data Rate**: 10-100 Hz

### Quality Indicators
- Packet error rate < 1%
- Sensor noise within specifications
- No stuck values for extended periods
- Reasonable gravity vector in accelerometer

### Troubleshooting Data Issues
- **High noise**: Check connections, reduce EMI
- **Missing data**: Verify baud rate, check cables
- **Stuck values**: Reset IMU, check firmware
- **Low rate**: Check CPU usage, reduce other processes