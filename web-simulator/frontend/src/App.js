import React, { useState, useEffect } from 'react';
import io from 'socket.io-client';
import JoystickController from './components/JoystickController';
import StatusPanel from './components/StatusPanel';
import RouteController from './components/RouteController';
import HILController from './components/HILController';
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
    // Connect to WebSocket server
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

  const handleRouteSelect = (routeIndex) => {
    if (socket) {
      socket.emit('routeSelect', routeIndex);
    }
  };

  const handleRouteControl = (action) => {
    if (socket) {
      socket.emit('routeControl', action);
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
            <JoystickController 
              onJoystickChange={handleJoystickChange}
              x={robotState.joystickX}
              y={robotState.joystickY}
            />
            
            <div className="controls-grid">
              <div className="control-group">
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
              </div>
              
              <div className="control-group">
                <div className="checkbox-group">
                  <input 
                    type="checkbox" 
                    id="powerCheckbox"
                    checked={robotState.powerOn}
                    onChange={(e) => handlePowerControl(e.target.checked)}
                  />
                  <label htmlFor="powerCheckbox">Power</label>
                </div>
                
                <div className="checkbox-group">
                  <input 
                    type="checkbox" 
                    id="chargingCheckbox"
                    checked={robotState.charging}
                    onChange={(e) => handleChargingControl(e.target.checked)}
                  />
                  <label htmlFor="chargingCheckbox">Charging</label>
                </div>
              </div>
            </div>
          </div>

          <div className="card">
            <StatusPanel robotState={robotState} />
          </div>
        </div>

        <div className="main-content">
          <div className="card">
            <RouteController 
              currentRoute={robotState.currentRoute}
              routeStatus={robotState.routeStatus}
              onRouteSelect={handleRouteSelect}
              onRouteControl={handleRouteControl}
            />
          </div>

          <div className="card">
            <HILController socket={socket} />
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
            <p style={{ marginTop: '10px', opacity: 0.8, fontSize: '0.9rem' }}>
              Status: 0x{robotState.magnetBarStatus.toString(16).padStart(4, '0').toUpperCase()}
            </p>
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