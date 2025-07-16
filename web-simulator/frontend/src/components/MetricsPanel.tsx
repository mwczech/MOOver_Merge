import React from 'react'
import { Battery, Thermometer, Navigation, Gauge, Cpu, Wifi } from 'lucide-react'
import { RobotState } from '@melkens/shared'

interface MetricsPanelProps {
  robotState: RobotState
}

export const MetricsPanel: React.FC<MetricsPanelProps> = ({ robotState }) => {
  const { sensors, motors, position } = robotState

  const getBatteryColor = (voltage: number) => {
    if (voltage > 23) return 'text-green-600'
    if (voltage > 21) return 'text-yellow-600'
    return 'text-red-600'
  }

  const getBatteryPercentage = (voltage: number) => {
    // Assuming 24V nominal, 20V minimum, 25V maximum
    return Math.min(Math.max(((voltage - 20) / 5) * 100, 0), 100)
  }

  const getTemperatureColor = (temp: number) => {
    if (temp < 30) return 'text-blue-600'
    if (temp < 40) return 'text-green-600'
    if (temp < 50) return 'text-yellow-600'
    return 'text-red-600'
  }

  const formatSpeed = (leftSpeed: number, rightSpeed: number) => {
    const avgSpeed = (Math.abs(leftSpeed) + Math.abs(rightSpeed)) / 2
    return Math.round(avgSpeed)
  }

  return (
    <div className="space-y-4">
      {/* Battery Status */}
      <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2">
          <Battery className={getBatteryColor(sensors.batteryVoltage)} size={20} />
          <span className="font-medium">Battery</span>
        </div>
        <div className="text-right">
          <div className={`font-mono text-lg ${getBatteryColor(sensors.batteryVoltage)}`}>
            {sensors.batteryVoltage.toFixed(1)}V
          </div>
          <div className="text-xs text-gray-500">
            {getBatteryPercentage(sensors.batteryVoltage).toFixed(0)}%
          </div>
        </div>
      </div>

      {/* Temperature */}
      <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2">
          <Thermometer className={getTemperatureColor(sensors.temperature)} size={20} />
          <span className="font-medium">Temperature</span>
        </div>
        <div className="text-right">
          <div className={`font-mono text-lg ${getTemperatureColor(sensors.temperature)}`}>
            {sensors.temperature.toFixed(1)}°C
          </div>
        </div>
      </div>

      {/* Position & Navigation */}
      <div className="p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2 mb-2">
          <Navigation className="text-blue-600" size={20} />
          <span className="font-medium">Position</span>
        </div>
        <div className="space-y-1 text-sm font-mono">
          <div>X: {Math.round(position.x)} mm</div>
          <div>Y: {Math.round(position.y)} mm</div>
          <div>Angle: {Math.round(position.angle)}°</div>
        </div>
      </div>

      {/* Motor Status */}
      <div className="p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2 mb-2">
          <Gauge className={motors.isRunning ? 'text-green-600' : 'text-gray-600'} size={20} />
          <span className="font-medium">Motors</span>
        </div>
        <div className="space-y-2">
          <div className="flex justify-between text-sm">
            <span>Left:</span>
            <span className="font-mono">{motors.leftSpeed} rpm</span>
          </div>
          <div className="flex justify-between text-sm">
            <span>Right:</span>
            <span className="font-mono">{motors.rightSpeed} rpm</span>
          </div>
          <div className="flex justify-between text-sm">
            <span>Speed:</span>
            <span className="font-mono">{formatSpeed(motors.leftSpeed, motors.rightSpeed)} mm/s</span>
          </div>
          <div className="flex justify-between text-sm">
            <span>Status:</span>
            <span className={`
              px-2 py-1 rounded text-xs font-medium
              ${motors.isRunning ? 'bg-green-100 text-green-800' : 'bg-gray-100 text-gray-800'}
            `}>
              {motors.isRunning ? 'Running' : 'Stopped'}
            </span>
          </div>
        </div>
      </div>

      {/* Magnetic Sensor */}
      <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2">
          <Cpu className="text-purple-600" size={20} />
          <span className="font-medium">Magnetic</span>
        </div>
        <div className="text-right">
          <div className="font-mono text-lg text-purple-600">
            {sensors.magneticPosition.toFixed(1)}
          </div>
          <div className="text-xs text-gray-500">Position</div>
        </div>
      </div>

      {/* Sensor Connection Status */}
      <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2">
          <Wifi className={sensors.isConnected ? 'text-green-600' : 'text-red-600'} size={20} />
          <span className="font-medium">Sensors</span>
        </div>
        <div className={`
          px-2 py-1 rounded text-xs font-medium
          ${sensors.isConnected ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'}
        `}>
          {sensors.isConnected ? 'Connected' : 'Disconnected'}
        </div>
      </div>

      {/* Encoders */}
      <div className="p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2 mb-2">
          <Gauge className="text-blue-600" size={20} />
          <span className="font-medium">Encoders</span>
        </div>
        <div className="space-y-1 text-sm">
          <div className="flex justify-between">
            <span>Left:</span>
            <span className="font-mono">{Math.round(sensors.encoderLeft)} ticks</span>
          </div>
          <div className="flex justify-between">
            <span>Right:</span>
            <span className="font-mono">{Math.round(sensors.encoderRight)} ticks</span>
          </div>
          <div className="flex justify-between">
            <span>Diff:</span>
            <span className="font-mono">{Math.round(Math.abs(sensors.encoderLeft - sensors.encoderRight))} ticks</span>
          </div>
        </div>
      </div>

      {/* IMU */}
      <div className="flex items-center justify-between p-3 bg-gray-50 rounded-lg">
        <div className="flex items-center space-x-2">
          <Navigation className="text-indigo-600" size={20} />
          <span className="font-medium">IMU Angle</span>
        </div>
        <div className="text-right">
          <div className="font-mono text-lg text-indigo-600">
            {sensors.imuAngle.toFixed(1)}°
          </div>
        </div>
      </div>

      {/* System Uptime */}
      <div className="p-3 bg-gray-50 rounded-lg text-center">
        <div className="text-xs text-gray-500 mb-1">Last Update</div>
        <div className="text-sm font-mono">
          {new Date(robotState.lastUpdate).toLocaleTimeString()}
        </div>
      </div>
    </div>
  )
}