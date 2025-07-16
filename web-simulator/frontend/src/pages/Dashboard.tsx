import React from 'react'
import { RobotVisualization } from '@/components/RobotVisualization'
import { RobotControls } from '@/components/RobotControls'
import { MetricsPanel } from '@/components/MetricsPanel'
import { RecentLogs } from '@/components/RecentLogs'
import { RouteProgress } from '@/components/RouteProgress'
import { useWebSocket } from '@/hooks/useWebSocket'
import { useRobotState } from '@/hooks/useApi'
import { AlertCircle, Loader2 } from 'lucide-react'

export const Dashboard: React.FC = () => {
  const { robotState: wsRobotState, isConnected } = useWebSocket()
  const { data: apiRobotState, isLoading, error } = useRobotState()

  // Use WebSocket data if available, otherwise fall back to API data
  const robotState = isConnected ? wsRobotState : apiRobotState

  if (isLoading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="flex items-center space-x-2">
          <Loader2 className="animate-spin" size={20} />
          <span>Loading robot state...</span>
        </div>
      </div>
    )
  }

  if (error && !isConnected) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="flex items-center space-x-2 text-red-600">
          <AlertCircle size={20} />
          <span>Failed to load robot state</span>
        </div>
      </div>
    )
  }

  if (!robotState) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-gray-500">No robot data available</div>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      {/* Status Banner */}
      <div className={`
        p-4 rounded-lg border-l-4 
        ${robotState.isRunning 
          ? 'bg-green-50 border-green-400 text-green-800' 
          : 'bg-gray-50 border-gray-400 text-gray-800'
        }
      `}>
        <div className="flex items-center justify-between">
          <div className="flex items-center space-x-2">
            <div className={`
              w-3 h-3 rounded-full 
              ${robotState.isRunning ? 'bg-green-500 animate-pulse' : 'bg-gray-400'}
            `} />
            <span className="font-medium">
              Robot Status: {robotState.isRunning ? 'Running' : 'Idle'}
            </span>
          </div>
          
          {robotState.currentRoute !== undefined && (
            <div className="text-sm">
              Executing Route {robotState.currentRoute}
              {robotState.currentStep !== undefined && ` - Step ${robotState.currentStep + 1}`}
            </div>
          )}
        </div>
        
        {robotState.errors.length > 0 && (
          <div className="mt-2 text-sm text-red-600">
            Active Errors: {robotState.errors.join(', ')}
          </div>
        )}
      </div>

      {/* Main Dashboard Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Robot Visualization - Takes up 2 columns on large screens */}
        <div className="lg:col-span-2">
          <div className="card">
            <div className="card-header">
              <h3 className="text-lg font-semibold">Robot Position & Map</h3>
            </div>
            <div className="card-content">
              <RobotVisualization robotState={robotState} />
            </div>
          </div>
        </div>

        {/* Right Column - Controls and Info */}
        <div className="space-y-6">
          {/* Robot Controls */}
          <div className="card">
            <div className="card-header">
              <h3 className="text-lg font-semibold">Robot Controls</h3>
            </div>
            <div className="card-content">
              <RobotControls robotState={robotState} />
            </div>
          </div>

          {/* Metrics Panel */}
          <div className="card">
            <div className="card-header">
              <h3 className="text-lg font-semibold">Live Metrics</h3>
            </div>
            <div className="card-content">
              <MetricsPanel robotState={robotState} />
            </div>
          </div>
        </div>
      </div>

      {/* Bottom Row */}
      <div className="grid grid-cols-1 xl:grid-cols-2 gap-6">
        {/* Route Progress */}
        <div className="card">
          <div className="card-header">
            <h3 className="text-lg font-semibold">Current Route Progress</h3>
          </div>
          <div className="card-content">
            <RouteProgress robotState={robotState} />
          </div>
        </div>

        {/* Recent Logs */}
        <div className="card">
          <div className="card-header">
            <h3 className="text-lg font-semibold">Recent Activity</h3>
          </div>
          <div className="card-content">
            <RecentLogs limit={10} />
          </div>
        </div>
      </div>
    </div>
  )
}