import React, { useState } from 'react'
import { Link, useLocation } from 'react-router-dom'
import { 
  Home, 
  Route as RouteIcon, 
  FileText, 
  Settings as SettingsIcon,
  Menu,
  X,
  Wifi,
  WifiOff,
  Bot
} from 'lucide-react'
import { useWebSocket } from '@/hooks/useWebSocket'
import { ConnectionStatus } from './ConnectionStatus'

interface LayoutProps {
  children: React.ReactNode
}

const navigationItems = [
  { path: '/', label: 'Dashboard', icon: Home },
  { path: '/routes', label: 'Routes', icon: RouteIcon },
  { path: '/logs', label: 'Logs', icon: FileText },
  { path: '/settings', label: 'Settings', icon: SettingsIcon },
]

export const Layout: React.FC<LayoutProps> = ({ children }) => {
  const [sidebarOpen, setSidebarOpen] = useState(false)
  const location = useLocation()
  const { isConnected, robotState } = useWebSocket()

  return (
    <div className="min-h-screen bg-gray-50">
      {/* Mobile menu button */}
      <div className="lg:hidden fixed top-4 left-4 z-50">
        <button
          onClick={() => setSidebarOpen(!sidebarOpen)}
          className="p-2 rounded-md bg-white shadow-md border border-gray-200"
        >
          {sidebarOpen ? <X size={20} /> : <Menu size={20} />}
        </button>
      </div>

      {/* Sidebar */}
      <div className={`
        fixed inset-y-0 left-0 z-40 w-64 bg-white shadow-lg transform transition-transform duration-300 ease-in-out lg:translate-x-0
        ${sidebarOpen ? 'translate-x-0' : '-translate-x-full'}
      `}>
        {/* Logo/Header */}
        <div className="flex items-center justify-center h-16 bg-melkens-600 text-white">
          <Bot className="mr-2" size={24} />
          <h1 className="text-xl font-bold">MELKENS WB</h1>
        </div>

        {/* Connection Status */}
        <ConnectionStatus />

        {/* Navigation */}
        <nav className="mt-8">
          <div className="px-4 space-y-2">
            {navigationItems.map((item) => {
              const Icon = item.icon
              const isActive = location.pathname === item.path
              
              return (
                <Link
                  key={item.path}
                  to={item.path}
                  onClick={() => setSidebarOpen(false)}
                  className={`
                    flex items-center px-4 py-3 text-sm font-medium rounded-lg transition-colors duration-200
                    ${isActive 
                      ? 'bg-melkens-100 text-melkens-700 border-r-2 border-melkens-600' 
                      : 'text-gray-600 hover:bg-gray-100 hover:text-gray-900'
                    }
                  `}
                >
                  <Icon className="mr-3" size={20} />
                  {item.label}
                </Link>
              )
            })}
          </div>
        </nav>

        {/* Robot Status Summary */}
        {robotState && (
          <div className="absolute bottom-4 left-4 right-4 p-4 bg-gray-50 rounded-lg text-xs">
            <div className="flex items-center justify-between mb-2">
              <span className="font-medium">Robot Status</span>
              <div className={`
                w-2 h-2 rounded-full 
                ${robotState.isRunning ? 'bg-robot-active animate-pulse' : 'bg-robot-idle'}
              `} />
            </div>
            <div className="space-y-1 text-gray-600">
              <div>Position: ({Math.round(robotState.position.x)}, {Math.round(robotState.position.y)})</div>
              <div>Angle: {Math.round(robotState.position.angle)}°</div>
              <div>Battery: {robotState.sensors.batteryVoltage.toFixed(1)}V</div>
            </div>
          </div>
        )}
      </div>

      {/* Overlay for mobile */}
      {sidebarOpen && (
        <div 
          className="fixed inset-0 bg-black bg-opacity-50 z-30 lg:hidden"
          onClick={() => setSidebarOpen(false)}
        />
      )}

      {/* Main content */}
      <div className="lg:ml-64">
        {/* Header */}
        <header className="bg-white shadow-sm border-b border-gray-200">
          <div className="px-4 sm:px-6 lg:px-8">
            <div className="flex justify-between items-center h-16">
              <div className="flex items-center lg:hidden">
                {/* Space for mobile menu button */}
              </div>
              
              <div className="flex-1 flex justify-center lg:justify-start">
                <h2 className="text-xl font-semibold text-gray-900 capitalize">
                  {location.pathname === '/' ? 'Dashboard' : location.pathname.slice(1)}
                </h2>
              </div>

              <div className="flex items-center space-x-4">
                {/* Connection indicator */}
                <div className="flex items-center space-x-2 text-sm">
                  {isConnected ? (
                    <>
                      <Wifi className="text-robot-active" size={16} />
                      <span className="text-robot-active">Connected</span>
                    </>
                  ) : (
                    <>
                      <WifiOff className="text-robot-error" size={16} />
                      <span className="text-robot-error">Disconnected</span>
                    </>
                  )}
                </div>

                {/* Current time */}
                <div className="text-sm text-gray-500">
                  {new Date().toLocaleTimeString()}
                </div>
              </div>
            </div>
          </div>
        </header>

        {/* Page content */}
        <main className="p-4 sm:p-6 lg:p-8">
          {children}
        </main>
      </div>
    </div>
  )
}