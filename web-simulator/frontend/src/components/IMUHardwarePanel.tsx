import React, { useState, useEffect, useCallback } from 'react';
import {
  Box,
  Paper,
  Typography,
  Button,
  TextField,
  Select,
  MenuItem,
  FormControl,
  InputLabel,
  Switch,
  FormControlLabel,
  Card,
  CardContent,
  Grid,
  Alert,
  Chip,
  LinearProgress,
  Accordion,
  AccordionSummary,
  AccordionDetails,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Slider,
  Tabs,
  Tab,
  IconButton,
  Tooltip,
  CircularProgress
} from '@mui/material';
import {
  ExpandMore as ExpandMoreIcon,
  Settings as SettingsIcon,
  Download as DownloadIcon,
  Refresh as RefreshIcon,
  Warning as WarningIcon,
  CheckCircle as CheckCircleIcon,
  Error as ErrorIcon,
  PlayArrow as PlayArrowIcon,
  Stop as StopIcon,
  Wifi as WifiIcon,
  WifiOff as WifiOffIcon
} from '@mui/icons-material';

interface IMUSensorData {
  accelerometer: { x: number; y: number; z: number };
  gyroscope: { x: number; y: number; z: number };
  magnetometer: { x: number; y: number; z: number };
  ahrs: { roll: number; pitch: number; yaw: number };
  magnetBar: { status: number; detectedMagnets: number[] };
  timestamp: number;
  sequenceNumber: number;
  crcValid: boolean;
}

interface IMUStatus {
  connected: boolean;
  hardwareMode: boolean;
  port: string;
  lastDataTimestamp?: number;
  error?: string;
}

interface FaultInjectionConfig {
  enabled: boolean;
  accelerometerBias: { x: number; y: number; z: number };
  gyroscopeDrift: { x: number; y: number; z: number };
  magnetometerInterference: { x: number; y: number; z: number };
  stuckAxis: {
    accelerometer: string | null;
    gyroscope: string | null;
    magnetometer: string | null;
  };
  noiseLevel: number;
}

export const IMUHardwarePanel: React.FC = () => {
  const [imuStatus, setIMUStatus] = useState<IMUStatus>({
    connected: false,
    hardwareMode: false,
    port: 'Not connected'
  });
  
  const [sensorData, setSensorData] = useState<IMUSensorData | null>(null);
  const [availablePorts, setAvailablePorts] = useState<string[]>([]);
  const [selectedPort, setSelectedPort] = useState<string>('');
  const [isConnecting, setIsConnecting] = useState(false);
  const [isTesting, setIsTesting] = useState(false);
  const [faultConfig, setFaultConfig] = useState<FaultInjectionConfig>({
    enabled: false,
    accelerometerBias: { x: 0, y: 0, z: 0 },
    gyroscopeDrift: { x: 0, y: 0, z: 0 },
    magnetometerInterference: { x: 0, y: 0, z: 0 },
    stuckAxis: { accelerometer: null, gyroscope: null, magnetometer: null },
    noiseLevel: 0
  });
  
  const [showFaultDialog, setShowFaultDialog] = useState(false);
  const [activeTab, setActiveTab] = useState(0);
  const [dataHistory, setDataHistory] = useState<IMUSensorData[]>([]);
  const maxHistoryLength = 100;

  // Fetch IMU status
  const fetchIMUStatus = useCallback(async () => {
    try {
      const response = await fetch('/api/imu/status');
      const data = await response.json();
      if (data.success) {
        setIMUStatus(data.data);
      }
    } catch (error) {
      console.error('Failed to fetch IMU status:', error);
    }
  }, []);

  // Fetch sensor data
  const fetchSensorData = useCallback(async () => {
    try {
      const response = await fetch('/api/imu/data');
      const data = await response.json();
      if (data.success && data.data) {
        setSensorData(data.data);
        setDataHistory(prev => {
          const newHistory = [...prev, data.data];
          return newHistory.slice(-maxHistoryLength);
        });
      }
    } catch (error) {
      // Silently handle errors for continuous polling
    }
  }, []);

  // Detect available ports
  const detectPorts = useCallback(async () => {
    try {
      const response = await fetch('/api/imu/ports/detect');
      const data = await response.json();
      if (data.success) {
        setAvailablePorts(data.data.availablePorts);
        if (data.data.recommended && !selectedPort) {
          setSelectedPort(data.data.recommended);
        }
      }
    } catch (error) {
      console.error('Failed to detect ports:', error);
    }
  }, [selectedPort]);

  // Connect to IMU
  const connectIMU = async () => {
    setIsConnecting(true);
    try {
      const response = await fetch('/api/imu/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ port: selectedPort })
      });
      const data = await response.json();
      if (data.success) {
        await fetchIMUStatus();
      } else {
        console.error('Failed to connect:', data.error);
      }
    } catch (error) {
      console.error('Connection error:', error);
    } finally {
      setIsConnecting(false);
    }
  };

  // Disconnect from IMU
  const disconnectIMU = async () => {
    try {
      const response = await fetch('/api/imu/disconnect', { method: 'POST' });
      const data = await response.json();
      if (data.success) {
        await fetchIMUStatus();
        setSensorData(null);
        setDataHistory([]);
      }
    } catch (error) {
      console.error('Disconnect error:', error);
    }
  };

  // Toggle hardware mode
  const toggleHardwareMode = async () => {
    const endpoint = imuStatus.hardwareMode ? 'disable' : 'enable';
    try {
      const response = await fetch(`/api/imu/hardware-mode/${endpoint}`, { method: 'POST' });
      const data = await response.json();
      if (data.success) {
        await fetchIMUStatus();
      }
    } catch (error) {
      console.error('Failed to toggle hardware mode:', error);
    }
  };

  // Test connection
  const testConnection = async () => {
    setIsTesting(true);
    try {
      const response = await fetch('/api/imu/test?timeout=5000');
      const data = await response.json();
      if (data.success) {
        alert(data.data.message);
      }
    } catch (error) {
      console.error('Test failed:', error);
    } finally {
      setIsTesting(false);
    }
  };

  // Apply fault injection
  const applyFaultInjection = async () => {
    try {
      const response = await fetch('/api/imu/fault-injection', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(faultConfig)
      });
      const data = await response.json();
      if (data.success) {
        setShowFaultDialog(false);
      }
    } catch (error) {
      console.error('Failed to apply fault injection:', error);
    }
  };

  // Apply predefined fault scenario
  const applyFaultScenario = async (scenario: string) => {
    try {
      const response = await fetch(`/api/imu/fault-injection/scenario/${scenario}`, { method: 'POST' });
      const data = await response.json();
      if (data.success) {
        setFaultConfig(data.data);
      }
    } catch (error) {
      console.error('Failed to apply fault scenario:', error);
    }
  };

  // Download log file
  const downloadLog = () => {
    window.open('/api/imu/log/download', '_blank');
  };

  // Format values for display
  const formatValue = (value: number, decimals: number = 3) => {
    return value.toFixed(decimals);
  };

  // Get status color
  const getStatusColor = () => {
    if (!imuStatus.connected) return 'error';
    if (imuStatus.hardwareMode) return 'success';
    return 'warning';
  };

  // Setup polling and WebSocket listeners
  useEffect(() => {
    fetchIMUStatus();
    detectPorts();

    const statusInterval = setInterval(fetchIMUStatus, 2000);
    const dataInterval = setInterval(fetchSensorData, 100); // 10Hz for UI updates

    return () => {
      clearInterval(statusInterval);
      clearInterval(dataInterval);
    };
  }, [fetchIMUStatus, fetchSensorData, detectPorts]);

  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" gutterBottom>
        IMU Hardware-in-the-Loop Integration
      </Typography>

      {/* Status Panel */}
      <Paper sx={{ p: 2, mb: 3 }}>
        <Box display="flex" alignItems="center" justifyContent="space-between">
          <Box display="flex" alignItems="center" gap={2}>
            {imuStatus.connected ? (
              <WifiIcon color={getStatusColor()} />
            ) : (
              <WifiOffIcon color="error" />
            )}
            <Typography variant="h6">
              IMU Status: {imuStatus.connected ? 'Connected' : 'Disconnected'}
            </Typography>
            <Chip
              label={imuStatus.hardwareMode ? 'Hardware Mode' : 'Simulation Mode'}
              color={getStatusColor()}
              variant="outlined"
            />
          </Box>
          <Box display="flex" gap={1}>
            <Button
              variant="outlined"
              startIcon={<RefreshIcon />}
              onClick={detectPorts}
              size="small"
            >
              Detect Ports
            </Button>
            <Button
              variant="outlined"
              startIcon={<DownloadIcon />}
              onClick={downloadLog}
              size="small"
            >
              Download Log
            </Button>
          </Box>
        </Box>

        {imuStatus.error && (
          <Alert severity="error" sx={{ mt: 1 }}>
            {imuStatus.error}
          </Alert>
        )}

        <Box sx={{ mt: 2 }}>
          <Typography variant="body2" color="textSecondary">
            Port: {imuStatus.port} | 
            Last Update: {sensorData ? new Date(sensorData.timestamp).toLocaleTimeString() : 'Never'} |
            Sequence: {sensorData?.sequenceNumber || 'N/A'}
          </Typography>
        </Box>
      </Paper>

      {/* Connection Control */}
      <Paper sx={{ p: 2, mb: 3 }}>
        <Typography variant="h6" gutterBottom>
          Connection Control
        </Typography>
        
        <Grid container spacing={2} alignItems="center">
          <Grid item xs={12} md={4}>
            <FormControl fullWidth>
              <InputLabel>Serial Port</InputLabel>
              <Select
                value={selectedPort}
                onChange={(e) => setSelectedPort(e.target.value)}
                disabled={imuStatus.connected}
              >
                {availablePorts.map((port) => (
                  <MenuItem key={port} value={port}>
                    {port}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
          </Grid>
          
          <Grid item xs={12} md={8}>
            <Box display="flex" gap={1}>
              {!imuStatus.connected ? (
                <Button
                  variant="contained"
                  color="primary"
                  onClick={connectIMU}
                  disabled={isConnecting || !selectedPort}
                  startIcon={isConnecting ? <CircularProgress size={16} /> : <PlayArrowIcon />}
                >
                  {isConnecting ? 'Connecting...' : 'Connect'}
                </Button>
              ) : (
                <Button
                  variant="contained"
                  color="secondary"
                  onClick={disconnectIMU}
                  startIcon={<StopIcon />}
                >
                  Disconnect
                </Button>
              )}
              
              <Button
                variant="outlined"
                onClick={testConnection}
                disabled={!imuStatus.connected || isTesting}
                startIcon={isTesting ? <CircularProgress size={16} /> : <CheckCircleIcon />}
              >
                {isTesting ? 'Testing...' : 'Test'}
              </Button>
              
              <FormControlLabel
                control={
                  <Switch
                    checked={imuStatus.hardwareMode}
                    onChange={toggleHardwareMode}
                    disabled={!imuStatus.connected}
                  />
                }
                label="Hardware Mode"
              />
            </Box>
          </Grid>
        </Grid>
      </Paper>

      {/* Sensor Data Visualization */}
      {sensorData && (
        <Paper sx={{ p: 2, mb: 3 }}>
          <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 2 }}>
            <Tabs value={activeTab} onChange={(_, newValue) => setActiveTab(newValue)}>
              <Tab label="Live Data" />
              <Tab label="Magnetometer" />
              <Tab label="Magnet Bar" />
              <Tab label="Data History" />
            </Tabs>
          </Box>

          {activeTab === 0 && (
            <Grid container spacing={3}>
              {/* Accelerometer */}
              <Grid item xs={12} md={4}>
                <Card>
                  <CardContent>
                    <Typography variant="h6" gutterBottom>
                      Accelerometer (g)
                    </Typography>
                    <Typography>X: {formatValue(sensorData.accelerometer.x)}</Typography>
                    <Typography>Y: {formatValue(sensorData.accelerometer.y)}</Typography>
                    <Typography>Z: {formatValue(sensorData.accelerometer.z)}</Typography>
                  </CardContent>
                </Card>
              </Grid>

              {/* Gyroscope */}
              <Grid item xs={12} md={4}>
                <Card>
                  <CardContent>
                    <Typography variant="h6" gutterBottom>
                      Gyroscope (rad/s)
                    </Typography>
                    <Typography>X: {formatValue(sensorData.gyroscope.x)}</Typography>
                    <Typography>Y: {formatValue(sensorData.gyroscope.y)}</Typography>
                    <Typography>Z: {formatValue(sensorData.gyroscope.z)}</Typography>
                  </CardContent>
                </Card>
              </Grid>

              {/* AHRS */}
              <Grid item xs={12} md={4}>
                <Card>
                  <CardContent>
                    <Typography variant="h6" gutterBottom>
                      AHRS (degrees)
                    </Typography>
                    <Typography>Roll: {formatValue(sensorData.ahrs.roll, 1)}</Typography>
                    <Typography>Pitch: {formatValue(sensorData.ahrs.pitch, 1)}</Typography>
                    <Typography>Yaw: {formatValue(sensorData.ahrs.yaw, 1)}</Typography>
                  </CardContent>
                </Card>
              </Grid>
            </Grid>
          )}

          {activeTab === 1 && (
            <Grid container spacing={3}>
              <Grid item xs={12} md={6}>
                <Card>
                  <CardContent>
                    <Typography variant="h6" gutterBottom>
                      Magnetometer (gauss)
                    </Typography>
                    <Typography>X: {formatValue(sensorData.magnetometer.x)}</Typography>
                    <Typography>Y: {formatValue(sensorData.magnetometer.y)}</Typography>
                    <Typography>Z: {formatValue(sensorData.magnetometer.z)}</Typography>
                    <Typography sx={{ mt: 1 }}>
                      Magnitude: {formatValue(Math.sqrt(
                        sensorData.magnetometer.x ** 2 +
                        sensorData.magnetometer.y ** 2 +
                        sensorData.magnetometer.z ** 2
                      ))}
                    </Typography>
                  </CardContent>
                </Card>
              </Grid>
            </Grid>
          )}

          {activeTab === 2 && (
            <Grid container spacing={3}>
              <Grid item xs={12}>
                <Card>
                  <CardContent>
                    <Typography variant="h6" gutterBottom>
                      Magnetic Position Sensors
                    </Typography>
                    <Typography>
                      Status: 0x{sensorData.magnetBar.status.toString(16).toUpperCase()}
                    </Typography>
                    <Typography>
                      Detected Magnets: {sensorData.magnetBar.detectedMagnets.join(', ') || 'None'}
                    </Typography>
                    <Box sx={{ mt: 2 }}>
                      {Array.from({ length: 32 }, (_, i) => (
                        <Chip
                          key={i}
                          label={i - 15}
                          size="small"
                          color={sensorData.magnetBar.detectedMagnets.includes(i) ? 'primary' : 'default'}
                          sx={{ m: 0.25 }}
                        />
                      ))}
                    </Box>
                  </CardContent>
                </Card>
              </Grid>
            </Grid>
          )}

          {activeTab === 3 && (
            <TableContainer component={Paper}>
              <Table size="small">
                <TableHead>
                  <TableRow>
                    <TableCell>Time</TableCell>
                    <TableCell>Seq</TableCell>
                    <TableCell>Accel X</TableCell>
                    <TableCell>Accel Y</TableCell>
                    <TableCell>Accel Z</TableCell>
                    <TableCell>Gyro Z</TableCell>
                    <TableCell>Yaw</TableCell>
                    <TableCell>CRC</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {dataHistory.slice(-20).reverse().map((data, index) => (
                    <TableRow key={index}>
                      <TableCell>{new Date(data.timestamp).toLocaleTimeString()}</TableCell>
                      <TableCell>{data.sequenceNumber}</TableCell>
                      <TableCell>{formatValue(data.accelerometer.x)}</TableCell>
                      <TableCell>{formatValue(data.accelerometer.y)}</TableCell>
                      <TableCell>{formatValue(data.accelerometer.z)}</TableCell>
                      <TableCell>{formatValue(data.gyroscope.z)}</TableCell>
                      <TableCell>{formatValue(data.ahrs.yaw, 1)}</TableCell>
                      <TableCell>
                        {data.crcValid ? (
                          <CheckCircleIcon color="success" fontSize="small" />
                        ) : (
                          <ErrorIcon color="error" fontSize="small" />
                        )}
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </TableContainer>
          )}
        </Paper>
      )}

      {/* Fault Injection Panel */}
      <Paper sx={{ p: 2 }}>
        <Box display="flex" alignItems="center" justifyContent="between" mb={2}>
          <Typography variant="h6">
            Fault Injection
          </Typography>
          <Button
            variant="outlined"
            startIcon={<SettingsIcon />}
            onClick={() => setShowFaultDialog(true)}
          >
            Configure
          </Button>
        </Box>

        <Box display="flex" gap={1} flexWrap="wrap">
          {['gyro_drift', 'accel_bias', 'mag_interference', 'stuck_accel_x', 'stuck_gyro_z', 'high_noise', 'combined_faults'].map((scenario) => (
            <Button
              key={scenario}
              variant="outlined"
              size="small"
              onClick={() => applyFaultScenario(scenario)}
            >
              {scenario.replace(/_/g, ' ').toUpperCase()}
            </Button>
          ))}
        </Box>

        {faultConfig.enabled && (
          <Alert severity="warning" sx={{ mt: 2 }}>
            Fault injection is active! Sensor data may be artificially modified.
          </Alert>
        )}
      </Paper>

      {/* Fault Configuration Dialog */}
      <Dialog open={showFaultDialog} onClose={() => setShowFaultDialog(false)} maxWidth="md" fullWidth>
        <DialogTitle>Fault Injection Configuration</DialogTitle>
        <DialogContent>
          <FormControlLabel
            control={
              <Switch
                checked={faultConfig.enabled}
                onChange={(e) => setFaultConfig({ ...faultConfig, enabled: e.target.checked })}
              />
            }
            label="Enable Fault Injection"
            sx={{ mb: 2 }}
          />

          <Typography gutterBottom>Noise Level</Typography>
          <Slider
            value={faultConfig.noiseLevel}
            onChange={(_, value) => setFaultConfig({ ...faultConfig, noiseLevel: value as number })}
            min={0}
            max={1}
            step={0.01}
            marks
            valueLabelDisplay="auto"
          />

          {/* More fault configuration controls can be added here */}
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setShowFaultDialog(false)}>Cancel</Button>
          <Button onClick={applyFaultInjection} variant="contained">Apply</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};