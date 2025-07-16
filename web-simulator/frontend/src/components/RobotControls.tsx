import React, { useState } from 'react'
import { Play, Square, AlertTriangle, RotateCcw, Loader2 } from 'lucide-react'
import { RobotState } from '@melkens/shared'
import { useRobotControl, useRoutes } from '@/hooks/useApi'
import { useWebSocket } from '@/hooks/useWebSocket'

interface RobotControlsProps {
  robotState: RobotState
}

export const RobotControls: React.FC<RobotControlsProps> = ({ robotState }) => {
  const [selectedRouteId, setSelectedRouteId] = useState<number>(0)
  const { data: routes = [] } = useRoutes()
  const { sendCommand } = useWebSocket()
  
  const {
    startRoute,
    stopRobot,
    emergencyStop,
    resetPosition
  } = useRobotControl()

  const handleStartRoute = () => {
    // Try WebSocket first, fallback to REST API
    sendCommand({
      type: 'start_route',
      data: { routeId: selectedRouteId }
    })
    
    // Also call REST API for reliability
    startRoute.mutate(selectedRouteId)
  }

  const handleStopRobot = () => {
    sendCommand({ type: 'stop_robot' })
    stopRobot.mutate()
  }

  const handleEmergencyStop = () => {
    sendCommand({ type: 'emergency_stop' })
    emergencyStop.mutate()
  }

  const handleResetPosition = () => {
    sendCommand({ type: 'reset_position' })
    resetPosition.mutate()
  }

  const isLoading = startRoute.isPending || stopRobot.isPending || 
                   emergencyStop.isPending || resetPosition.isPending

  return (
    <div className="space-y-4">
      {/* Route Selection */}
      <div>
        <label className="form-label">Select Route</label>
        <select
          value={selectedRouteId}
          onChange={(e) => setSelectedRouteId(Number(e.target.value))}
          className="form-input"
          disabled={robotState.isRunning || isLoading}
        >
          {routes.map((route) => (
            <option key={route.id} value={route.id}>
              {route.name} ({route.steps.length} steps)
            </option>
          ))}
        </select>
      </div>

      {/* Main Controls */}
      <div className="grid grid-cols-2 gap-3">
        <button
          onClick={handleStartRoute}
          disabled={robotState.isRunning || isLoading || routes.length === 0}
          className="btn btn-success flex items-center justify-center space-x-2"
        >
          {startRoute.isPending ? (
            <Loader2 className="animate-spin" size={16} />
          ) : (
            <Play size={16} />
          )}
          <span>Start Route</span>
        </button>

        <button
          onClick={handleStopRobot}
          disabled={!robotState.isRunning || isLoading}
          className="btn btn-secondary flex items-center justify-center space-x-2"
        >
          {stopRobot.isPending ? (
            <Loader2 className="animate-spin" size={16} />
          ) : (
            <Square size={16} />
          )}
          <span>Stop</span>
        </button>
      </div>

      {/* Emergency and Utility Controls */}
      <div className="grid grid-cols-2 gap-3">
        <button
          onClick={handleEmergencyStop}
          disabled={isLoading}
          className="btn btn-danger flex items-center justify-center space-x-2"
        >
          {emergencyStop.isPending ? (
            <Loader2 className="animate-spin" size={16} />
          ) : (
            <AlertTriangle size={16} />
          )}
          <span>E-Stop</span>
        </button>

        <button
          onClick={handleResetPosition}
          disabled={robotState.isRunning || isLoading}
          className="btn btn-outline flex items-center justify-center space-x-2"
        >
          {resetPosition.isPending ? (
            <Loader2 className="animate-spin" size={16} />
          ) : (
            <RotateCcw size={16} />
          )}
          <span>Reset Pos</span>
        </button>
      </div>

      {/* Current Status */}
      <div className="p-3 bg-gray-50 rounded-lg text-sm">
        <div className="flex items-center justify-between mb-2">
          <span className="font-medium">Current Status:</span>
          <div className={`
            px-2 py-1 rounded text-xs font-medium
            ${robotState.isRunning 
              ? 'bg-green-100 text-green-800' 
              : 'bg-gray-100 text-gray-800'
            }
          `}>
            {robotState.isRunning ? 'Running' : 'Idle'}
          </div>
        </div>

        {robotState.currentRoute !== undefined && (
          <div className="space-y-1 text-gray-600">
            <div>Route: {routes.find(r => r.id === robotState.currentRoute)?.name || robotState.currentRoute}</div>
            {robotState.currentStep !== undefined && (
              <div>Step: {robotState.currentStep + 1} / {routes.find(r => r.id === robotState.currentRoute)?.steps.length || '?'}</div>
            )}
          </div>
        )}

        {robotState.errors.length > 0 && (
          <div className="mt-2 p-2 bg-red-50 rounded text-red-600 text-xs">
            <div className="font-medium mb-1">Active Errors:</div>
            {robotState.errors.map((error, index) => (
              <div key={index}>• {error}</div>
            ))}
          </div>
        )}
      </div>

      {/* Quick Actions */}
      <div className="text-xs text-gray-500 space-y-1">
        <div>• Select a route and click "Start Route" to begin</div>
        <div>• Use "E-Stop" for immediate halt in emergencies</div>
        <div>• "Reset Pos" moves robot back to center</div>
      </div>
    </div>
  )
}