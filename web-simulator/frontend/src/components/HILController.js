import React, { useState } from 'react';

const HILController = ({ socket }) => {
  const [hilStatus, setHilStatus] = useState('Disconnected');
  const [deviceType, setDeviceType] = useState('PMB');
  const [connectionString, setConnectionString] = useState('COM3:115200');
  const [uploadStatus, setUploadStatus] = useState('');
  const [dragOver, setDragOver] = useState(false);

  const handleHILConnect = async () => {
    try {
      const response = await fetch('/api/hil/connect', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          deviceType,
          connectionString
        })
      });
      
      const result = await response.json();
      
      if (result.success) {
        setHilStatus('Connected');
        setUploadStatus(`✅ Connected to ${deviceType}`);
      } else {
        setHilStatus('Failed');
        setUploadStatus(`❌ Connection failed: ${result.message}`);
      }
    } catch (error) {
      setHilStatus('Error');
      setUploadStatus(`❌ Connection error: ${error.message}`);
    }
  };

  const handleHILDisconnect = async () => {
    try {
      const response = await fetch('/api/hil/disconnect', {
        method: 'POST'
      });
      
      const result = await response.json();
      
      if (result.success) {
        setHilStatus('Disconnected');
        setUploadStatus('🔌 Disconnected from HIL');
      }
    } catch (error) {
      setUploadStatus(`❌ Disconnect error: ${error.message}`);
    }
  };

  const handleFileUpload = async (file, endpoint, description) => {
    const formData = new FormData();
    formData.append('firmware', file);

    try {
      setUploadStatus(`📤 Uploading ${description}...`);
      
      const response = await fetch(endpoint, {
        method: 'POST',
        body: formData
      });
      
      const result = await response.json();
      
      if (result.success) {
        setUploadStatus(`✅ ${description} uploaded successfully (${(file.size / 1024).toFixed(1)} KB)`);
      } else {
        setUploadStatus(`❌ Upload failed: ${result.message}`);
      }
    } catch (error) {
      setUploadStatus(`❌ Upload error: ${error.message}`);
    }
  };

  const handleConfigUpload = async (file) => {
    const formData = new FormData();
    formData.append('config', file);

    try {
      setUploadStatus('📤 Uploading configuration...');
      
      const response = await fetch('/api/config', {
        method: 'POST',
        body: formData
      });
      
      const result = await response.json();
      
      if (result.success) {
        setUploadStatus('✅ Configuration uploaded successfully');
      } else {
        setUploadStatus(`❌ Config upload failed: ${result.message}`);
      }
    } catch (error) {
      setUploadStatus(`❌ Config upload error: ${error.message}`);
    }
  };

  const handleDrop = (e, handler) => {
    e.preventDefault();
    setDragOver(false);
    
    const files = e.dataTransfer.files;
    if (files.length > 0) {
      handler(files[0]);
    }
  };

  const handleDragOver = (e) => {
    e.preventDefault();
    setDragOver(true);
  };

  const handleDragLeave = (e) => {
    e.preventDefault();
    setDragOver(false);
  };

  const getStatusColor = () => {
    switch (hilStatus) {
      case 'Connected':
        return '#4CAF50';
      case 'Disconnected':
        return '#666';
      case 'Failed':
      case 'Error':
        return '#F44336';
      default:
        return '#FF9800';
    }
  };

  return (
    <div>
      <h3>🔧 HIL Controller</h3>
      
      <div className="status-item" style={{ marginBottom: '20px' }}>
        <span className="label">HIL Status:</span>
        <span 
          className="value" 
          style={{ color: getStatusColor() }}
        >
          {hilStatus}
        </span>
      </div>

      <div className="hil-grid">
        <div className="input-group">
          <label>Device Type:</label>
          <select 
            value={deviceType} 
            onChange={(e) => setDeviceType(e.target.value)}
          >
            <option value="PMB">PMB (Power Management)</option>
            <option value="IMU">IMU (Inertial Unit)</option>
            <option value="ESP32">ESP32 (Connectivity)</option>
          </select>
        </div>
        
        <div className="input-group">
          <label>Connection:</label>
          <input
            type="text"
            value={connectionString}
            onChange={(e) => setConnectionString(e.target.value)}
            placeholder="COM3:115200 or IP:PORT"
          />
        </div>
      </div>

      <div style={{ display: 'flex', gap: '10px', margin: '15px 0' }}>
        <button
          className="btn success"
          onClick={handleHILConnect}
          disabled={hilStatus === 'Connected'}
          style={{ 
            opacity: hilStatus === 'Connected' ? 0.5 : 1,
            flex: 1
          }}
        >
          🔌 Connect HIL
        </button>
        
        <button
          className="btn danger"
          onClick={handleHILDisconnect}
          disabled={hilStatus === 'Disconnected'}
          style={{ 
            opacity: hilStatus === 'Disconnected' ? 0.5 : 1,
            flex: 1
          }}
        >
          🔌 Disconnect
        </button>
      </div>

      <div className="hil-controls">
        <h4 style={{ margin: '10px 0', opacity: 0.9 }}>📡 Firmware & Config</h4>
        
        {/* ESP32 Firmware Upload */}
        <div 
          className={`upload-area ${dragOver ? 'dragover' : ''}`}
          onDrop={(e) => handleDrop(e, (file) => handleFileUpload(file, '/api/firmware/esp', 'ESP32 firmware'))}
          onDragOver={handleDragOver}
          onDragLeave={handleDragLeave}
          onClick={() => document.getElementById('esp-firmware').click()}
        >
          <div>📱 ESP32 Firmware (.bin)</div>
          <div style={{ fontSize: '0.8rem', opacity: 0.7 }}>
            Click or drag & drop firmware file
          </div>
          <input
            type="file"
            id="esp-firmware"
            accept=".bin"
            style={{ display: 'none' }}
            onChange={(e) => {
              if (e.target.files[0]) {
                handleFileUpload(e.target.files[0], '/api/firmware/esp', 'ESP32 firmware');
              }
            }}
          />
        </div>

        {/* PMB Firmware Upload */}
        <div 
          className={`upload-area ${dragOver ? 'dragover' : ''}`}
          onDrop={(e) => handleDrop(e, (file) => handleFileUpload(file, '/api/firmware/pmb', 'PMB firmware'))}
          onDragOver={handleDragOver}
          onDragLeave={handleDragLeave}
          onClick={() => document.getElementById('pmb-firmware').click()}
        >
          <div>⚡ PMB Firmware (.hex)</div>
          <div style={{ fontSize: '0.8rem', opacity: 0.7 }}>
            Click or drag & drop firmware file
          </div>
          <input
            type="file"
            id="pmb-firmware"
            accept=".hex,.bin"
            style={{ display: 'none' }}
            onChange={(e) => {
              if (e.target.files[0]) {
                handleFileUpload(e.target.files[0], '/api/firmware/pmb', 'PMB firmware');
              }
            }}
          />
        </div>

        {/* Configuration Upload */}
        <div 
          className={`upload-area ${dragOver ? 'dragover' : ''}`}
          onDrop={(e) => handleDrop(e, handleConfigUpload)}
          onDragOver={handleDragOver}
          onDragLeave={handleDragLeave}
          onClick={() => document.getElementById('config-file').click()}
        >
          <div>⚙️ Configuration (.json)</div>
          <div style={{ fontSize: '0.8rem', opacity: 0.7 }}>
            Click or drag & drop config.json
          </div>
          <input
            type="file"
            id="config-file"
            accept=".json"
            style={{ display: 'none' }}
            onChange={(e) => {
              if (e.target.files[0]) {
                handleConfigUpload(e.target.files[0]);
              }
            }}
          />
        </div>

        {uploadStatus && (
          <div style={{
            padding: '10px',
            marginTop: '15px',
            background: 'rgba(255, 255, 255, 0.1)',
            borderRadius: '8px',
            fontSize: '0.9rem',
            textAlign: 'center'
          }}>
            {uploadStatus}
          </div>
        )}
      </div>

      <div style={{ 
        marginTop: '20px', 
        padding: '15px', 
        background: 'rgba(255, 255, 255, 0.05)',
        borderRadius: '8px',
        fontSize: '0.85rem',
        opacity: 0.8 
      }}>
        <h5 style={{ margin: '0 0 10px 0' }}>HIL Integration:</h5>
        <p style={{ margin: '5px 0' }}>
          • Connect real hardware for testing
        </p>
        <p style={{ margin: '5px 0' }}>
          • Upload firmware over-the-air (OTA)
        </p>
        <p style={{ margin: '5px 0' }}>
          • Configure WiFi/MQTT settings
        </p>
        <p style={{ margin: '5px 0' }}>
          • Monitor real sensor data
        </p>
      </div>
    </div>
  );
};

export default HILController;