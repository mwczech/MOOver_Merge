import React from 'react'
import { AlertCircle, Info, AlertTriangle, Bug } from 'lucide-react'
import { useWebSocket } from '@/hooks/useWebSocket'
import type { LogEntry } from '@melkens/shared'

interface RecentLogsProps {
  limit?: number
}

export const RecentLogs: React.FC<RecentLogsProps> = ({ limit = 10 }) => {
  const { logs } = useWebSocket()

  const getLogIcon = (level: LogEntry['level']) => {
    switch (level) {
      case 'error':
        return <AlertCircle className="text-red-600" size={16} />
      case 'warning':
        return <AlertTriangle className="text-yellow-600" size={16} />
      case 'debug':
        return <Bug className="text-purple-600" size={16} />
      default:
        return <Info className="text-blue-600" size={16} />
    }
  }

  const getLogColor = (level: LogEntry['level']) => {
    switch (level) {
      case 'error':
        return 'text-red-800 bg-red-50 border-red-200'
      case 'warning':
        return 'text-yellow-800 bg-yellow-50 border-yellow-200'
      case 'debug':
        return 'text-purple-800 bg-purple-50 border-purple-200'
      default:
        return 'text-blue-800 bg-blue-50 border-blue-200'
    }
  }

  const formatTime = (timestamp: number) => {
    return new Date(timestamp).toLocaleTimeString()
  }

  const displayLogs = logs.slice(0, limit)

  if (displayLogs.length === 0) {
    return (
      <div className="text-center text-gray-500 py-8">
        <Info size={24} className="mx-auto mb-2" />
        <p>No recent activity</p>
        <p className="text-sm">Robot logs will appear here</p>
      </div>
    )
  }

  return (
    <div className="space-y-2 max-h-80 overflow-y-auto scrollbar-thin">
      {displayLogs.map((log, index) => (
        <div
          key={`${log.timestamp}-${index}`}
          className={`p-3 rounded-lg border ${getLogColor(log.level)}`}
        >
          <div className="flex items-start space-x-2">
            <div className="flex-shrink-0 mt-0.5">
              {getLogIcon(log.level)}
            </div>
            <div className="flex-1 min-w-0">
              <div className="flex items-center justify-between mb-1">
                <span className="text-xs font-medium uppercase tracking-wide">
                  {log.level}
                </span>
                <span className="text-xs font-mono">
                  {formatTime(log.timestamp)}
                </span>
              </div>
              <div className="text-sm font-medium mb-1">
                {log.source}
              </div>
              <div className="text-sm">
                {log.message}
              </div>
              {log.data && (
                <details className="mt-2">
                  <summary className="text-xs cursor-pointer hover:underline">
                    Show details
                  </summary>
                  <pre className="text-xs mt-1 p-2 bg-white bg-opacity-50 rounded overflow-x-auto">
                    {JSON.stringify(log.data, null, 2)}
                  </pre>
                </details>
              )}
            </div>
          </div>
        </div>
      ))}
      
      {logs.length > limit && (
        <div className="text-center py-2">
          <span className="text-xs text-gray-500">
            Showing {limit} of {logs.length} recent logs
          </span>
        </div>
      )}
    </div>
  )
}