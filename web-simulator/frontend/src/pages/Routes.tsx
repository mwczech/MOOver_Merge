import React, { useState } from 'react'
import { Plus, Edit, Trash2, Play, Copy, Eye } from 'lucide-react'
import { useRoutes, useRouteManagement } from '@/hooks/useApi'
import { Route, OperationType } from '@melkens/shared'
import { RouteEditor } from '@/components/RouteEditor'
import { RouteDetails } from '@/components/RouteDetails'

export const Routes: React.FC = () => {
  const { data: routes = [], isLoading } = useRoutes()
  const { executeRoute, deleteRoute } = useRouteManagement()
  const [selectedRoute, setSelectedRoute] = useState<Route | null>(null)
  const [editingRoute, setEditingRoute] = useState<Route | null>(null)
  const [showCreateForm, setShowCreateForm] = useState(false)
  const [viewMode, setViewMode] = useState<'list' | 'details' | 'edit'>('list')

  const handleCreateNew = () => {
    setEditingRoute(null)
    setShowCreateForm(true)
    setViewMode('edit')
  }

  const handleEdit = (route: Route) => {
    setEditingRoute(route)
    setShowCreateForm(false)
    setViewMode('edit')
  }

  const handleView = (route: Route) => {
    setSelectedRoute(route)
    setViewMode('details')
  }

  const handleExecute = (routeId: number) => {
    executeRoute.mutate(routeId)
  }

  const handleDelete = (routeId: number) => {
    if (window.confirm('Are you sure you want to delete this route?')) {
      deleteRoute.mutate(routeId)
    }
  }

  const handleClone = (route: Route) => {
    const clonedRoute = {
      ...route,
      name: `${route.name} (Copy)`,
      id: Date.now() // Temporary ID
    }
    setEditingRoute(clonedRoute)
    setShowCreateForm(false)
    setViewMode('edit')
  }

  const getOperationName = (operation: OperationType) => {
    switch (operation) {
      case OperationType.NORM: return 'Normal'
      case OperationType.TU_L: return 'Turn Left'
      case OperationType.TU_R: return 'Turn Right'
      case OperationType.L_90: return '90° Left'
      case OperationType.R_90: return '90° Right'
      case OperationType.DIFF: return 'Differential'
      case OperationType.NORM_NOMAGNET: return 'No Magnet'
      default: return 'Unknown'
    }
  }

  if (isLoading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="loading-spinner" />
        <span className="ml-2">Loading routes...</span>
      </div>
    )
  }

  if (viewMode === 'edit') {
    return (
      <RouteEditor
        route={editingRoute}
        isCreating={showCreateForm}
        onCancel={() => setViewMode('list')}
        onSave={() => setViewMode('list')}
      />
    )
  }

  if (viewMode === 'details' && selectedRoute) {
    return (
      <RouteDetails
        route={selectedRoute}
        onBack={() => setViewMode('list')}
        onEdit={() => handleEdit(selectedRoute)}
        onExecute={() => handleExecute(selectedRoute.id)}
      />
    )
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-bold text-gray-900">Routes Management</h1>
          <p className="text-gray-600">Create, edit, and manage robot routes</p>
        </div>
        <button
          onClick={handleCreateNew}
          className="btn btn-primary flex items-center space-x-2"
        >
          <Plus size={16} />
          <span>Create Route</span>
        </button>
      </div>

      {/* Stats */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
        <div className="card">
          <div className="card-content">
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm font-medium text-gray-600">Total Routes</p>
                <p className="text-2xl font-bold text-gray-900">{routes.length}</p>
              </div>
              <div className="text-melkens-600">
                <svg className="w-8 h-8" fill="currentColor" viewBox="0 0 20 20">
                  <path fillRule="evenodd" d="M3 4a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1zm0 4a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1zm0 4a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1z" clipRule="evenodd" />
                </svg>
              </div>
            </div>
          </div>
        </div>

        <div className="card">
          <div className="card-content">
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm font-medium text-gray-600">Active Routes</p>
                <p className="text-2xl font-bold text-gray-900">
                  {routes.filter(r => r.isActive).length}
                </p>
              </div>
              <div className="text-green-600">
                <svg className="w-8 h-8" fill="currentColor" viewBox="0 0 20 20">
                  <path fillRule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z" clipRule="evenodd" />
                </svg>
              </div>
            </div>
          </div>
        </div>

        <div className="card">
          <div className="card-content">
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm font-medium text-gray-600">Total Steps</p>
                <p className="text-2xl font-bold text-gray-900">
                  {routes.reduce((sum, route) => sum + route.steps.length, 0)}
                </p>
              </div>
              <div className="text-blue-600">
                <svg className="w-8 h-8" fill="currentColor" viewBox="0 0 20 20">
                  <path fillRule="evenodd" d="M12.293 5.293a1 1 0 011.414 0l4 4a1 1 0 010 1.414l-4 4a1 1 0 01-1.414-1.414L14.586 11H3a1 1 0 110-2h11.586l-2.293-2.293a1 1 0 010-1.414z" clipRule="evenodd" />
                </svg>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Routes List */}
      {routes.length === 0 ? (
        <div className="card">
          <div className="card-content text-center py-12">
            <div className="text-gray-400 mb-4">
              <svg className="w-16 h-16 mx-auto" fill="currentColor" viewBox="0 0 20 20">
                <path fillRule="evenodd" d="M3 4a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1zm0 4a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1zm0 4a1 1 0 011-1h12a1 1 0 110 2H4a1 1 0 01-1-1z" clipRule="evenodd" />
              </svg>
            </div>
            <h3 className="text-lg font-medium text-gray-900 mb-2">No routes found</h3>
            <p className="text-gray-600 mb-4">Get started by creating your first route</p>
            <button
              onClick={handleCreateNew}
              className="btn btn-primary"
            >
              Create Route
            </button>
          </div>
        </div>
      ) : (
        <div className="card">
          <div className="card-header">
            <h3 className="text-lg font-semibold">All Routes</h3>
          </div>
          <div className="card-content p-0">
            <div className="overflow-x-auto">
              <table className="w-full">
                <thead className="bg-gray-50">
                  <tr>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                      Route
                    </th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                      Steps
                    </th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                      Distance
                    </th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                      Status
                    </th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                      Actions
                    </th>
                  </tr>
                </thead>
                <tbody className="bg-white divide-y divide-gray-200">
                  {routes.map((route) => (
                    <tr key={route.id} className="hover:bg-gray-50">
                      <td className="px-6 py-4 whitespace-nowrap">
                        <div>
                          <div className="text-sm font-medium text-gray-900">
                            {route.name}
                          </div>
                          <div className="text-sm text-gray-500">
                            ID: {route.id} • Repeat: {route.repeatCount}x
                          </div>
                        </div>
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap">
                        <div className="text-sm text-gray-900">{route.steps.length}</div>
                        <div className="text-sm text-gray-500">
                          {route.steps.slice(0, 2).map(step => getOperationName(step.operation)).join(', ')}
                          {route.steps.length > 2 && '...'}
                        </div>
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap">
                        <div className="text-sm text-gray-900">
                          {route.steps.reduce((sum, step) => sum + step.distance, 0).toLocaleString()} mm
                        </div>
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap">
                        <span className={`
                          inline-flex px-2 py-1 text-xs font-semibold rounded-full
                          ${route.isActive 
                            ? 'bg-green-100 text-green-800' 
                            : 'bg-gray-100 text-gray-800'
                          }
                        `}>
                          {route.isActive ? 'Active' : 'Inactive'}
                        </span>
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm font-medium">
                        <div className="flex items-center space-x-2">
                          <button
                            onClick={() => handleView(route)}
                            className="text-blue-600 hover:text-blue-900"
                            title="View Details"
                          >
                            <Eye size={16} />
                          </button>
                          <button
                            onClick={() => handleExecute(route.id)}
                            className="text-green-600 hover:text-green-900"
                            disabled={route.isActive || executeRoute.isPending}
                            title="Execute Route"
                          >
                            <Play size={16} />
                          </button>
                          <button
                            onClick={() => handleEdit(route)}
                            className="text-indigo-600 hover:text-indigo-900"
                            disabled={route.isActive}
                            title="Edit Route"
                          >
                            <Edit size={16} />
                          </button>
                          <button
                            onClick={() => handleClone(route)}
                            className="text-purple-600 hover:text-purple-900"
                            title="Clone Route"
                          >
                            <Copy size={16} />
                          </button>
                          <button
                            onClick={() => handleDelete(route.id)}
                            className="text-red-600 hover:text-red-900"
                            disabled={route.isActive || deleteRoute.isPending}
                            title="Delete Route"
                          >
                            <Trash2 size={16} />
                          </button>
                        </div>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>
        </div>
      )}
    </div>
  )
}