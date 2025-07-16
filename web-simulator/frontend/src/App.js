import React, { useState, useEffect } from 'react';
import io from 'socket.io-client';
import './App.css';

function App() {
  const [socket, setSocket] = useState(null);
  const [connected, setConnected] = useState(false);
  const [robotState, setRobotState] = useState({
    motorRightSpeed: 0,
    motorLeftSpeed: 0,
    augerSpeed: 50,
    batteryVoltage: 12400,
    adcCurrent: 250,
    thumbleCurrent: 180,
    powerOn: false,
    charging: false,
    pmbConnection: "Disconnected",
    crcImu2PmbErrorCount: 0,
    crcPmb2ImuErrorCount: 0,
    crcEsp2ImuErrorCount: 0,
    magnetBarStatus: 0x0000,
    currentRoute: null,
    routeStatus: "stopped",
    joystickX: 0,
    joystickY: 0
  });

  useEffect(() => {
    const socketConnection = io(window.location.origin);
    
    socketConnection.on('connect', () => {
      console.log('Connected to server');
      setConnected(true);
    });

    socketConnection.on('disconnect', () => {
      console.log('Disconnected from server');
      setConnected(false);
    });

    socketConnection.on('robotState', (state) => {
      setRobotState(state);
    });

    setSocket(socketConnection);

    return () => {
      socketConnection.disconnect();
    };
  }, []);

  const handleJoystickChange = (x, y) => {
    if (socket) {
      socket.emit('joystick', { x, y });
    }
  };

  const handlePowerControl = (powerOn) => {
    if (socket) {
      socket.emit('powerControl', powerOn);
    }
  };

  const handleChargingControl = (charging) => {
    if (socket) {
      socket.emit('chargingControl', charging);
    }
  };

  const handleAugerSpeed = (speed) => {
    if (socket) {
      socket.emit('augerSpeed', speed);
    }
  };

  return (
    <div className="App">
      <div className="container">
        <header className="header">
          <h1>🤖 MOOver MELKENS</h1>
          <p>Web Simulator with HIL Integration</p>
          <div className={`connection-status ${connected ? 'connected' : 'disconnected'}`}>
            <span className={connected ? '🟢' : '🔴'}></span>
            {connected ? 'Connected' : 'Disconnected'}
          </div>
        </header>

        <div className="main-content">
          <div className="card">
            <h3>🕹️ Motion Controller</h3>
            <div className="joystick-container">
              <div className="joystick" onMouseMove={(e) => {
                const rect = e.currentTarget.getBoundingClientRect();
                const x = ((e.clientX - rect.left) / rect.width - 0.5) * 200;
                const y = -((e.clientY - rect.top) / rect.height - 0.5) * 200;
                handleJoystickChange(Math.max(-100, Math.min(100, x)), Math.max(-100, Math.min(100, y)));
              }}>
                <div className="joystick-handle" style={{
                  left: `${50 + robotState.joystickX/2}%`,
                  top: `${50 - robotState.joystickY/2}%`
                }}></div>
              </div>
              <div className="joystick-values">
                <div>X: <span>{Math.round(robotState.joystickX)}</span></div>
                <div>Y: <span>{Math.round(robotState.joystickY)}</span></div>
              </div>
            </div>
            
            <div className="controls">
              <div className="slider-container">
                <label>Auger Speed: {robotState.augerSpeed}</label>
                <input 
                  type="range" 
                  min="0" 
                  max="1500" 
                  value={robotState.augerSpeed}
                  onChange={(e) => handleAugerSpeed(parseInt(e.target.value))}
                />
              </div>
              
              <div className="checkbox-controls">
                <label>
                  <input 
                    type="checkbox" 
                    checked={robotState.powerOn}
                    onChange={(e) => handlePowerControl(e.target.checked)}
                  />
                  Power
                </label>
                
                <label>
                  <input 
                    type="checkbox" 
                    checked={robotState.charging}
                    onChange={(e) => handleChargingControl(e.target.checked)}
                  />
                  Charging
                </label>
              </div>
            </div>
          </div>

          <div className="card">
            <h3>📊 Robot Status</h3>
            <div className="status-grid">
              <div className="status-item">
                <span>PMB Connection:</span>
                <span style={{ color: robotState.pmbConnection === "Connected" ? '#4CAF50' : '#F44336' }}>
                  {robotState.pmbConnection}
                </span>
              </div>
              <div className="status-item">
                <span>Motor Right:</span>
                <span>{robotState.motorRightSpeed} RPM</span>
              </div>
              <div className="status-item">
                <span>Motor Left:</span>
                <span>{robotState.motorLeftSpeed} RPM</span>
              </div>
              <div className="status-item">
                <span>Battery:</span>
                <span style={{ color: robotState.batteryVoltage > 12000 ? '#4CAF50' : '#F44336' }}>
                  {(robotState.batteryVoltage / 1000).toFixed(2)}V
                </span>
              </div>
              <div className="status-item">
                <span>ADC Current:</span>
                <span>{Math.round(robotState.adcCurrent)} mA</span>
              </div>
              <div className="status-item">
                <span>CRC Errors:</span>
                <span>{robotState.crcImu2PmbErrorCount + robotState.crcPmb2ImuErrorCount + robotState.crcEsp2ImuErrorCount}</span>
              </div>
            </div>
          </div>
        </div>

        <div className="card">
          <h3>🧲 Magnetic Line Sensor</h3>
          <div className="magnet-bar">
            <div className="bit-indicators">
              {Array.from({ length: 16 }, (_, i) => (
                <div 
                  key={i} 
                  className={`bit ${(robotState.magnetBarStatus & (1 << i)) ? 'on' : ''}`}
                  title={`Bit ${i}`}
                >
                  {i}
                </div>
              ))}
            </div>
            <p>Status: 0x{robotState.magnetBarStatus.toString(16).padStart(4, '0').toUpperCase()}</p>
          </div>
        </div>

        <footer style={{ textAlign: 'center', marginTop: '40px', opacity: 0.7 }}>
          <p>&copy; 2024 Melkens Sp. z o.o. - MOOver MELKENS Robot Web Simulator</p>
        </footer>
      </div>
    </div>
  );
}

export default App;
