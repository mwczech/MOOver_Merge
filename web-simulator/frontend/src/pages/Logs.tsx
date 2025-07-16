import React, { useState } from 'react'
import { Download, Trash2, Filter, Search, RefreshCw } from 'lucide-react'
import { useLogs, useLogManagement } from '@/hooks/useApi'
import { useWebSocket } from '@/hooks/useWebSocket'
import { RecentLogs } from '@/components/RecentLogs'

export const Logs: React.FC = () => {
  const [filters, setFilters] = useState({
    level: '',
    source: '',
    search: '',
    limit: 100,
    offset: 0
  })

  const { logs: wsLogs } = useWebSocket()
  const { data: apiLogs, isLoading, refetch } = useLogs(filters)
  const { clearLogs, exportLogs } = useLogManagement()

  // Use WebSocket logs if available, otherwise API logs
  const logs = wsLogs.length > 0 ? {
    logs: wsLogs.slice(filters.offset, filters.offset + filters.limit),
    total: wsLogs.length
  } : apiLogs

  const handleFilterChange = (key: string, value: string | number) => {
    setFilters(prev => ({ ...prev, [key]: value, offset: 0 }))
  }

  const handleExport = (format: 'json' | 'csv') => {
    exportLogs(format)
  }

  const handleClearLogs = () => {
    if (window.confirm('Are you sure you want to clear all logs? This action cannot be undone.')) {
      clearLogs.mutate()
    }
  }

  const filteredLogs = logs?.logs?.filter(log => {
    if (filters.level && log.level !== filters.level) return false
    if (filters.source && !log.source.toLowerCase().includes(filters.source.toLowerCase())) return false
    if (filters.search && !log.message.toLowerCase().includes(filters.search.toLowerCase())) return false
    return true
  }) || []

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-bold text-gray-900">System Logs</h1>
          <p className="text-gray-600">Monitor robot activities and system events</p>
        </div>
        <div className="flex items-center space-x-3">
          <button
            onClick={() => refetch()}
            className="btn btn-outline flex items-center space-x-2"
            disabled={isLoading}
          >
            <RefreshCw className={isLoading ? 'animate-spin' : ''} size={16} />
            <span>Refresh</span>
          </button>
          <button
            onClick={() => handleExport('json')}
            className="btn btn-secondary flex items-center space-x-2"
          >
            <Download size={16} />
            <span>Export JSON</span>
          </button>
          <button
            onClick={() => handleExport('csv')}
            className="btn btn-secondary flex items-center space-x-2"
          >
            <Download size={16} />
            <span>Export CSV</span>
          </button>
          <button
            onClick={handleClearLogs}
            className="btn btn-danger flex items-center space-x-2"
            disabled={clearLogs.isPending}
          >
            <Trash2 size={16} />
            <span>Clear All</span>
          </button>
        </div>
      </div>

      {/* Stats */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-6">
        {['info', 'warning', 'error', 'debug'].map((level) => {
          const count = filteredLogs.filter(log => log.level === level).length
          const colorMap = {
            info: 'text-blue-600 bg-blue-50',
            warning: 'text-yellow-600 bg-yellow-50',
            error: 'text-red-600 bg-red-50',
            debug: 'text-purple-600 bg-purple-50'
          }
          
          return (
            <div key={level} className="card">
              <div className="card-content">
                <div className="flex items-center justify-between">
                  <div>
                    <p className="text-sm font-medium text-gray-600 capitalize">{level}</p>
                    <p className="text-2xl font-bold text-gray-900">{count}</p>
                  </div>
                  <div className={`p-3 rounded-full ${colorMap[level as keyof typeof colorMap]}`}>
                    <svg className="w-6 h-6" fill="currentColor" viewBox="0 0 20 20">
                      <path fillRule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-7-4a1 1 0 11-2 0 1 1 0 012 0zM9 9a1 1 0 000 2v3a1 1 0 001 1h1a1 1 0 100-2v-3a1 1 0 00-1-1H9z" clipRule="evenodd" />
                    </svg>
                  </div>
                </div>
              </div>
            </div>
          )
        })}
      </div>

      {/* Filters */}
      <div className="card">
        <div className="card-header">
          <h3 className="text-lg font-semibold flex items-center space-x-2">
            <Filter size={20} />
            <span>Filters</span>
          </h3>
        </div>
        <div className="card-content">
          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <div>
              <label className="form-label">Level</label>
              <select
                value={filters.level}
                onChange={(e) => handleFilterChange('level', e.target.value)}
                className="form-input"
              >
                <option value="">All levels</option>
                <option value="info">Info</option>
                <option value="warning">Warning</option>
                <option value="error">Error</option>
                <option value="debug">Debug</option>
              </select>
            </div>

            <div>
              <label className="form-label">Source</label>
              <input
                type="text"
                value={filters.source}
                onChange={(e) => handleFilterChange('source', e.target.value)}
                placeholder="Filter by source..."
                className="form-input"
              />
            </div>

            <div>
              <label className="form-label">Search</label>
              <div className="relative">
                <input
                  type="text"
                  value={filters.search}
                  onChange={(e) => handleFilterChange('search', e.target.value)}
                  placeholder="Search in messages..."
                  className="form-input pl-10"
                />
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400" size={16} />
              </div>
            </div>

            <div>
              <label className="form-label">Limit</label>
              <select
                value={filters.limit}
                onChange={(e) => handleFilterChange('limit', Number(e.target.value))}
                className="form-input"
              >
                <option value={50}>50 logs</option>
                <option value={100}>100 logs</option>
                <option value={250}>250 logs</option>
                <option value={500}>500 logs</option>
              </select>
            </div>
          </div>
        </div>
      </div>

      {/* Logs Display */}
      <div className="card">
        <div className="card-header">
          <div className="flex items-center justify-between">
            <h3 className="text-lg font-semibold">
              Log Entries
              {logs?.total && (
                <span className="text-sm text-gray-500 ml-2">
                  ({filteredLogs.length} of {logs.total} total)
                </span>
              )}
            </h3>
            {logs?.total && logs.total > filters.limit && (
              <div className="flex items-center space-x-2">
                <button
                  onClick={() => handleFilterChange('offset', Math.max(0, filters.offset - filters.limit))}
                  disabled={filters.offset === 0}
                  className="btn btn-outline btn-sm"
                >
                  Previous
                </button>
                <span className="text-sm text-gray-500">
                  Page {Math.floor(filters.offset / filters.limit) + 1} of {Math.ceil(logs.total / filters.limit)}
                </span>
                <button
                  onClick={() => handleFilterChange('offset', filters.offset + filters.limit)}
                  disabled={filters.offset + filters.limit >= logs.total}
                  className="btn btn-outline btn-sm"
                >
                  Next
                </button>
              </div>
            )}
          </div>
        </div>
        <div className="card-content">
          {isLoading ? (
            <div className="flex items-center justify-center py-8">
              <div className="loading-spinner" />
              <span className="ml-2">Loading logs...</span>
            </div>
          ) : filteredLogs.length === 0 ? (
            <div className="text-center py-8 text-gray-500">
              <svg className="w-16 h-16 mx-auto mb-4" fill="currentColor" viewBox="0 0 20 20">
                <path fillRule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-7-4a1 1 0 11-2 0 1 1 0 012 0zM9 9a1 1 0 000 2v3a1 1 0 001 1h1a1 1 0 100-2v-3a1 1 0 00-1-1H9z" clipRule="evenodd" />
              </svg>
              <p>No logs found matching your filters</p>
              <p className="text-sm">Try adjusting your search criteria</p>
            </div>
          ) : (
            <RecentLogs limit={filteredLogs.length} />
          )}
        </div>
      </div>
    </div>
  )
}