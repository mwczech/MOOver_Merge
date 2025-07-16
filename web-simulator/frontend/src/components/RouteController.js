import React from 'react';

const RouteController = ({ currentRoute, routeStatus, onRouteSelect, onRouteControl }) => {
  const routes = ['A', 'B', 'C', 'D', 'F', 'G', 'H', 'I', 'J', 'K'];

  const handleRouteChange = (e) => {
    const routeIndex = parseInt(e.target.value);
    if (onRouteSelect) {
      onRouteSelect(routeIndex);
    }
  };

  const handleRouteAction = (action) => {
    if (onRouteControl) {
      onRouteControl(action);
    }
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'playing':
        return '#4CAF50';
      case 'paused':
        return '#FF9800';
      case 'stopped':
        return '#F44336';
      default:
        return '#666';
    }
  };

  const getStatusIcon = (status) => {
    switch (status) {
      case 'playing':
        return '▶️';
      case 'paused':
        return '⏸️';
      case 'stopped':
        return '⏹️';
      default:
        return '⏹️';
    }
  };

  return (
    <div>
      <h3>🗺️ Route Controller</h3>
      
      <div className="route-selection">
        <h4 style={{ margin: '0 0 15px 0', opacity: 0.9 }}>Select Route:</h4>
        
        <div className="route-options">
          {routes.map((route, index) => (
            <label key={route}>
              <input
                type="radio"
                name="routeOption"
                value={index}
                checked={currentRoute === index}
                onChange={handleRouteChange}
              />
              {route}
            </label>
          ))}
        </div>
        
        <div style={{ 
          textAlign: 'center', 
          margin: '15px 0',
          opacity: 0.8,
          fontSize: '0.9rem'
        }}>
          Current Route: <strong>
            {currentRoute !== null ? routes[currentRoute] : 'None'}
          </strong>
        </div>
      </div>

      <div className="route-controls">
        <button
          className="btn success"
          onClick={() => handleRouteAction('playing')}
          disabled={currentRoute === null}
          style={{ 
            opacity: currentRoute === null ? 0.5 : 1,
            cursor: currentRoute === null ? 'not-allowed' : 'pointer'
          }}
        >
          ▶️ Play
        </button>
        
        <button
          className="btn warning"
          onClick={() => handleRouteAction('paused')}
          disabled={routeStatus !== 'playing'}
          style={{ 
            opacity: routeStatus !== 'playing' ? 0.5 : 1,
            cursor: routeStatus !== 'playing' ? 'not-allowed' : 'pointer'
          }}
        >
          ⏸️ Pause
        </button>
        
        <button
          className="btn danger"
          onClick={() => handleRouteAction('stopped')}
          disabled={routeStatus === 'stopped'}
          style={{ 
            opacity: routeStatus === 'stopped' ? 0.5 : 1,
            cursor: routeStatus === 'stopped' ? 'not-allowed' : 'pointer'
          }}
        >
          ⏹️ Stop
        </button>
      </div>

      <div style={{ marginTop: '20px' }}>
        <div className="status-item">
          <span className="label">Route Status:</span>
          <span 
            className="value" 
            style={{ 
              color: getStatusColor(routeStatus),
              fontWeight: 'bold'
            }}
          >
            {getStatusIcon(routeStatus)} {routeStatus.toUpperCase()}
          </span>
        </div>
      </div>

      <div style={{ 
        marginTop: '20px', 
        padding: '15px', 
        background: 'rgba(255, 255, 255, 0.05)',
        borderRadius: '8px',
        fontSize: '0.85rem',
        opacity: 0.8 
      }}>
        <h5 style={{ margin: '0 0 10px 0' }}>Route Information:</h5>
        <p style={{ margin: '5px 0' }}>
          • Routes A-K represent different predefined paths
        </p>
        <p style={{ margin: '5px 0' }}>
          • Select a route and press Play to start autonomous navigation
        </p>
        <p style={{ margin: '5px 0' }}>
          • Use Pause to temporarily halt execution
        </p>
        <p style={{ margin: '5px 0' }}>
          • Stop returns robot to manual control
        </p>
      </div>
    </div>
  );
};

export default RouteController;