import React, { useState } from 'react'
import { Save, RotateCcw, Settings as SettingsIcon, Info } from 'lucide-react'
import { DEFAULT_SETTINGS } from '@melkens/shared'
import { useHealthCheck } from '@/hooks/useApi'
import toast from 'react-hot-toast'

interface SimulatorConfig {
  updateRate: number
  correctionAngleThreshold: number
  deg90Offset: number
  deg45Offset: number
  imuJudgementFactor: number
  encoderJudgementFactor: number
  encoderStepMaxMultiplier: number
  defaultSpeed: number
  defaultSpeedLift: number
  defaultSpeedBelt: number
}

export const Settings: React.FC = () => {
  const { data: healthData } = useHealthCheck()
  
  const [settings, setSettings] = useState<SimulatorConfig>({
    updateRate: 100,
    correctionAngleThreshold: DEFAULT_SETTINGS.CORRECTION_ANGLE_THRESHOLD,
    deg90Offset: DEFAULT_SETTINGS.DEG_90_OFFSET,
    deg45Offset: DEFAULT_SETTINGS.DEG_45_OFFSET,
    imuJudgementFactor: DEFAULT_SETTINGS.IMU_JUDGEMENT_FACTOR,
    encoderJudgementFactor: DEFAULT_SETTINGS.ENCODER_JUDGEMENT_FACTOR,
    encoderStepMaxMultiplier: DEFAULT_SETTINGS.ENCODER_STEP_MAX_MULTIPLIER,
    defaultSpeed: DEFAULT_SETTINGS.DEFAULT_SPEED,
    defaultSpeedLift: DEFAULT_SETTINGS.DEFAULT_SPEED_LIFT,
    defaultSpeedBelt: DEFAULT_SETTINGS.DEFAULT_SPEED_BELT
  })

  const [isDirty, setIsDirty] = useState(false)

  const handleSettingChange = (key: keyof SimulatorConfig, value: number) => {
    setSettings(prev => ({ ...prev, [key]: value }))
    setIsDirty(true)
  }

  const handleSave = () => {
    // In a real implementation, this would send settings to the backend
    console.log('Saving settings:', settings)
    toast.success('Settings saved successfully')
    setIsDirty(false)
  }

  const handleReset = () => {
    if (window.confirm('Are you sure you want to reset all settings to default values?')) {
      setSettings({
        updateRate: 100,
        correctionAngleThreshold: DEFAULT_SETTINGS.CORRECTION_ANGLE_THRESHOLD,
        deg90Offset: DEFAULT_SETTINGS.DEG_90_OFFSET,
        deg45Offset: DEFAULT_SETTINGS.DEG_45_OFFSET,
        imuJudgementFactor: DEFAULT_SETTINGS.IMU_JUDGEMENT_FACTOR,
        encoderJudgementFactor: DEFAULT_SETTINGS.ENCODER_JUDGEMENT_FACTOR,
        encoderStepMaxMultiplier: DEFAULT_SETTINGS.ENCODER_STEP_MAX_MULTIPLIER,
        defaultSpeed: DEFAULT_SETTINGS.DEFAULT_SPEED,
        defaultSpeedLift: DEFAULT_SETTINGS.DEFAULT_SPEED_LIFT,
        defaultSpeedBelt: DEFAULT_SETTINGS.DEFAULT_SPEED_BELT
      })
      setIsDirty(true)
      toast.success('Settings reset to defaults')
    }
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-bold text-gray-900">Settings</h1>
          <p className="text-gray-600">Configure simulator and robot parameters</p>
        </div>
        <div className="flex items-center space-x-3">
          <button
            onClick={handleReset}
            className="btn btn-outline flex items-center space-x-2"
          >
            <RotateCcw size={16} />
            <span>Reset to Defaults</span>
          </button>
          <button
            onClick={handleSave}
            disabled={!isDirty}
            className="btn btn-primary flex items-center space-x-2"
          >
            <Save size={16} />
            <span>Save Changes</span>
          </button>
        </div>
      </div>

      {/* System Status */}
      {healthData && (
        <div className="card">
          <div className="card-header">
            <h3 className="text-lg font-semibold flex items-center space-x-2">
              <Info size={20} />
              <span>System Status</span>
            </h3>
          </div>
          <div className="card-content">
            <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
              <div className="p-4 bg-gray-50 rounded-lg">
                <div className="text-sm text-gray-600">Environment</div>
                <div className="font-semibold capitalize">{healthData.environment}</div>
              </div>
              <div className="p-4 bg-gray-50 rounded-lg">
                <div className="text-sm text-gray-600">Version</div>
                <div className="font-semibold">{healthData.version}</div>
              </div>
              <div className="p-4 bg-gray-50 rounded-lg">
                <div className="text-sm text-gray-600">WebSocket Connections</div>
                <div className="font-semibold">{healthData.websocketConnections}</div>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Simulator Settings */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold flex items-center space-x-2">
            <SettingsIcon size={20} />
            <span>Simulator Configuration</span>
          </h3>
        </div>
        <div className="card-content">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div>
              <label className="form-label">Update Rate (ms)</label>
              <input
                type="number"
                value={settings.updateRate}
                onChange={(e) => handleSettingChange('updateRate', Number(e.target.value))}
                min="50"
                max="1000"
                step="50"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">How often the simulation updates (50-1000ms)</p>
            </div>

            <div>
              <label className="form-label">Correction Angle Threshold (degrees)</label>
              <input
                type="number"
                value={settings.correctionAngleThreshold}
                onChange={(e) => handleSettingChange('correctionAngleThreshold', Number(e.target.value))}
                min="0.1"
                max="5"
                step="0.1"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Minimum angle for correction activation</p>
            </div>
          </div>
        </div>
      </div>

      {/* Navigation Parameters */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold">Navigation Parameters</h3>
        </div>
        <div className="card-content">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            <div>
              <label className="form-label">90° Turn Offset (degrees)</label>
              <input
                type="number"
                value={settings.deg90Offset}
                onChange={(e) => handleSettingChange('deg90Offset', Number(e.target.value))}
                min="-10"
                max="10"
                step="0.1"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Compensation for 90-degree turns</p>
            </div>

            <div>
              <label className="form-label">45° Turn Offset (degrees)</label>
              <input
                type="number"
                value={settings.deg45Offset}
                onChange={(e) => handleSettingChange('deg45Offset', Number(e.target.value))}
                min="-10"
                max="10"
                step="0.1"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Compensation for 45-degree turns</p>
            </div>

            <div>
              <label className="form-label">IMU Judgement Factor</label>
              <input
                type="number"
                value={settings.imuJudgementFactor}
                onChange={(e) => handleSettingChange('imuJudgementFactor', Number(e.target.value))}
                min="0"
                max="1"
                step="0.1"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Weight of IMU in decision making (0-1)</p>
            </div>

            <div>
              <label className="form-label">Encoder Judgement Factor</label>
              <input
                type="number"
                value={settings.encoderJudgementFactor}
                onChange={(e) => handleSettingChange('encoderJudgementFactor', Number(e.target.value))}
                min="0"
                max="1"
                step="0.1"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Weight of encoders in decision making (0-1)</p>
            </div>

            <div>
              <label className="form-label">Encoder Step Max Multiplier</label>
              <input
                type="number"
                value={settings.encoderStepMaxMultiplier}
                onChange={(e) => handleSettingChange('encoderStepMaxMultiplier', Number(e.target.value))}
                min="1"
                max="3"
                step="0.1"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Maximum encoder value before step termination</p>
            </div>
          </div>
        </div>
      </div>

      {/* Motor Settings */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold">Motor Configuration</h3>
        </div>
        <div className="card-content">
          <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
            <div>
              <label className="form-label">Default Speed (rpm)</label>
              <input
                type="number"
                value={settings.defaultSpeed}
                onChange={(e) => handleSettingChange('defaultSpeed', Number(e.target.value))}
                min="50"
                max="1000"
                step="10"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Default drive motor speed</p>
            </div>

            <div>
              <label className="form-label">Lift Motor Speed (rpm)</label>
              <input
                type="number"
                value={settings.defaultSpeedLift}
                onChange={(e) => handleSettingChange('defaultSpeedLift', Number(e.target.value))}
                min="50"
                max="1000"
                step="10"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Default lift mechanism speed</p>
            </div>

            <div>
              <label className="form-label">Belt Motor Speed (rpm)</label>
              <input
                type="number"
                value={settings.defaultSpeedBelt}
                onChange={(e) => handleSettingChange('defaultSpeedBelt', Number(e.target.value))}
                min="50"
                max="1000"
                step="10"
                className="form-input"
              />
              <p className="text-xs text-gray-500 mt-1">Default conveyor belt speed</p>
            </div>
          </div>
        </div>
      </div>

      {/* Info Panel */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold">About</h3>
        </div>
        <div className="card-content">
          <div className="prose text-sm text-gray-600">
            <p>
              This simulator is based on the MELKENS PMB (Power Management Board) configuration 
              and WB (Wasserbauer) navigation algorithm. Settings mirror the hardware parameters 
              found in the embedded C code.
            </p>
            <p className="mt-3">
              <strong>Important:</strong> Changes to these settings affect the simulation behavior. 
              Incorrect values may cause navigation issues or unrealistic robot behavior.
            </p>
          </div>
        </div>
      </div>

      {/* Save reminder */}
      {isDirty && (
        <div className="fixed bottom-4 right-4 p-4 bg-yellow-50 border border-yellow-200 rounded-lg shadow-lg">
          <div className="flex items-center space-x-2 text-yellow-800">
            <Info size={16} />
            <span className="text-sm font-medium">You have unsaved changes</span>
          </div>
        </div>
      )}
    </div>
  )
}