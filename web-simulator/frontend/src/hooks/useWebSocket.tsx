import React, { createContext, useContext, useEffect, useState, useCallback, ReactNode } from 'react'
import toast from 'react-hot-toast'
import type { 
  WebSocketMessage, 
  RobotState, 
  LogEntry, 
  Route 
} from '@melkens/shared'

interface WebSocketContextType {
  isConnected: boolean
  robotState: RobotState | null
  logs: LogEntry[]
  routes: Route[]
  sendCommand: (command: any) => void
  connect: () => void
  disconnect: () => void
}

const WebSocketContext = createContext<WebSocketContextType | null>(null)

interface WebSocketProviderProps {
  children: ReactNode
}

export const WebSocketProvider: React.FC<WebSocketProviderProps> = ({ children }) => {
  const [socket, setSocket] = useState<WebSocket | null>(null)
  const [isConnected, setIsConnected] = useState(false)
  const [robotState, setRobotState] = useState<RobotState | null>(null)
  const [logs, setLogs] = useState<LogEntry[]>([])
  const [routes, setRoutes] = useState<Route[]>([])
  const [reconnectAttempts, setReconnectAttempts] = useState(0)

  const maxReconnectAttempts = 5
  const reconnectDelay = 3000

  const connect = useCallback(() => {
    try {
      const ws = new WebSocket('ws://localhost:3002')
      
      ws.onopen = () => {
        console.log('WebSocket connected')
        setIsConnected(true)
        setReconnectAttempts(0)
        toast.success('Connected to robot simulator')
      }

      ws.onmessage = (event) => {
        try {
          const message: WebSocketMessage = JSON.parse(event.data)
          handleMessage(message)
        } catch (error) {
          console.error('Failed to parse WebSocket message:', error)
        }
      }

      ws.onclose = (event) => {
        console.log('WebSocket disconnected:', event.code, event.reason)
        setIsConnected(false)
        setSocket(null)

        // Attempt to reconnect
        if (reconnectAttempts < maxReconnectAttempts) {
          setTimeout(() => {
            setReconnectAttempts(prev => prev + 1)
            connect()
          }, reconnectDelay)
        } else {
          toast.error('Lost connection to robot simulator')
        }
      }

      ws.onerror = (error) => {
        console.error('WebSocket error:', error)
        toast.error('WebSocket connection error')
      }

      setSocket(ws)
    } catch (error) {
      console.error('Failed to create WebSocket connection:', error)
      toast.error('Failed to connect to robot simulator')
    }
  }, [reconnectAttempts])

  const disconnect = useCallback(() => {
    if (socket) {
      socket.close()
      setSocket(null)
      setIsConnected(false)
    }
  }, [socket])

  const sendCommand = useCallback((command: any) => {
    if (socket && isConnected) {
      socket.send(JSON.stringify(command))
    } else {
      toast.error('Not connected to robot simulator')
    }
  }, [socket, isConnected])

  const handleMessage = useCallback((message: WebSocketMessage) => {
    switch (message.type) {
      case 'robot_state_update':
        setRobotState(message.data)
        break

      case 'log_message':
        setLogs(prev => {
          const newLogs = [message.data, ...prev]
          // Keep only last 1000 logs in memory
          return newLogs.slice(0, 1000)
        })
        
        // Show important logs as toasts
        if (message.data.level === 'error') {
          toast.error(message.data.message)
        } else if (message.data.level === 'warning') {
          toast(message.data.message, { icon: '⚠️' })
        }
        break

      case 'routes_update':
        setRoutes(message.data)
        break

      case 'route_started':
        toast.success(`Route ${message.data.routeId} started`)
        break

      case 'route_completed':
        if (message.data.success) {
          toast.success(`Route ${message.data.routeId} completed successfully`)
        } else {
          toast.error(`Route ${message.data.routeId} failed`)
        }
        break

      case 'emergency_stop':
        toast.error('🚨 Emergency stop activated!')
        break

      case 'command_response':
        // Handle command responses if needed
        console.log('Command response:', message.data)
        break

      case 'error':
        toast.error(message.data.error)
        break

      default:
        console.log('Unknown WebSocket message type:', message.type)
    }
  }, [])

  // Connect on mount
  useEffect(() => {
    connect()
    
    return () => {
      disconnect()
    }
  }, [])

  // Heartbeat to maintain connection
  useEffect(() => {
    if (!isConnected || !socket) return

    const interval = setInterval(() => {
      if (socket.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify({ type: 'ping', timestamp: Date.now() }))
      }
    }, 30000) // Send ping every 30 seconds

    return () => clearInterval(interval)
  }, [isConnected, socket])

  const value: WebSocketContextType = {
    isConnected,
    robotState,
    logs,
    routes,
    sendCommand,
    connect,
    disconnect
  }

  return (
    <WebSocketContext.Provider value={value}>
      {children}
    </WebSocketContext.Provider>
  )
}

export const useWebSocket = () => {
  const context = useContext(WebSocketContext)
  if (!context) {
    throw new Error('useWebSocket must be used within a WebSocketProvider')
  }
  return context
}