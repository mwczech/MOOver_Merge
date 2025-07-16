#!/bin/bash

# MELKENS HIL Simulator Startup Script
# This script starts the HIL simulator backend with proper environment setup

echo "Starting MELKENS HIL Simulator..."

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is not installed or not in PATH"
    exit 1
fi

# Check Python version (require 3.8+)
python_version=$(python3 -c 'import sys; print(".".join(map(str, sys.version_info[:2])))')
required_version="3.8"

if [ "$(printf '%s\n' "$required_version" "$python_version" | sort -V | head -n1)" != "$required_version" ]; then
    echo "Error: Python $required_version or higher is required. Found $python_version"
    exit 1
fi

# Set script directory as working directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# Create virtual environment if it doesn't exist
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

# Activate virtual environment
echo "Activating virtual environment..."
source venv/bin/activate

# Install/upgrade dependencies
echo "Installing dependencies..."
pip install --upgrade pip
pip install -r requirements.txt

# Create logs directory
echo "Creating logs directory..."
mkdir -p ../../logs

# Set environment variables
export PYTHONPATH="${SCRIPT_DIR}:${PYTHONPATH}"
export MELKENS_HIL_LOG_LEVEL="INFO"

# Check for required permissions (Linux/macOS)
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "darwin"* ]]; then
    # Check if user is in dialout group (Linux)
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if ! groups | grep -q dialout; then
            echo "Warning: User is not in 'dialout' group. Serial port access may be restricted."
            echo "Run: sudo usermod -a -G dialout \$USER"
            echo "Then log out and back in."
        fi
    fi
    
    # Check for common serial ports
    if ls /dev/ttyUSB* 1> /dev/null 2>&1; then
        echo "Found USB serial devices:"
        ls -la /dev/ttyUSB*
    elif ls /dev/ttyACM* 1> /dev/null 2>&1; then
        echo "Found ACM serial devices:"
        ls -la /dev/ttyACM*
    elif ls /dev/cu.usb* 1> /dev/null 2>&1; then
        echo "Found USB serial devices:"
        ls -la /dev/cu.usb*
    else
        echo "Warning: No USB serial devices found. Simulator will start in simulation mode."
    fi
fi

# Function to handle cleanup on exit
cleanup() {
    echo ""
    echo "Shutting down HIL Simulator..."
    deactivate 2>/dev/null || true
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Start the simulator
echo "Starting HIL Simulator backend..."
echo "Access the web interface at: http://localhost:8000"
echo "Press Ctrl+C to stop the simulator"
echo ""

# Start with auto-reload in development mode
if [ "$1" = "dev" ]; then
    echo "Starting in development mode with auto-reload..."
    python main.py --reload --log-level debug
else
    python main.py
fi

# Cleanup on normal exit
cleanup