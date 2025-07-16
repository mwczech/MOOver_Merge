import React from 'react'
import { ArrowLeft, Edit, Play, ArrowRight, TurnLeft, TurnRight } from 'lucide-react'
import { Route, OperationType } from '@melkens/shared'

interface RouteDetailsProps {
  route: Route
  onBack: () => void
  onEdit: () => void
  onExecute: () => void
}

export const RouteDetails: React.FC<RouteDetailsProps> = ({
  route,
  onBack,
  onEdit,
  onExecute
}) => {
  const getOperationIcon = (operation: OperationType) => {
    switch (operation) {
      case OperationType.L_90:
        return <TurnLeft size={20} className="text-blue-600" />
      case OperationType.R_90:
        return <TurnRight size={20} className="text-blue-600" />
      case OperationType.TU_L:
        return <TurnLeft size={20} className="text-purple-600" />
      case OperationType.TU_R:
        return <TurnRight size={20} className="text-purple-600" />
      default:
        return <ArrowRight size={20} className="text-green-600" />
    }
  }

  const getOperationName = (operation: OperationType) => {
    switch (operation) {
      case OperationType.NORM: return 'Normal Movement'
      case OperationType.TU_L: return 'Turn Left'
      case OperationType.TU_R: return 'Turn Right'
      case OperationType.L_90: return '90° Left Turn'
      case OperationType.R_90: return '90° Right Turn'
      case OperationType.DIFF: return 'Differential'
      case OperationType.NORM_NOMAGNET: return 'No Magnet'
      default: return 'Unknown'
    }
  }

  const getOperationColor = (operation: OperationType) => {
    switch (operation) {
      case OperationType.L_90:
      case OperationType.R_90:
        return 'bg-blue-50 border-blue-200 text-blue-800'
      case OperationType.TU_L:
      case OperationType.TU_R:
        return 'bg-purple-50 border-purple-200 text-purple-800'
      case OperationType.DIFF:
        return 'bg-yellow-50 border-yellow-200 text-yellow-800'
      case OperationType.NORM_NOMAGNET:
        return 'bg-red-50 border-red-200 text-red-800'
      default:
        return 'bg-green-50 border-green-200 text-green-800'
    }
  }

  const totalDistance = route.steps.reduce((sum, step) => sum + step.distance, 0)
  const avgSpeed = route.steps.reduce((sum, step) => sum + step.speed, 0) / route.steps.length
  const estimatedTime = route.steps.reduce((sum, step) => sum + (step.distance / step.speed), 0)

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div className="flex items-center space-x-4">
          <button
            onClick={onBack}
            className="btn btn-outline flex items-center space-x-2"
          >
            <ArrowLeft size={16} />
            <span>Back to Routes</span>
          </button>
          <div>
            <h1 className="text-2xl font-bold text-gray-900">{route.name}</h1>
            <p className="text-gray-600">Route ID: {route.id}</p>
          </div>
        </div>
        <div className="flex items-center space-x-3">
          <button
            onClick={onEdit}
            disabled={route.isActive}
            className="btn btn-secondary flex items-center space-x-2"
          >
            <Edit size={16} />
            <span>Edit Route</span>
          </button>
          <button
            onClick={onExecute}
            disabled={route.isActive}
            className="btn btn-success flex items-center space-x-2"
          >
            <Play size={16} />
            <span>Execute Route</span>
          </button>
        </div>
      </div>

      {/* Status Banner */}
      <div className={`
        p-4 rounded-lg border-l-4 
        ${route.isActive 
          ? 'bg-green-50 border-green-400 text-green-800' 
          : 'bg-gray-50 border-gray-400 text-gray-800'
        }
      `}>
        <div className="flex items-center space-x-2">
          <div className={`
            w-3 h-3 rounded-full 
            ${route.isActive ? 'bg-green-500 animate-pulse' : 'bg-gray-400'}
          `} />
          <span className="font-medium">
            Status: {route.isActive ? 'Currently Executing' : 'Inactive'}
          </span>
        </div>
      </div>

      {/* Route Summary */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-6">
        <div className="card">
          <div className="card-content text-center">
            <div className="text-2xl font-bold text-blue-600">{route.steps.length}</div>
            <div className="text-sm text-gray-600">Total Steps</div>
          </div>
        </div>
        <div className="card">
          <div className="card-content text-center">
            <div className="text-2xl font-bold text-green-600">
              {totalDistance.toLocaleString()}
            </div>
            <div className="text-sm text-gray-600">Distance (mm)</div>
          </div>
        </div>
        <div className="card">
          <div className="card-content text-center">
            <div className="text-2xl font-bold text-purple-600">
              {Math.round(avgSpeed)}
            </div>
            <div className="text-sm text-gray-600">Avg Speed</div>
          </div>
        </div>
        <div className="card">
          <div className="card-content text-center">
            <div className="text-2xl font-bold text-orange-600">
              {Math.round(estimatedTime)}
            </div>
            <div className="text-sm text-gray-600">Est. Time (s)</div>
          </div>
        </div>
      </div>

      {/* Route Configuration */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold">Configuration</h3>
        </div>
        <div className="card-content">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div className="p-4 bg-gray-50 rounded-lg">
              <div className="text-sm text-gray-600">Repeat Count</div>
              <div className="text-lg font-semibold">{route.repeatCount}x</div>
            </div>
            <div className="p-4 bg-gray-50 rounded-lg">
              <div className="text-sm text-gray-600">Total Distance (with repeats)</div>
              <div className="text-lg font-semibold">
                {(totalDistance * route.repeatCount).toLocaleString()} mm
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Steps Details */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold">Route Steps</h3>
        </div>
        <div className="card-content">
          <div className="space-y-4">
            {route.steps.map((step, index) => (
              <div
                key={step.id}
                className={`p-4 rounded-lg border-2 ${getOperationColor(step.operation)}`}
              >
                <div className="flex items-center space-x-3">
                  {/* Step Number */}
                  <div className="flex-shrink-0 w-8 h-8 bg-white rounded-full flex items-center justify-center font-bold text-sm">
                    {index + 1}
                  </div>

                  {/* Operation Icon */}
                  <div className="flex-shrink-0">
                    {getOperationIcon(step.operation)}
                  </div>

                  {/* Step Details */}
                  <div className="flex-1">
                    <div className="flex items-center justify-between mb-2">
                      <h4 className="font-semibold">{getOperationName(step.operation)}</h4>
                      <div className="text-sm opacity-75">
                        {step.distance}mm @ {step.speed} speed
                      </div>
                    </div>
                    <p className="text-sm opacity-90">{step.description}</p>
                    
                    {/* Additional Parameters */}
                    <div className="mt-2 flex items-center space-x-4 text-xs opacity-75">
                      <span>Magnetic: {step.magnetCorrection}</span>
                      {step.angle && <span>Angle: {step.angle}°</span>}
                      <span>Duration: ~{Math.round(step.distance / step.speed)}s</span>
                    </div>
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>

      {/* Operation Types Summary */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold">Operation Types Used</h3>
        </div>
        <div className="card-content">
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
            {Object.values(OperationType)
              .filter(op => typeof op === 'number')
              .map(op => {
                const count = route.steps.filter(step => step.operation === op).length
                if (count === 0) return null
                
                return (
                  <div key={op} className="p-3 bg-gray-50 rounded-lg text-center">
                    <div className="text-lg font-bold">{count}</div>
                    <div className="text-sm text-gray-600">
                      {getOperationName(op as OperationType)}
                    </div>
                  </div>
                )
              })
              .filter(Boolean)}
          </div>
        </div>
      </div>
    </div>
  )
}