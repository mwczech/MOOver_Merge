import React from 'react'
import { Routes, Route } from 'react-router-dom'
import { Layout } from '@/components/Layout'
import { Dashboard } from '@/pages/Dashboard'
import { Routes as RoutesPage } from '@/pages/Routes'
import { Logs } from '@/pages/Logs'
import { Settings } from '@/pages/Settings'
import { NotFound } from '@/pages/NotFound'
import { WebSocketProvider } from '@/hooks/useWebSocket'

function App() {
  return (
    <WebSocketProvider>
      <Layout>
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/routes" element={<RoutesPage />} />
          <Route path="/logs" element={<Logs />} />
          <Route path="/settings" element={<Settings />} />
          <Route path="*" element={<NotFound />} />
        </Routes>
      </Layout>
    </WebSocketProvider>
  )
}

export default App