import { Router, Request, Response } from 'express';
import { ApiResponse, LogEntry } from '@melkens/shared';

// In-memory log storage for demo purposes
// In production, this would be connected to a proper logging system
const logs: LogEntry[] = [];

export function createLogsRoutes(): Router {
  const router = Router();

  // Get logs with optional filtering
  router.get('/', async (req: Request, res: Response) => {
    try {
      const { level, source, limit = '100', offset = '0' } = req.query;
      
      let filteredLogs = [...logs];
      
      // Filter by level
      if (level && typeof level === 'string') {
        filteredLogs = filteredLogs.filter(log => log.level === level);
      }
      
      // Filter by source
      if (source && typeof source === 'string') {
        filteredLogs = filteredLogs.filter(log => log.source === source);
      }
      
      // Sort by timestamp (newest first)
      filteredLogs.sort((a, b) => b.timestamp - a.timestamp);
      
      // Apply pagination
      const limitNum = parseInt(limit as string, 10);
      const offsetNum = parseInt(offset as string, 10);
      const paginatedLogs = filteredLogs.slice(offsetNum, offsetNum + limitNum);
      
      const response: ApiResponse = {
        success: true,
        data: {
          logs: paginatedLogs,
          total: filteredLogs.length,
          limit: limitNum,
          offset: offsetNum
        },
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error getting logs:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to get logs',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  });

  // Export logs as JSON
  router.get('/export', async (req: Request, res: Response) => {
    try {
      const { format = 'json' } = req.query;
      
      if (format === 'json') {
        res.setHeader('Content-Type', 'application/json');
        res.setHeader('Content-Disposition', `attachment; filename="robot_logs_${Date.now()}.json"`);
        res.json(logs);
      } else if (format === 'csv') {
        res.setHeader('Content-Type', 'text/csv');
        res.setHeader('Content-Disposition', `attachment; filename="robot_logs_${Date.now()}.csv"`);
        
        // CSV header
        let csv = 'timestamp,level,source,message,data\n';
        
        // CSV data
        logs.forEach(log => {
          const timestamp = new Date(log.timestamp).toISOString();
          const data = log.data ? JSON.stringify(log.data).replace(/"/g, '""') : '';
          csv += `"${timestamp}","${log.level}","${log.source}","${log.message.replace(/"/g, '""')}","${data}"\n`;
        });
        
        res.send(csv);
      } else {
        const response: ApiResponse = {
          success: false,
          error: 'Unsupported export format. Use json or csv',
          timestamp: Date.now()
        };
        res.status(400).json(response);
      }
    } catch (error) {
      console.error('Error exporting logs:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to export logs',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  });

  // Clear logs
  router.delete('/', async (req: Request, res: Response) => {
    try {
      const beforeCount = logs.length;
      logs.length = 0; // Clear array
      
      const response: ApiResponse = {
        success: true,
        data: { cleared: beforeCount },
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error clearing logs:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to clear logs',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  });

  return router;
}

// Function to add log entry (called by robot simulator)
export function addLogEntry(logEntry: LogEntry): void {
  logs.push(logEntry);
  
  // Keep only last 10000 logs to prevent memory issues
  if (logs.length > 10000) {
    logs.splice(0, logs.length - 10000);
  }
}