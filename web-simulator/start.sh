#!/bin/bash
set -e

echo "🚀 Starting MELKENS HIL Simulator..."
echo "🔧 Port: ${PORT:-3000}"
echo "🐍 Python path: ${PYTHONPATH:-/app/backend}"

# Change to backend directory
cd /app/backend

# Create logs directory if it doesn't exist
mkdir -p logs logs/scenarios

# Start the FastAPI application
echo "🌟 Starting FastAPI server..."
exec python main.py