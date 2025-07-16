import React from 'react'
import { Link } from 'react-router-dom'
import { Home, ArrowLeft } from 'lucide-react'

export const NotFound: React.FC = () => {
  return (
    <div className="flex items-center justify-center min-h-96">
      <div className="text-center">
        <div className="text-6xl font-bold text-gray-300 mb-4">404</div>
        <h1 className="text-2xl font-bold text-gray-900 mb-2">Page Not Found</h1>
        <p className="text-gray-600 mb-8">
          The page you're looking for doesn't exist.
        </p>
        <div className="space-x-4">
          <Link
            to="/"
            className="btn btn-primary inline-flex items-center space-x-2"
          >
            <Home size={16} />
            <span>Go Home</span>
          </Link>
          <button
            onClick={() => window.history.back()}
            className="btn btn-outline inline-flex items-center space-x-2"
          >
            <ArrowLeft size={16} />
            <span>Go Back</span>
          </button>
        </div>
      </div>
    </div>
  )
}