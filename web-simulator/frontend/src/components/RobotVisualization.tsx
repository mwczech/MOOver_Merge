import React, { useRef, useEffect, useState } from 'react'
import { RobotState, SIMULATION_SETTINGS } from '@melkens/shared'
import { RotateCw, ZoomIn, ZoomOut, Maximize2 } from 'lucide-react'

interface RobotVisualizationProps {
  robotState: RobotState
}

interface Point {
  x: number
  y: number
  timestamp: number
}

export const RobotVisualization: React.FC<RobotVisualizationProps> = ({ robotState }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const [scale, setScale] = useState(0.1) // 10cm per pixel initially
  const [offset, setOffset] = useState({ x: 0, y: 0 })
  const [isDragging, setIsDragging] = useState(false)
  const [lastMousePos, setLastMousePos] = useState({ x: 0, y: 0 })
  const [robotPath, setRobotPath] = useState<Point[]>([])

  // Robot dimensions (in mm)
  const robotWidth = SIMULATION_SETTINGS.ROBOT_SIZE.WIDTH
  const robotLength = SIMULATION_SETTINGS.ROBOT_SIZE.LENGTH
  const mapWidth = SIMULATION_SETTINGS.MAP_SIZE.WIDTH
  const mapHeight = SIMULATION_SETTINGS.MAP_SIZE.HEIGHT

  // Add current position to path
  useEffect(() => {
    const newPoint: Point = {
      x: robotState.position.x,
      y: robotState.position.y,
      timestamp: robotState.position.timestamp
    }

    setRobotPath(prev => {
      const updated = [...prev, newPoint]
      // Keep only last 500 points to prevent memory issues
      return updated.slice(-500)
    })
  }, [robotState.position.x, robotState.position.y])

  // Drawing function
  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return

    const ctx = canvas.getContext('2d')
    if (!ctx) return

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height)

    // Save context
    ctx.save()

    // Apply transformations
    ctx.translate(canvas.width / 2 + offset.x, canvas.height / 2 + offset.y)
    ctx.scale(scale, scale)

    // Draw coordinate system
    drawCoordinateSystem(ctx)

    // Draw map boundaries
    drawMapBoundaries(ctx)

    // Draw robot path
    drawRobotPath(ctx)

    // Draw magnetic line (simulated)
    drawMagneticLine(ctx)

    // Draw robot
    drawRobot(ctx, robotState)

    // Draw position info
    drawPositionInfo(ctx, robotState)

    // Restore context
    ctx.restore()
  }, [robotState, scale, offset, robotPath])

  const drawCoordinateSystem = (ctx: CanvasRenderingContext2D) => {
    ctx.strokeStyle = '#e5e7eb'
    ctx.lineWidth = 1
    
    // Grid lines every 500mm
    const gridSize = 500
    const gridRange = Math.max(mapWidth, mapHeight)
    
    // Vertical lines
    for (let x = -gridRange; x <= gridRange; x += gridSize) {
      ctx.beginPath()
      ctx.moveTo(x, -gridRange)
      ctx.lineTo(x, gridRange)
      ctx.stroke()
    }
    
    // Horizontal lines
    for (let y = -gridRange; y <= gridRange; y += gridSize) {
      ctx.beginPath()
      ctx.moveTo(-gridRange, y)
      ctx.lineTo(gridRange, y)
      ctx.stroke()
    }

    // Main axes
    ctx.strokeStyle = '#9ca3af'
    ctx.lineWidth = 2
    
    // X-axis
    ctx.beginPath()
    ctx.moveTo(-gridRange, 0)
    ctx.lineTo(gridRange, 0)
    ctx.stroke()
    
    // Y-axis
    ctx.beginPath()
    ctx.moveTo(0, -gridRange)
    ctx.lineTo(0, gridRange)
    ctx.stroke()
  }

  const drawMapBoundaries = (ctx: CanvasRenderingContext2D) => {
    ctx.strokeStyle = '#ef4444'
    ctx.lineWidth = 3
    ctx.setLineDash([10, 5])
    
    ctx.strokeRect(
      -mapWidth / 2,
      -mapHeight / 2,
      mapWidth,
      mapHeight
    )
    
    ctx.setLineDash([])
  }

  const drawMagneticLine = (ctx: CanvasRenderingContext2D) => {
    // Draw a simulated magnetic line path
    ctx.strokeStyle = '#8b5cf6'
    ctx.lineWidth = 8
    ctx.setLineDash([20, 10])
    
    // Simple path for demonstration
    ctx.beginPath()
    ctx.moveTo(-mapWidth / 2 + 500, 0)
    ctx.lineTo(mapWidth / 2 - 1000, 0)
    ctx.lineTo(mapWidth / 2 - 1000, mapHeight / 3)
    ctx.lineTo(-mapWidth / 2 + 500, mapHeight / 3)
    ctx.stroke()
    
    ctx.setLineDash([])
  }

  const drawRobotPath = (ctx: CanvasRenderingContext2D) => {
    if (robotPath.length < 2) return

    ctx.strokeStyle = '#3b82f6'
    ctx.lineWidth = 3
    ctx.setLineDash([5, 5])
    
    ctx.beginPath()
    ctx.moveTo(robotPath[0].x, robotPath[0].y)
    
    for (let i = 1; i < robotPath.length; i++) {
      ctx.lineTo(robotPath[i].x, robotPath[i].y)
    }
    
    ctx.stroke()
    ctx.setLineDash([])

    // Draw path points
    ctx.fillStyle = '#3b82f6'
    for (const point of robotPath) {
      ctx.beginPath()
      ctx.arc(point.x, point.y, 5, 0, 2 * Math.PI)
      ctx.fill()
    }
  }

  const drawRobot = (ctx: CanvasRenderingContext2D, state: RobotState) => {
    ctx.save()
    
    // Move to robot position
    ctx.translate(state.position.x, state.position.y)
    ctx.rotate((state.position.angle * Math.PI) / 180)

    // Robot body
    ctx.fillStyle = state.isRunning ? '#10b981' : '#6b7280'
    ctx.strokeStyle = '#1f2937'
    ctx.lineWidth = 3
    
    ctx.fillRect(-robotWidth / 2, -robotLength / 2, robotWidth, robotLength)
    ctx.strokeRect(-robotWidth / 2, -robotLength / 2, robotWidth, robotLength)

    // Direction indicator (front of robot)
    ctx.fillStyle = '#f59e0b'
    ctx.fillRect(-robotWidth / 4, -robotLength / 2 - 50, robotWidth / 2, 40)

    // Motor indicators
    const motorSpeed = Math.abs(state.motors.leftSpeed + state.motors.rightSpeed) / 2
    const motorColor = motorSpeed > 0 ? '#10b981' : '#6b7280'
    
    ctx.fillStyle = motorColor
    // Left motor
    ctx.fillRect(-robotWidth / 2 - 20, -robotLength / 4, 15, robotLength / 2)
    // Right motor
    ctx.fillRect(robotWidth / 2 + 5, -robotLength / 4, 15, robotLength / 2)

    // Sensor indicators
    ctx.fillStyle = state.sensors.isConnected ? '#10b981' : '#ef4444'
    ctx.beginPath()
    ctx.arc(0, robotLength / 2 - 30, 10, 0, 2 * Math.PI)
    ctx.fill()

    ctx.restore()
  }

  const drawPositionInfo = (ctx: CanvasRenderingContext2D, state: RobotState) => {
    // Draw position info near robot
    ctx.save()
    
    ctx.fillStyle = '#1f2937'
    ctx.font = '16px monospace'
    ctx.textAlign = 'left'
    
    const infoX = state.position.x + robotWidth / 2 + 50
    const infoY = state.position.y - robotLength / 2
    
    ctx.fillText(`(${Math.round(state.position.x)}, ${Math.round(state.position.y)})`, infoX, infoY)
    ctx.fillText(`${Math.round(state.position.angle)}°`, infoX, infoY + 25)
    ctx.fillText(`${Math.round(state.motors.leftSpeed + state.motors.rightSpeed) / 2} mm/s`, infoX, infoY + 50)
    
    ctx.restore()
  }

  // Mouse event handlers
  const handleMouseDown = (e: React.MouseEvent) => {
    setIsDragging(true)
    setLastMousePos({ x: e.clientX, y: e.clientY })
  }

  const handleMouseMove = (e: React.MouseEvent) => {
    if (!isDragging) return

    const deltaX = e.clientX - lastMousePos.x
    const deltaY = e.clientY - lastMousePos.y

    setOffset(prev => ({
      x: prev.x + deltaX,
      y: prev.y + deltaY
    }))

    setLastMousePos({ x: e.clientX, y: e.clientY })
  }

  const handleMouseUp = () => {
    setIsDragging(false)
  }

  const zoomIn = () => setScale(prev => Math.min(prev * 1.5, 1))
  const zoomOut = () => setScale(prev => Math.max(prev / 1.5, 0.01))
  const resetView = () => {
    setScale(0.1)
    setOffset({ x: 0, y: 0 })
  }

  const centerOnRobot = () => {
    setOffset({
      x: -robotState.position.x * scale,
      y: -robotState.position.y * scale
    })
  }

  return (
    <div className="relative">
      {/* Canvas */}
      <canvas
        ref={canvasRef}
        width={800}
        height={600}
        className="border border-gray-300 rounded-lg cursor-move bg-white"
        onMouseDown={handleMouseDown}
        onMouseMove={handleMouseMove}
        onMouseUp={handleMouseUp}
        onMouseLeave={handleMouseUp}
      />

      {/* Controls */}
      <div className="absolute top-4 right-4 flex flex-col space-y-2">
        <button
          onClick={zoomIn}
          className="p-2 bg-white border border-gray-300 rounded shadow hover:bg-gray-50"
          title="Zoom In"
        >
          <ZoomIn size={16} />
        </button>
        <button
          onClick={zoomOut}
          className="p-2 bg-white border border-gray-300 rounded shadow hover:bg-gray-50"
          title="Zoom Out"
        >
          <ZoomOut size={16} />
        </button>
        <button
          onClick={resetView}
          className="p-2 bg-white border border-gray-300 rounded shadow hover:bg-gray-50"
          title="Reset View"
        >
          <Maximize2 size={16} />
        </button>
        <button
          onClick={centerOnRobot}
          className="p-2 bg-white border border-gray-300 rounded shadow hover:bg-gray-50"
          title="Center on Robot"
        >
          <RotateCw size={16} />
        </button>
      </div>

      {/* Info panel */}
      <div className="absolute bottom-4 left-4 bg-white border border-gray-300 rounded p-3 text-sm font-mono">
        <div>Scale: {(scale * 1000).toFixed(1)}mm/px</div>
        <div>Position: ({Math.round(robotState.position.x)}, {Math.round(robotState.position.y)})</div>
        <div>Angle: {Math.round(robotState.position.angle)}°</div>
        <div>Path points: {robotPath.length}</div>
      </div>
    </div>
  )
}