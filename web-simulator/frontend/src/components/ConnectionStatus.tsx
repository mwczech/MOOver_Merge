import React from 'react'
import { Wifi, WifiOff, AlertCircle } from 'lucide-react'
import { useWebSocket } from '@/hooks/useWebSocket'

export const ConnectionStatus: React.FC = () => {
  const { isConnected, connect } = useWebSocket()

  return (
    <div className="p-4 border-b border-gray-200">
      <div className={`
        flex items-center justify-between p-3 rounded-lg
        ${isConnected ? 'bg-green-50 border border-green-200' : 'bg-red-50 border border-red-200'}
      `}>
        <div className="flex items-center space-x-2">
          {isConnected ? (
            <>
              <Wifi className="text-green-600" size={16} />
              <span className="text-sm font-medium text-green-800">Connected</span>
            </>
          ) : (
            <>
              <WifiOff className="text-red-600" size={16} />
              <span className="text-sm font-medium text-red-800">Disconnected</span>
            </>
          )}
        </div>
        
        {!isConnected && (
          <button
            onClick={connect}
            className="text-xs px-2 py-1 bg-red-600 text-white rounded hover:bg-red-700 transition-colors"
          >
            Reconnect
          </button>
        )}
      </div>
      
      {!isConnected && (
        <div className="mt-2 flex items-start space-x-2 p-2 bg-yellow-50 border border-yellow-200 rounded">
          <AlertCircle className="text-yellow-600 flex-shrink-0 mt-0.5" size={14} />
          <p className="text-xs text-yellow-800">
            Real-time updates are disabled. Check if the backend server is running.
          </p>
        </div>
      )}
    </div>
  )
}