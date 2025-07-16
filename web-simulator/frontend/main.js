/**
 * MELKENS HIL Simulator Frontend
 * Handles real-time visualization and user interactions
 */

class HILSimulator {
    constructor() {
        this.websocket = null;
        this.isConnected = false;
        this.currentTab = 'accelerometer';
        
        // Data buffers for plotting
        this.imuDataBuffer = {
            accelerometer: { x: [], y: [], z: [], timestamps: [] },
            gyroscope: { x: [], y: [], z: [], timestamps: [] },
            magnetometer: { x: [], y: [], z: [], timestamps: [] }
        };
        
        this.robotPathBuffer = { x: [], y: [] };
        this.maxDataPoints = 1000;
        
        // Configuration
        this.config = {
            updateRate: 50,
            maxPathPoints: 500
        };
        
        // Initialize the application
        this.init();
    }
    
    init() {
        this.setupEventListeners();
        this.initializePlots();
        this.connectWebSocket();
        this.setupUIUpdates();
        
        console.log('MELKENS HIL Simulator initialized');
    }
    
    setupEventListeners() {
        // Configuration buttons
        document.getElementById('apply-config').addEventListener('click', () => {
            this.applySystemConfiguration();
        });
        
        // Route control buttons
        document.getElementById('start-route').addEventListener('click', () => {
            this.controlRoute('start');
        });
        
        document.getElementById('pause-route').addEventListener('click', () => {
            this.controlRoute('pause');
        });
        
        document.getElementById('stop-route').addEventListener('click', () => {
            this.controlRoute('stop');
        });
        
        // Route selection
        document.getElementById('route-select').addEventListener('change', (e) => {
            if (e.target.value) {
                this.setRoute(e.target.value);
            }
        });
        
        // Fault injection
        document.getElementById('fault-enable').addEventListener('change', () => {
            this.updateFaultInjection();
        });
        
        document.getElementById('apply-fault').addEventListener('click', () => {
            this.updateFaultInjection();
        });
        
        // Fault severity slider
        document.getElementById('fault-severity').addEventListener('input', (e) => {
            document.getElementById('fault-severity-value').textContent = e.target.value;
        });
        
        // System actions
        document.getElementById('run-self-test').addEventListener('click', () => {
            this.runSelfTest();
        });
        
        document.getElementById('download-logs').addEventListener('click', () => {
            this.downloadLogs();
        });
        
        document.getElementById('reset-position').addEventListener('click', () => {
            this.resetPosition();
        });
        
        // Plot tabs
        document.querySelectorAll('.tab-button').forEach(button => {
            button.addEventListener('click', (e) => {
                this.switchTab(e.target.dataset.tab);
            });
        });
        
        // Modal controls
        document.querySelector('.close').addEventListener('click', () => {
            this.closeModal('self-test-modal');
        });
        
        // Close modal when clicking outside
        window.addEventListener('click', (e) => {
            if (e.target.classList.contains('modal')) {
                e.target.style.display = 'none';
            }
        });
    }
    
    connectWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        try {
            this.websocket = new WebSocket(wsUrl);
            
            this.websocket.onopen = () => {
                this.isConnected = true;
                this.addMessage('Connected to HIL Simulator', 'success');
                this.updateConnectionStatus(true);
            };
            
            this.websocket.onmessage = (event) => {
                const data = JSON.parse(event.data);
                this.handleWebSocketMessage(data);
            };
            
            this.websocket.onclose = () => {
                this.isConnected = false;
                this.addMessage('Disconnected from HIL Simulator', 'warning');
                this.updateConnectionStatus(false);
                
                // Attempt to reconnect after 3 seconds
                setTimeout(() => {
                    if (!this.isConnected) {
                        this.connectWebSocket();
                    }
                }, 3000);
            };
            
            this.websocket.onerror = (error) => {
                this.addMessage(`WebSocket error: ${error}`, 'error');
            };
            
        } catch (error) {
            this.addMessage(`Failed to connect: ${error}`, 'error');
        }
    }
    
    handleWebSocketMessage(data) {
        switch (data.type) {
            case 'imu_data':
                this.updateIMUData(data.data);
                break;
            case 'robot_position':
                this.updateRobotPosition(data.data);
                break;
            case 'system_status':
                this.updateSystemStatus(data.data);
                break;
            default:
                console.log('Unknown message type:', data.type);
        }
    }
    
    updateIMUData(data) {
        if (!data) return;
        
        const timestamp = Date.now();
        
        // Update real-time display values
        this.updateElement('acc-x', data.accelerometer?.x?.toFixed(2) || '0.00');
        this.updateElement('acc-y', data.accelerometer?.y?.toFixed(2) || '0.00');
        this.updateElement('acc-z', data.accelerometer?.z?.toFixed(2) || '0.00');
        
        this.updateElement('gyro-x', data.gyroscope?.x?.toFixed(3) || '0.000');
        this.updateElement('gyro-y', data.gyroscope?.y?.toFixed(3) || '0.000');
        this.updateElement('gyro-z', data.gyroscope?.z?.toFixed(3) || '0.000');
        
        this.updateElement('roll', (data.euler_angles?.roll * 180 / Math.PI)?.toFixed(1) || '0.0');
        this.updateElement('pitch', (data.euler_angles?.pitch * 180 / Math.PI)?.toFixed(1) || '0.0');
        this.updateElement('yaw', (data.euler_angles?.yaw * 180 / Math.PI)?.toFixed(1) || '0.0');
        
        this.updateElement('pos-x', data.position?.x?.toFixed(2) || '0.00');
        this.updateElement('pos-y', data.position?.y?.toFixed(2) || '0.00');
        
        // Update connection status
        const statusElement = document.getElementById('imu-status');
        if (data.hardware_connected) {
            statusElement.textContent = 'Connected';
            statusElement.className = 'status-value connected';
        } else {
            statusElement.textContent = 'Simulated';
            statusElement.className = 'status-value disconnected';
        }
        
        // Update data buffers for plotting
        this.addToDataBuffer('accelerometer', data.accelerometer, timestamp);
        this.addToDataBuffer('gyroscope', data.gyroscope, timestamp);
        this.addToDataBuffer('magnetometer', data.magnetometer, timestamp);
        
        // Update current plot if needed
        this.updateCurrentPlot();
    }
    
    updateRobotPosition(data) {
        if (!data) return;
        
        // Update robot position display
        this.updateElement('robot-angle', data.angle_degrees?.toFixed(1) || '0.0');
        this.updateElement('motor-left', data.motor_left_speed || '0');
        this.updateElement('motor-right', data.motor_right_speed || '0');
        this.updateElement('auger-speed', data.auger_speed || '0');
        
        // Update route progress
        if (data.current_route && data.total_steps > 0) {
            const progress = (data.current_step / data.total_steps) * 100;
            document.getElementById('route-progress').style.width = `${progress}%`;
            document.getElementById('route-step-info').textContent = 
                `Step ${data.current_step} of ${data.total_steps}`;
        }
        
        // Update route status
        this.updateElement('route-status', data.route_state || 'Idle');
        
        // Add to path buffer for robot plot
        if (data.x !== undefined && data.y !== undefined) {
            this.robotPathBuffer.x.push(data.x);
            this.robotPathBuffer.y.push(data.y);
            
            // Limit buffer size
            if (this.robotPathBuffer.x.length > this.config.maxPathPoints) {
                this.robotPathBuffer.x.shift();
                this.robotPathBuffer.y.shift();
            }
            
            this.updateRobotPlot(data);
        }
    }
    
    addToDataBuffer(sensor, sensorData, timestamp) {
        if (!sensorData) return;
        
        const buffer = this.imuDataBuffer[sensor];
        
        buffer.x.push(sensorData.x || 0);
        buffer.y.push(sensorData.y || 0);
        buffer.z.push(sensorData.z || 0);
        buffer.timestamps.push(timestamp);
        
        // Limit buffer size
        if (buffer.x.length > this.maxDataPoints) {
            buffer.x.shift();
            buffer.y.shift();
            buffer.z.shift();
            buffer.timestamps.shift();
        }
    }
    
    initializePlots() {
        // Initialize robot position plot
        this.initializeRobotPlot();
        
        // Initialize IMU sensor plot
        this.initializeIMUPlot();
    }
    
    initializeRobotPlot() {
        const layout = {
            title: 'Robot Position and Path',
            xaxis: { title: 'X Position (m)', range: [-5, 5] },
            yaxis: { title: 'Y Position (m)', range: [-5, 5] },
            showlegend: true,
            paper_bgcolor: '#3a3a3a',
            plot_bgcolor: '#2d2d2d',
            font: { color: '#ffffff' }
        };
        
        const data = [
            {
                x: [],
                y: [],
                mode: 'lines',
                name: 'Path',
                line: { color: '#4CAF50', width: 2 }
            },
            {
                x: [0],
                y: [0],
                mode: 'markers',
                name: 'Robot',
                marker: { 
                    size: 12, 
                    color: '#2196F3',
                    symbol: 'triangle-up'
                }
            }
        ];
        
        Plotly.newPlot('robot-plot', data, layout, { responsive: true });
    }
    
    initializeIMUPlot() {
        this.updateIMUPlot('accelerometer');
    }
    
    updateRobotPlot(robotData) {
        const pathTrace = {
            x: this.robotPathBuffer.x,
            y: this.robotPathBuffer.y
        };
        
        const robotTrace = {
            x: [robotData.x],
            y: [robotData.y]
        };
        
        Plotly.restyle('robot-plot', pathTrace, 0);
        Plotly.restyle('robot-plot', robotTrace, 1);
    }
    
    updateIMUPlot(sensor) {
        const buffer = this.imuDataBuffer[sensor];
        
        if (buffer.timestamps.length === 0) return;
        
        // Convert timestamps to relative time in seconds
        const startTime = buffer.timestamps[0];
        const relativeTime = buffer.timestamps.map(t => (t - startTime) / 1000);
        
        const data = [
            {
                x: relativeTime,
                y: buffer.x,
                mode: 'lines',
                name: 'X Axis',
                line: { color: '#f44336', width: 2 }
            },
            {
                x: relativeTime,
                y: buffer.y,
                mode: 'lines',
                name: 'Y Axis',
                line: { color: '#4CAF50', width: 2 }
            },
            {
                x: relativeTime,
                y: buffer.z,
                mode: 'lines',
                name: 'Z Axis',
                line: { color: '#2196F3', width: 2 }
            }
        ];
        
        let yAxisTitle;
        switch (sensor) {
            case 'accelerometer':
                yAxisTitle = 'Acceleration (m/s²)';
                break;
            case 'gyroscope':
                yAxisTitle = 'Angular Velocity (rad/s)';
                break;
            case 'magnetometer':
                yAxisTitle = 'Magnetic Field (µT)';
                break;
        }
        
        const layout = {
            title: `${sensor.charAt(0).toUpperCase() + sensor.slice(1)} Data`,
            xaxis: { title: 'Time (s)' },
            yaxis: { title: yAxisTitle },
            showlegend: true,
            paper_bgcolor: '#3a3a3a',
            plot_bgcolor: '#2d2d2d',
            font: { color: '#ffffff' }
        };
        
        Plotly.newPlot('imu-plot', data, layout, { responsive: true });
    }
    
    updateCurrentPlot() {
        if (this.currentTab) {
            this.updateIMUPlot(this.currentTab);
        }
    }
    
    switchTab(tab) {
        // Update tab buttons
        document.querySelectorAll('.tab-button').forEach(btn => {
            btn.classList.remove('active');
        });
        document.querySelector(`[data-tab="${tab}"]`).classList.add('active');
        
        this.currentTab = tab;
        this.updateIMUPlot(tab);
    }
    
    async applySystemConfiguration() {
        const config = {
            imu_mode: document.getElementById('imu-mode').value,
            serial_port: document.getElementById('serial-port').value,
            baud_rate: parseInt(document.getElementById('baud-rate').value),
            update_rate_ms: parseInt(document.getElementById('update-rate').value)
        };
        
        try {
            const response = await fetch('/api/config/system', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            });
            
            if (response.ok) {
                this.addMessage('Configuration applied successfully', 'success');
                
                // Update current mode display
                document.getElementById('current-mode').textContent = 
                    config.imu_mode === 'hardware' ? 'Hardware' : 'Simulated';
            } else {
                throw new Error(`HTTP ${response.status}`);
            }
        } catch (error) {
            this.addMessage(`Failed to apply configuration: ${error}`, 'error');
        }
    }
    
    async setRoute(routeId) {
        try {
            const response = await fetch(`/api/robot/route/${routeId}`, {
                method: 'POST'
            });
            
            if (response.ok) {
                this.addMessage(`Route ${routeId} selected`, 'info');
            } else {
                throw new Error(`HTTP ${response.status}`);
            }
        } catch (error) {
            this.addMessage(`Failed to set route: ${error}`, 'error');
        }
    }
    
    async controlRoute(action) {
        // This would need to be implemented on the backend
        // For now, just show a message
        this.addMessage(`Route ${action} requested`, 'info');
    }
    
    async updateFaultInjection() {
        const config = {
            enabled: document.getElementById('fault-enable').checked,
            fault_type: document.getElementById('fault-type').value,
            axis: document.getElementById('fault-axis').value,
            severity: parseFloat(document.getElementById('fault-severity').value)
        };
        
        try {
            const response = await fetch('/api/config/fault-injection', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            });
            
            if (response.ok) {
                this.addMessage('Fault injection configuration updated', 'info');
            } else {
                throw new Error(`HTTP ${response.status}`);
            }
        } catch (error) {
            this.addMessage(`Failed to update fault injection: ${error}`, 'error');
        }
    }
    
    async runSelfTest() {
        this.addMessage('Running IMU self-test...', 'info');
        
        try {
            const response = await fetch('/api/imu/self-test', {
                method: 'POST'
            });
            
            if (response.ok) {
                const results = await response.json();
                this.displaySelfTestResults(results);
            } else {
                throw new Error(`HTTP ${response.status}`);
            }
        } catch (error) {
            this.addMessage(`Self-test failed: ${error}`, 'error');
        }
    }
    
    displaySelfTestResults(results) {
        const resultsDiv = document.getElementById('self-test-results');
        resultsDiv.innerHTML = '';
        
        // Overall result
        const overallDiv = document.createElement('div');
        overallDiv.className = `test-result ${results.success ? 'passed' : 'failed'}`;
        overallDiv.innerHTML = `
            <div class="test-name">Overall Result: ${results.success ? 'PASSED' : 'FAILED'}</div>
            <div class="test-details">Timestamp: ${results.timestamp}</div>
        `;
        resultsDiv.appendChild(overallDiv);
        
        // Individual tests
        for (const [testName, testResult] of Object.entries(results.tests)) {
            const testDiv = document.createElement('div');
            testDiv.className = `test-result ${testResult.passed ? 'passed' : 'failed'}`;
            
            let details = '';
            if (testResult.error) {
                details = `Error: ${testResult.error}`;
            } else {
                details = Object.entries(testResult)
                    .filter(([key]) => key !== 'passed')
                    .map(([key, value]) => `${key}: ${JSON.stringify(value)}`)
                    .join('<br>');
            }
            
            testDiv.innerHTML = `
                <div class="test-name">${testName}: ${testResult.passed ? 'PASSED' : 'FAILED'}</div>
                <div class="test-details">${details}</div>
            `;
            resultsDiv.appendChild(testDiv);
        }
        
        this.showModal('self-test-modal');
    }
    
    async downloadLogs() {
        try {
            const response = await fetch('/api/logs');
            if (response.ok) {
                const logs = await response.json();
                
                // Create CSV content
                const csvContent = this.convertLogsToCSV(logs.logs);
                
                // Download file
                const blob = new Blob([csvContent], { type: 'text/csv' });
                const url = window.URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = `hil_logs_${new Date().toISOString().slice(0, 19)}.csv`;
                a.click();
                window.URL.revokeObjectURL(url);
                
                this.addMessage('Logs downloaded successfully', 'success');
            } else {
                throw new Error(`HTTP ${response.status}`);
            }
        } catch (error) {
            this.addMessage(`Failed to download logs: ${error}`, 'error');
        }
    }
    
    convertLogsToCSV(logs) {
        if (!logs || logs.length === 0) return 'No log data available';
        
        const headers = ['timestamp', 'type', 'message'];
        const rows = logs.map(log => [
            log.timestamp,
            log.type || 'data',
            JSON.stringify(log).replace(/"/g, '""')
        ]);
        
        return [headers, ...rows].map(row => row.join(',')).join('\n');
    }
    
    resetPosition() {
        // Clear robot path
        this.robotPathBuffer.x = [];
        this.robotPathBuffer.y = [];
        
        // Reset robot plot
        Plotly.restyle('robot-plot', { x: [[]], y: [[]] }, 0);
        Plotly.restyle('robot-plot', { x: [[0]], y: [[0]] }, 1);
        
        this.addMessage('Robot position reset', 'info');
    }
    
    setupUIUpdates() {
        // Update system status periodically
        setInterval(async () => {
            try {
                const response = await fetch('/api/imu/status');
                if (response.ok) {
                    const status = await response.json();
                    this.updateSystemStatusDisplay(status);
                }
            } catch (error) {
                // Silently fail - connection issues will be handled by WebSocket
            }
        }, 5000);
    }
    
    updateSystemStatusDisplay(status) {
        // Update data rate
        document.getElementById('data-rate').textContent = `${status.data_rate?.toFixed(1) || 0} Hz`;
        
        // Update packet info
        document.getElementById('packet-info').textContent = 
            `${status.packet_count || 0} / ${status.error_count || 0} errors`;
        
        // Update connection icon
        const connectionIcon = document.getElementById('connection-icon');
        const connectionStatus = document.getElementById('connection-status');
        
        if (status.hardware_connected) {
            connectionIcon.className = 'fas fa-wifi';
            connectionStatus.textContent = 'Connected';
        } else {
            connectionIcon.className = 'fas fa-wifi-slash';
            connectionStatus.textContent = 'Disconnected';
        }
    }
    
    updateConnectionStatus(connected) {
        const statusElement = document.getElementById('imu-status');
        if (connected) {
            statusElement.textContent = 'Connected';
            statusElement.className = 'status-value connected';
        } else {
            statusElement.textContent = 'Disconnected';
            statusElement.className = 'status-value disconnected';
        }
    }
    
    updateElement(id, value) {
        const element = document.getElementById(id);
        if (element) {
            element.textContent = value;
        }
    }
    
    addMessage(message, type = 'info') {
        const messageLog = document.getElementById('message-log');
        const messageDiv = document.createElement('div');
        messageDiv.className = `message ${type}`;
        messageDiv.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;
        
        messageLog.appendChild(messageDiv);
        messageLog.scrollTop = messageLog.scrollHeight;
        
        // Limit message count
        while (messageLog.children.length > 50) {
            messageLog.removeChild(messageLog.firstChild);
        }
        
        console.log(`[${type.toUpperCase()}] ${message}`);
    }
    
    showModal(modalId) {
        document.getElementById(modalId).style.display = 'block';
    }
    
    closeModal(modalId) {
        document.getElementById(modalId).style.display = 'none';
    }
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.hilSimulator = new HILSimulator();
});