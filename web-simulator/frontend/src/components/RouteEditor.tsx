import React, { useState } from 'react'
import { Save, X, Plus, Trash2, ArrowUp, ArrowDown } from 'lucide-react'
import { Route, OperationType, CreateRouteRequest } from '@melkens/shared'
import { useRouteManagement } from '@/hooks/useApi'

interface RouteEditorProps {
  route?: Route | null
  isCreating: boolean
  onCancel: () => void
  onSave: () => void
}

interface StepForm {
  operation: OperationType
  distance: number
  speed: number
  magnetCorrection: number
  angle?: number
  description: string
}

export const RouteEditor: React.FC<RouteEditorProps> = ({ 
  route, 
  isCreating, 
  onCancel, 
  onSave 
}) => {
  const { createRoute, updateRoute } = useRouteManagement()
  
  const [formData, setFormData] = useState({
    name: route?.name || '',
    repeatCount: route?.repeatCount || 1
  })

  const [steps, setSteps] = useState<StepForm[]>(
    route?.steps.map(step => ({
      operation: step.operation,
      distance: step.distance,
      speed: step.speed,
      magnetCorrection: step.magnetCorrection,
      angle: step.angle,
      description: step.description
    })) || []
  )

  const [errors, setErrors] = useState<Record<string, string>>({})

  const operationOptions = [
    { value: OperationType.NORM, label: 'Normal Movement' },
    { value: OperationType.TU_L, label: 'Turn Left' },
    { value: OperationType.TU_R, label: 'Turn Right' },
    { value: OperationType.L_90, label: '90° Left Turn' },
    { value: OperationType.R_90, label: '90° Right Turn' },
    { value: OperationType.DIFF, label: 'Differential' },
    { value: OperationType.NORM_NOMAGNET, label: 'No Magnet' }
  ]

  const validateForm = (): boolean => {
    const newErrors: Record<string, string> = {}

    if (!formData.name.trim()) {
      newErrors.name = 'Route name is required'
    }

    if (formData.repeatCount < 1 || formData.repeatCount > 100) {
      newErrors.repeatCount = 'Repeat count must be between 1 and 100'
    }

    if (steps.length === 0) {
      newErrors.steps = 'At least one step is required'
    }

    steps.forEach((step, index) => {
      if (step.distance < 0) {
        newErrors[`step_${index}_distance`] = 'Distance cannot be negative'
      }
      if (step.speed < 1 || step.speed > 1000) {
        newErrors[`step_${index}_speed`] = 'Speed must be between 1 and 1000'
      }
      if (!step.description.trim()) {
        newErrors[`step_${index}_description`] = 'Description is required'
      }
    })

    setErrors(newErrors)
    return Object.keys(newErrors).length === 0
  }

  const handleSave = async () => {
    if (!validateForm()) return

    const routeData: CreateRouteRequest = {
      name: formData.name,
      repeatCount: formData.repeatCount,
      steps: steps.map(step => ({
        operation: step.operation,
        distance: step.distance,
        speed: step.speed,
        magnetCorrection: step.magnetCorrection,
        angle: step.angle,
        description: step.description
      }))
    }

    try {
      if (isCreating) {
        await createRoute.mutateAsync(routeData)
      } else if (route) {
        await updateRoute.mutateAsync({
          id: route.id,
          ...routeData
        })
      }
      onSave()
    } catch (error) {
      console.error('Failed to save route:', error)
    }
  }

  const addStep = () => {
    setSteps([...steps, {
      operation: OperationType.NORM,
      distance: 1000,
      speed: 500,
      magnetCorrection: 0,
      description: `Step ${steps.length + 1}`
    }])
  }

  const removeStep = (index: number) => {
    setSteps(steps.filter((_, i) => i !== index))
  }

  const moveStep = (index: number, direction: 'up' | 'down') => {
    const newSteps = [...steps]
    const targetIndex = direction === 'up' ? index - 1 : index + 1
    
    if (targetIndex >= 0 && targetIndex < steps.length) {
      [newSteps[index], newSteps[targetIndex]] = [newSteps[targetIndex], newSteps[index]]
      setSteps(newSteps)
    }
  }

  const updateStep = (index: number, field: keyof StepForm, value: any) => {
    const newSteps = [...steps]
    newSteps[index] = { ...newSteps[index], [field]: value }
    setSteps(newSteps)
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-bold text-gray-900">
            {isCreating ? 'Create New Route' : `Edit Route: ${route?.name}`}
          </h1>
          <p className="text-gray-600">
            {isCreating ? 'Design a new robot route' : 'Modify the existing route'}
          </p>
        </div>
        <div className="flex items-center space-x-3">
          <button
            onClick={onCancel}
            className="btn btn-outline flex items-center space-x-2"
          >
            <X size={16} />
            <span>Cancel</span>
          </button>
          <button
            onClick={handleSave}
            disabled={createRoute.isPending || updateRoute.isPending}
            className="btn btn-primary flex items-center space-x-2"
          >
            <Save size={16} />
            <span>Save Route</span>
          </button>
        </div>
      </div>

      {/* Basic Information */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold">Route Information</h3>
        </div>
        <div className="card-content">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div>
              <label className="form-label">Route Name</label>
              <input
                type="text"
                value={formData.name}
                onChange={(e) => setFormData({ ...formData, name: e.target.value })}
                placeholder="Enter route name..."
                className={`form-input ${errors.name ? 'border-red-500' : ''}`}
              />
              {errors.name && (
                <p className="text-red-500 text-sm mt-1">{errors.name}</p>
              )}
            </div>

            <div>
              <label className="form-label">Repeat Count</label>
              <input
                type="number"
                value={formData.repeatCount}
                onChange={(e) => setFormData({ ...formData, repeatCount: Number(e.target.value) })}
                min="1"
                max="100"
                className={`form-input ${errors.repeatCount ? 'border-red-500' : ''}`}
              />
              {errors.repeatCount && (
                <p className="text-red-500 text-sm mt-1">{errors.repeatCount}</p>
              )}
            </div>
          </div>
        </div>
      </div>

      {/* Steps */}
      <div className="card">
        <div className="card-header">
          <div className="flex items-center justify-between">
            <h3 className="text-lg font-semibold">Route Steps</h3>
            <button
              onClick={addStep}
              className="btn btn-secondary flex items-center space-x-2"
            >
              <Plus size={16} />
              <span>Add Step</span>
            </button>
          </div>
        </div>
        <div className="card-content">
          {steps.length === 0 ? (
            <div className="text-center py-8 text-gray-500">
              <p>No steps defined</p>
              <p className="text-sm">Click "Add Step" to start building your route</p>
            </div>
          ) : (
            <div className="space-y-4">
              {steps.map((step, index) => (
                <div key={index} className="border border-gray-200 rounded-lg p-4">
                  <div className="flex items-center justify-between mb-4">
                    <h4 className="font-medium">Step {index + 1}</h4>
                    <div className="flex items-center space-x-2">
                      <button
                        onClick={() => moveStep(index, 'up')}
                        disabled={index === 0}
                        className="p-1 text-gray-500 hover:text-gray-700 disabled:opacity-50"
                      >
                        <ArrowUp size={16} />
                      </button>
                      <button
                        onClick={() => moveStep(index, 'down')}
                        disabled={index === steps.length - 1}
                        className="p-1 text-gray-500 hover:text-gray-700 disabled:opacity-50"
                      >
                        <ArrowDown size={16} />
                      </button>
                      <button
                        onClick={() => removeStep(index)}
                        className="p-1 text-red-500 hover:text-red-700"
                      >
                        <Trash2 size={16} />
                      </button>
                    </div>
                  </div>

                  <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
                    <div>
                      <label className="form-label">Operation</label>
                      <select
                        value={step.operation}
                        onChange={(e) => updateStep(index, 'operation', Number(e.target.value))}
                        className="form-input"
                      >
                        {operationOptions.map(option => (
                          <option key={option.value} value={option.value}>
                            {option.label}
                          </option>
                        ))}
                      </select>
                    </div>

                    <div>
                      <label className="form-label">Distance (mm)</label>
                      <input
                        type="number"
                        value={step.distance}
                        onChange={(e) => updateStep(index, 'distance', Number(e.target.value))}
                        min="0"
                        className={`form-input ${errors[`step_${index}_distance`] ? 'border-red-500' : ''}`}
                      />
                      {errors[`step_${index}_distance`] && (
                        <p className="text-red-500 text-sm mt-1">{errors[`step_${index}_distance`]}</p>
                      )}
                    </div>

                    <div>
                      <label className="form-label">Speed</label>
                      <input
                        type="number"
                        value={step.speed}
                        onChange={(e) => updateStep(index, 'speed', Number(e.target.value))}
                        min="1"
                        max="1000"
                        className={`form-input ${errors[`step_${index}_speed`] ? 'border-red-500' : ''}`}
                      />
                      {errors[`step_${index}_speed`] && (
                        <p className="text-red-500 text-sm mt-1">{errors[`step_${index}_speed`]}</p>
                      )}
                    </div>

                    <div>
                      <label className="form-label">Magnetic Correction</label>
                      <input
                        type="number"
                        value={step.magnetCorrection}
                        onChange={(e) => updateStep(index, 'magnetCorrection', Number(e.target.value))}
                        step="0.1"
                        className="form-input"
                      />
                    </div>

                    {(step.operation === OperationType.L_90 || step.operation === OperationType.R_90) && (
                      <div>
                        <label className="form-label">Angle (degrees)</label>
                        <input
                          type="number"
                          value={step.angle || 90}
                          onChange={(e) => updateStep(index, 'angle', Number(e.target.value))}
                          min="-180"
                          max="180"
                          className="form-input"
                        />
                      </div>
                    )}

                    <div className="md:col-span-2">
                      <label className="form-label">Description</label>
                      <input
                        type="text"
                        value={step.description}
                        onChange={(e) => updateStep(index, 'description', e.target.value)}
                        placeholder="Describe this step..."
                        className={`form-input ${errors[`step_${index}_description`] ? 'border-red-500' : ''}`}
                      />
                      {errors[`step_${index}_description`] && (
                        <p className="text-red-500 text-sm mt-1">{errors[`step_${index}_description`]}</p>
                      )}
                    </div>
                  </div>
                </div>
              ))}
            </div>
          )}
          {errors.steps && (
            <p className="text-red-500 text-sm mt-2">{errors.steps}</p>
          )}
        </div>
      </div>

      {/* Summary */}
      {steps.length > 0 && (
        <div className="card">
          <div className="card-header">
            <h3 className="text-lg font-semibold">Route Summary</h3>
          </div>
          <div className="card-content">
            <div className="grid grid-cols-2 md:grid-cols-4 gap-4 text-center">
              <div>
                <div className="text-2xl font-bold text-blue-600">{steps.length}</div>
                <div className="text-sm text-gray-600">Total Steps</div>
              </div>
              <div>
                <div className="text-2xl font-bold text-green-600">
                  {steps.reduce((sum, step) => sum + step.distance, 0).toLocaleString()}
                </div>
                <div className="text-sm text-gray-600">Total Distance (mm)</div>
              </div>
              <div>
                <div className="text-2xl font-bold text-purple-600">
                  {Math.round(steps.reduce((sum, step) => sum + step.speed, 0) / steps.length)}
                </div>
                <div className="text-sm text-gray-600">Avg Speed</div>
              </div>
              <div>
                <div className="text-2xl font-bold text-orange-600">
                  {Math.round(steps.reduce((sum, step) => sum + (step.distance / step.speed), 0))}
                </div>
                <div className="text-sm text-gray-600">Est. Time (s)</div>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  )
}