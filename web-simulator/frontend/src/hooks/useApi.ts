import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query'
import axios from 'axios'
import toast from 'react-hot-toast'
import type { 
  RobotState, 
  Route, 
  CreateRouteRequest, 
  UpdateRouteRequest,
  ApiResponse,
  LogEntry 
} from '@melkens/shared'

const api = axios.create({
  baseURL: '/api',
  timeout: 10000,
})

// Request interceptor
api.interceptors.request.use((config) => {
  config.headers['x-request-id'] = `req_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`
  return config
})

// Response interceptor
api.interceptors.response.use(
  (response) => response,
  (error) => {
    if (error.response?.data?.error) {
      toast.error(error.response.data.error)
    } else if (error.message) {
      toast.error(error.message)
    } else {
      toast.error('An unexpected error occurred')
    }
    return Promise.reject(error)
  }
)

// Robot API
export const useRobotState = () => {
  return useQuery({
    queryKey: ['robot', 'state'],
    queryFn: async (): Promise<RobotState> => {
      const response = await api.get<ApiResponse<RobotState>>('/robot/state')
      return response.data.data!
    },
    refetchInterval: 1000, // Refetch every second
  })
}

export const useRobotControl = () => {
  const queryClient = useQueryClient()

  const startRoute = useMutation({
    mutationFn: async (routeId: number) => {
      const response = await api.post('/robot/control/start', { routeId })
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['robot'] })
      toast.success('Route started successfully')
    }
  })

  const stopRobot = useMutation({
    mutationFn: async () => {
      const response = await api.post('/robot/control/stop')
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['robot'] })
      toast.success('Robot stopped')
    }
  })

  const emergencyStop = useMutation({
    mutationFn: async () => {
      const response = await api.post('/robot/control/emergency-stop')
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['robot'] })
      toast.error('Emergency stop activated')
    }
  })

  const resetPosition = useMutation({
    mutationFn: async () => {
      const response = await api.post('/robot/control/reset-position')
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['robot'] })
      toast.success('Position reset')
    }
  })

  return {
    startRoute,
    stopRobot,
    emergencyStop,
    resetPosition
  }
}

export const useRobotDiagnostics = () => {
  return useQuery({
    queryKey: ['robot', 'diagnostics'],
    queryFn: async () => {
      const response = await api.get('/robot/diagnostics')
      return response.data.data
    },
    refetchInterval: 5000, // Refetch every 5 seconds
  })
}

// Routes API
export const useRoutes = () => {
  return useQuery({
    queryKey: ['routes'],
    queryFn: async (): Promise<Route[]> => {
      const response = await api.get<ApiResponse<Route[]>>('/routes')
      return response.data.data!
    },
  })
}

export const useRoute = (id: number) => {
  return useQuery({
    queryKey: ['routes', id],
    queryFn: async (): Promise<Route> => {
      const response = await api.get<ApiResponse<Route>>(`/routes/${id}`)
      return response.data.data!
    },
    enabled: !!id,
  })
}

export const useRouteManagement = () => {
  const queryClient = useQueryClient()

  const createRoute = useMutation({
    mutationFn: async (route: CreateRouteRequest) => {
      const response = await api.post('/routes', route)
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['routes'] })
      toast.success('Route created successfully')
    }
  })

  const updateRoute = useMutation({
    mutationFn: async ({ id, ...route }: UpdateRouteRequest) => {
      const response = await api.put(`/routes/${id}`, route)
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['routes'] })
      toast.success('Route updated successfully')
    }
  })

  const deleteRoute = useMutation({
    mutationFn: async (id: number) => {
      const response = await api.delete(`/routes/${id}`)
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['routes'] })
      toast.success('Route deleted successfully')
    }
  })

  const executeRoute = useMutation({
    mutationFn: async (id: number) => {
      const response = await api.post(`/routes/${id}/execute`)
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['robot'] })
      toast.success('Route execution started')
    }
  })

  return {
    createRoute,
    updateRoute,
    deleteRoute,
    executeRoute
  }
}

// Logs API
export const useLogs = (filters?: { level?: string; source?: string; limit?: number; offset?: number }) => {
  return useQuery({
    queryKey: ['logs', filters],
    queryFn: async () => {
      const params = new URLSearchParams()
      if (filters?.level) params.append('level', filters.level)
      if (filters?.source) params.append('source', filters.source)
      if (filters?.limit) params.append('limit', filters.limit.toString())
      if (filters?.offset) params.append('offset', filters.offset.toString())

      const response = await api.get(`/logs?${params.toString()}`)
      return response.data.data
    },
    refetchInterval: 2000, // Refetch every 2 seconds
  })
}

export const useLogManagement = () => {
  const queryClient = useQueryClient()

  const clearLogs = useMutation({
    mutationFn: async () => {
      const response = await api.delete('/logs')
      return response.data
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['logs'] })
      toast.success('Logs cleared successfully')
    }
  })

  const exportLogs = async (format: 'json' | 'csv' = 'json') => {
    try {
      const response = await api.get(`/logs/export?format=${format}`, {
        responseType: 'blob'
      })
      
      const blob = new Blob([response.data])
      const url = window.URL.createObjectURL(blob)
      const link = document.createElement('a')
      link.href = url
      link.download = `robot_logs_${Date.now()}.${format}`
      document.body.appendChild(link)
      link.click()
      document.body.removeChild(link)
      window.URL.revokeObjectURL(url)
      
      toast.success(`Logs exported as ${format.toUpperCase()}`)
    } catch (error) {
      toast.error('Failed to export logs')
    }
  }

  return {
    clearLogs,
    exportLogs
  }
}

// Health check
export const useHealthCheck = () => {
  return useQuery({
    queryKey: ['health'],
    queryFn: async () => {
      const response = await api.get('/health')
      return response.data.data
    },
    refetchInterval: 30000, // Refetch every 30 seconds
  })
}