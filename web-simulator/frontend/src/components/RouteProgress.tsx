import React from 'react'
import { CheckCircle, Circle, Clock, ArrowRight, RotateCw, TurnLeft, TurnRight } from 'lucide-react'
import { RobotState, OperationType } from '@melkens/shared'
import { useRoutes } from '@/hooks/useApi'

interface RouteProgressProps {
  robotState: RobotState
}

export const RouteProgress: React.FC<RouteProgressProps> = ({ robotState }) => {
  const { data: routes = [] } = useRoutes()
  const currentRoute = routes.find(r => r.id === robotState.currentRoute)

  const getOperationIcon = (operation: OperationType) => {
    switch (operation) {
      case OperationType.L_90:
        return <TurnLeft size={16} className="text-blue-600" />
      case OperationType.R_90:
        return <TurnRight size={16} className="text-blue-600" />
      case OperationType.TU_L:
        return <TurnLeft size={16} className="text-purple-600" />
      case OperationType.TU_R:
        return <TurnRight size={16} className="text-purple-600" />
      default:
        return <ArrowRight size={16} className="text-green-600" />
    }
  }

  const getOperationName = (operation: OperationType) => {
    switch (operation) {
      case OperationType.NORM:
        return 'Normal'
      case OperationType.TU_L:
        return 'Turn Left'
      case OperationType.TU_R:
        return 'Turn Right'
      case OperationType.L_90:
        return '90° Left'
      case OperationType.R_90:
        return '90° Right'
      case OperationType.DIFF:
        return 'Differential'
      case OperationType.NORM_NOMAGNET:
        return 'No Magnet'
      default:
        return 'Unknown'
    }
  }

  if (!robotState.isRunning || !currentRoute) {
    return (
      <div className="text-center text-gray-500 py-8">
        <Clock size={24} className="mx-auto mb-2" />
        <p>No route in progress</p>
        <p className="text-sm">Start a route to see progress here</p>
      </div>
    )
  }

  const currentStepIndex = robotState.currentStep ?? 0
  const totalSteps = currentRoute.steps.length
  const progressPercentage = totalSteps > 0 ? ((currentStepIndex + 1) / totalSteps) * 100 : 0

  return (
    <div className="space-y-4">
      {/* Route Header */}
      <div className="flex items-center justify-between">
        <h4 className="font-medium">{currentRoute.name}</h4>
        <div className="text-sm text-gray-500">
          {currentStepIndex + 1} / {totalSteps} steps
        </div>
      </div>

      {/* Progress Bar */}
      <div className="w-full bg-gray-200 rounded-full h-2">
        <div
          className="bg-blue-600 h-2 rounded-full transition-all duration-300"
          style={{ width: `${progressPercentage}%` }}
        />
      </div>

      {/* Current Step Details */}
      {currentRoute.steps[currentStepIndex] && (
        <div className="p-3 bg-blue-50 border border-blue-200 rounded-lg">
          <div className="flex items-center space-x-2 mb-2">
            <RotateCw className="text-blue-600 animate-spin" size={16} />
            <span className="font-medium text-blue-800">Current Step</span>
          </div>
          <div className="text-sm text-blue-700">
            {currentRoute.steps[currentStepIndex].description}
          </div>
          <div className="flex items-center mt-2 space-x-4 text-xs text-blue-600">
            <span>Distance: {currentRoute.steps[currentStepIndex].distance}mm</span>
            <span>Speed: {currentRoute.steps[currentStepIndex].speed}</span>
          </div>
        </div>
      )}

      {/* Steps List */}
      <div className="space-y-2 max-h-60 overflow-y-auto scrollbar-thin">
        {currentRoute.steps.map((step, index) => {
          const isCompleted = index < currentStepIndex
          const isCurrent = index === currentStepIndex
          const isPending = index > currentStepIndex

          return (
            <div
              key={step.id}
              className={`
                flex items-center space-x-3 p-3 rounded-lg border transition-all
                ${isCurrent 
                  ? 'bg-blue-50 border-blue-200' 
                  : isCompleted 
                    ? 'bg-green-50 border-green-200' 
                    : 'bg-gray-50 border-gray-200'
                }
              `}
            >
              {/* Step Number & Status */}
              <div className="flex-shrink-0">
                {isCompleted ? (
                  <CheckCircle className="text-green-600" size={20} />
                ) : isCurrent ? (
                  <RotateCw className="text-blue-600 animate-spin" size={20} />
                ) : (
                  <Circle className="text-gray-400" size={20} />
                )}
              </div>

              {/* Operation Icon */}
              <div className="flex-shrink-0">
                {getOperationIcon(step.operation)}
              </div>

              {/* Step Details */}
              <div className="flex-1 min-w-0">
                <div className="flex items-center justify-between">
                  <span className={`
                    text-sm font-medium
                    ${isCurrent ? 'text-blue-800' : isCompleted ? 'text-green-800' : 'text-gray-600'}
                  `}>
                    Step {index + 1}: {getOperationName(step.operation)}
                  </span>
                  <span className="text-xs text-gray-500">
                    {step.distance}mm
                  </span>
                </div>
                <div className={`
                  text-xs truncate
                  ${isCurrent ? 'text-blue-600' : isCompleted ? 'text-green-600' : 'text-gray-500'}
                `}>
                  {step.description}
                </div>
              </div>
            </div>
          )
        })}
      </div>

      {/* Route Summary */}
      <div className="p-3 bg-gray-50 rounded-lg text-sm">
        <div className="grid grid-cols-2 gap-4">
          <div>
            <span className="text-gray-600">Total Distance:</span>
            <div className="font-mono">
              {currentRoute.steps.reduce((sum, step) => sum + step.distance, 0)}mm
            </div>
          </div>
          <div>
            <span className="text-gray-600">Repeat Count:</span>
            <div className="font-mono">{currentRoute.repeatCount}</div>
          </div>
        </div>
      </div>
    </div>
  )
}