import React from 'react';

const StatusPanel = ({ robotState }) => {
  const {
    pmbConnection,
    motorRightSpeed,
    motorLeftSpeed,
    batteryVoltage,
    adcCurrent,
    thumbleCurrent,
    crcImu2PmbErrorCount,
    crcPmb2ImuErrorCount,
    crcEsp2ImuErrorCount,
    powerOn,
    charging
  } = robotState;

  const getConnectionColor = (connection) => {
    if (connection === "Connected" || connection === "HIL Connected") {
      return '#4CAF50';
    } else if (connection === "Disconnected") {
      return '#F44336';
    }
    return '#FF9800';
  };

  const getBatteryColor = (voltage) => {
    if (voltage > 12000) return '#4CAF50'; // Green
    if (voltage > 11000) return '#FF9800'; // Orange
    return '#F44336'; // Red
  };

  const formatVoltage = (voltage) => {
    return (voltage / 1000).toFixed(2) + 'V';
  };

  return (
    <div>
      <h3>📊 Robot Status</h3>
      
      <div className="status-grid">
        <div className="status-item">
          <span className="label">PMB Connection:</span>
          <span 
            className="value" 
            style={{ color: getConnectionColor(pmbConnection) }}
          >
            {pmbConnection}
          </span>
        </div>
        
        <div className="status-item">
          <span className="label">Power Status:</span>
          <span 
            className="value" 
            style={{ color: powerOn ? '#4CAF50' : '#F44336' }}
          >
            {powerOn ? 'ON' : 'OFF'}
          </span>
        </div>
        
        <div className="status-item">
          <span className="label">Motor Right:</span>
          <span className="value">{motorRightSpeed} RPM</span>
        </div>
        
        <div className="status-item">
          <span className="label">Motor Left:</span>
          <span className="value">{motorLeftSpeed} RPM</span>
        </div>
        
        <div className="status-item">
          <span className="label">Battery:</span>
          <span 
            className="value" 
            style={{ color: getBatteryColor(batteryVoltage) }}
          >
            {formatVoltage(batteryVoltage)}
          </span>
        </div>
        
        <div className="status-item">
          <span className="label">Charging:</span>
          <span 
            className="value" 
            style={{ color: charging ? '#4CAF50' : '#666' }}
          >
            {charging ? 'YES' : 'NO'}
          </span>
        </div>
        
        <div className="status-item">
          <span className="label">ADC Current:</span>
          <span className="value">{Math.round(adcCurrent)} mA</span>
        </div>
        
        <div className="status-item">
          <span className="label">Thumble Current:</span>
          <span className="value">{Math.round(thumbleCurrent)} mA</span>
        </div>
      </div>
      
      <div style={{ marginTop: '20px' }}>
        <h4 style={{ margin: '10px 0', opacity: 0.9 }}>🚨 Error Counters</h4>
        <div className="status-grid">
          <div className="status-item">
            <span className="label">IMU → PMB:</span>
            <span 
              className="value"
              style={{ color: crcImu2PmbErrorCount > 0 ? '#F44336' : '#4CAF50' }}
            >
              {crcImu2PmbErrorCount}
            </span>
          </div>
          
          <div className="status-item">
            <span className="label">PMB → IMU:</span>
            <span 
              className="value"
              style={{ color: crcPmb2ImuErrorCount > 0 ? '#F44336' : '#4CAF50' }}
            >
              {crcPmb2ImuErrorCount}
            </span>
          </div>
          
          <div className="status-item">
            <span className="label">ESP → IMU:</span>
            <span 
              className="value"
              style={{ color: crcEsp2ImuErrorCount > 0 ? '#F44336' : '#4CAF50' }}
            >
              {crcEsp2ImuErrorCount}
            </span>
          </div>
          
          <div className="status-item">
            <span className="label">Total Errors:</span>
            <span 
              className="value"
              style={{ 
                color: (crcImu2PmbErrorCount + crcPmb2ImuErrorCount + crcEsp2ImuErrorCount) > 0 ? '#F44336' : '#4CAF50'
              }}
            >
              {crcImu2PmbErrorCount + crcPmb2ImuErrorCount + crcEsp2ImuErrorCount}
            </span>
          </div>
        </div>
      </div>
    </div>
  );
};

export default StatusPanel;