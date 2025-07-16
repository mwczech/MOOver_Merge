#!/bin/sh

echo "🚀 Starting MELKENS WB Robot Simulator..."

# Check if required directories exist
if [ ! -d "/app/backend/dist" ]; then
    echo "❌ Backend build not found!"
    exit 1
fi

if [ ! -d "/app/frontend/dist" ]; then
    echo "❌ Frontend build not found!"
    exit 1
fi

# Set environment variables with defaults
export NODE_ENV=${NODE_ENV:-production}
export PORT=${PORT:-3001}
export WS_PORT=${WS_PORT:-3002}
export HOST=${HOST:-0.0.0.0}

echo "📊 Environment: $NODE_ENV"
echo "🌐 HTTP Port: $PORT"
echo "🔌 WebSocket Port: $WS_PORT"

# Start the backend server
cd /app/backend
echo "🏁 Starting server..."
exec node dist/server.js