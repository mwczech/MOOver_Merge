@echo off
REM MELKENS HIL Simulator Startup Script for Windows
REM This script starts the HIL simulator backend with proper environment setup

echo Starting MELKENS HIL Simulator...

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo Error: Python is not installed or not in PATH
    echo Please install Python 3.8 or higher from https://www.python.org/downloads/
    pause
    exit /b 1
)

REM Get current directory
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

REM Create virtual environment if it doesn't exist
if not exist "venv" (
    echo Creating virtual environment...
    python -m venv venv
)

REM Activate virtual environment
echo Activating virtual environment...
call venv\Scripts\activate.bat

REM Install/upgrade dependencies
echo Installing dependencies...
python -m pip install --upgrade pip
pip install -r requirements.txt

REM Create logs directory
echo Creating logs directory...
if not exist "..\..\logs" mkdir "..\..\logs"

REM Set environment variables
set PYTHONPATH=%SCRIPT_DIR%;%PYTHONPATH%
set MELKENS_HIL_LOG_LEVEL=INFO

REM Check for serial ports
echo Checking for available COM ports...
for /f "tokens=1" %%i in ('wmic path Win32_SerialPort get DeviceID /format:list ^| find "="') do (
    echo Found: %%i
)

REM Start the simulator
echo Starting HIL Simulator backend...
echo Access the web interface at: http://localhost:8000
echo Press Ctrl+C to stop the simulator
echo.

REM Start with auto-reload in development mode
if "%1"=="dev" (
    echo Starting in development mode with auto-reload...
    python main.py --reload --log-level debug
) else (
    python main.py
)

REM Cleanup
echo.
echo Shutting down HIL Simulator...
call venv\Scripts\deactivate.bat
pause