import { Request, Response } from 'express';
import { HardwareBridge, HardwareConfig } from '../services/HardwareBridge';
import { ApiResponse, RouteStep } from '@melkens/shared';

// Global hardware bridge instance
let hardwareBridge: HardwareBridge | null = null;

// Initialize hardware connection
export const initializeHardware = async (req: Request, res: Response): Promise<void> => {
  try {
    const config: HardwareConfig = req.body;
    
    if (!config.baudRate) {
      res.status(400).json({
        success: false,
        error: 'baudRate is required'
      } as ApiResponse<null>);
      return;
    }
    
    // Close existing connection if any
    if (hardwareBridge) {
      await hardwareBridge.close();
    }
    
    hardwareBridge = new HardwareBridge(config);
    await hardwareBridge.initialize();
    
    res.json({
      success: true,
      data: { initialized: true },
      message: 'Hardware bridge initialized successfully'
    } as ApiResponse<{ initialized: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to initialize hardware'
    } as ApiResponse<null>);
  }
};

// Get connection status
export const getConnectionStatus = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.json({
        success: true,
        data: { connected: false, mode: 'virtual', config: null },
        message: 'No hardware bridge initialized'
      } as ApiResponse<any>);
      return;
    }
    
    const status = hardwareBridge.getConnectionStatus();
    
    res.json({
      success: true,
      data: status,
      message: 'Connection status retrieved'
    } as ApiResponse<typeof status>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Send command to hardware
export const sendCommand = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    const { command, data } = req.body;
    
    if (!command) {
      res.status(400).json({
        success: false,
        error: 'command is required'
      } as ApiResponse<null>);
      return;
    }
    
    await hardwareBridge.sendCommand(command, data);
    
    res.json({
      success: true,
      data: { sent: true },
      message: `Command '${command}' sent to hardware`
    } as ApiResponse<{ sent: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to send command'
    } as ApiResponse<null>);
  }
};

// Send route to hardware
export const sendRoute = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    const route: RouteStep[] = req.body.route;
    
    if (!route || !Array.isArray(route)) {
      res.status(400).json({
        success: false,
        error: 'route array is required'
      } as ApiResponse<null>);
      return;
    }
    
    await hardwareBridge.sendRoute(route);
    
    res.json({
      success: true,
      data: { sent: true },
      message: `Route with ${route.length} steps sent to hardware`
    } as ApiResponse<{ sent: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to send route'
    } as ApiResponse<null>);
  }
};

// Start route execution on hardware
export const startRoute = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    await hardwareBridge.startRoute();
    
    res.json({
      success: true,
      data: { started: true },
      message: 'Route execution started on hardware'
    } as ApiResponse<{ started: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to start route'
    } as ApiResponse<null>);
  }
};

// Stop route execution on hardware
export const stopRoute = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    await hardwareBridge.stopRoute();
    
    res.json({
      success: true,
      data: { stopped: true },
      message: 'Route execution stopped on hardware'
    } as ApiResponse<{ stopped: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to stop route'
    } as ApiResponse<null>);
  }
};

// Emergency stop
export const emergencyStop = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    await hardwareBridge.emergencyStop();
    
    res.json({
      success: true,
      data: { emergencyStopped: true },
      message: 'Emergency stop executed on hardware'
    } as ApiResponse<{ emergencyStopped: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to execute emergency stop'
    } as ApiResponse<null>);
  }
};

// Set HIL mode
export const setHILMode = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    const { enabled } = req.body;
    
    if (typeof enabled !== 'boolean') {
      res.status(400).json({
        success: false,
        error: 'enabled boolean parameter is required'
      } as ApiResponse<null>);
      return;
    }
    
    hardwareBridge.setHILMode(enabled);
    
    res.json({
      success: true,
      data: { hilMode: enabled },
      message: `HIL mode ${enabled ? 'enabled' : 'disabled'}`
    } as ApiResponse<{ hilMode: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to set HIL mode'
    } as ApiResponse<null>);
  }
};

// Set mixed mode
export const setMixedMode = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    const { enabled } = req.body;
    
    if (typeof enabled !== 'boolean') {
      res.status(400).json({
        success: false,
        error: 'enabled boolean parameter is required'
      } as ApiResponse<null>);
      return;
    }
    
    hardwareBridge.setMixedMode(enabled);
    
    res.json({
      success: true,
      data: { mixedMode: enabled },
      message: `Mixed mode ${enabled ? 'enabled' : 'disabled'}`
    } as ApiResponse<{ mixedMode: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to set mixed mode'
    } as ApiResponse<null>);
  }
};

// Get hardware messages
export const getHardwareMessages = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    const limit = req.query.limit ? parseInt(req.query.limit as string) : undefined;
    const messages = hardwareBridge.getMessageBuffer(limit);
    
    res.json({
      success: true,
      data: messages,
      message: `Retrieved ${messages.length} hardware messages`
    } as ApiResponse<typeof messages>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to get hardware messages'
    } as ApiResponse<null>);
  }
};

// Export hardware logs
export const exportHardwareLogs = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.status(400).json({
        success: false,
        error: 'Hardware bridge not initialized'
      } as ApiResponse<null>);
      return;
    }
    
    const exportData = hardwareBridge.exportHardwareLogs();
    
    res.json({
      success: true,
      data: exportData,
      message: 'Hardware logs exported successfully'
    } as ApiResponse<typeof exportData>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to export hardware logs'
    } as ApiResponse<null>);
  }
};

// Close hardware connection
export const closeHardware = async (req: Request, res: Response): Promise<void> => {
  try {
    if (!hardwareBridge) {
      res.json({
        success: true,
        data: { closed: true },
        message: 'No hardware connection to close'
      } as ApiResponse<{ closed: boolean }>);
      return;
    }
    
    await hardwareBridge.close();
    hardwareBridge = null;
    
    res.json({
      success: true,
      data: { closed: true },
      message: 'Hardware connection closed'
    } as ApiResponse<{ closed: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to close hardware connection'
    } as ApiResponse<null>);
  }
};