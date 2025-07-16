import { Request, Response, NextFunction } from 'express';
import { Schema } from 'joi';
import { ApiResponse } from '@melkens/shared';

export const validateBody = (schema: Schema) => {
  return (req: Request, res: Response, next: NextFunction): void => {
    const { error, value } = schema.validate(req.body);
    
    if (error) {
      const response: ApiResponse = {
        success: false,
        error: `Validation error: ${error.details[0].message}`,
        timestamp: Date.now()
      };
      res.status(400).json(response);
      return;
    }
    
    req.body = value;
    next();
  };
};

export const validateQuery = (schema: Schema) => {
  return (req: Request, res: Response, next: NextFunction): void => {
    const { error, value } = schema.validate(req.query);
    
    if (error) {
      const response: ApiResponse = {
        success: false,
        error: `Query validation error: ${error.details[0].message}`,
        timestamp: Date.now()
      };
      res.status(400).json(response);
      return;
    }
    
    req.query = value;
    next();
  };
};

export const validateParams = (schema: Schema) => {
  return (req: Request, res: Response, next: NextFunction): void => {
    const { error, value } = schema.validate(req.params);
    
    if (error) {
      const response: ApiResponse = {
        success: false,
        error: `Parameters validation error: ${error.details[0].message}`,
        timestamp: Date.now()
      };
      res.status(400).json(response);
      return;
    }
    
    req.params = value;
    next();
  };
};