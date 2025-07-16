import { Request, Response } from 'express';
import { validationEngine, ValidationResult, GoldenRun } from '../services/ValidationEngine';
import { ApiResponse, LogEntry, RobotState } from '@melkens/shared';

// Validate route execution
export const validateRoute = async (req: Request, res: Response): Promise<void> => {
  try {
    const { routeId, logs, finalState, metadata } = req.body;
    
    if (!routeId || !logs || !finalState) {
      res.status(400).json({
        success: false,
        error: 'routeId, logs, and finalState are required'
      } as ApiResponse<null>);
      return;
    }
    
    const result = await validationEngine.validateRoute(routeId, logs, finalState, metadata);
    
    res.json({
      success: true,
      data: result,
      message: `Route validation completed with status: ${result.status}`
    } as ApiResponse<ValidationResult>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Validation failed'
    } as ApiResponse<null>);
  }
};

// Save golden run
export const saveGoldenRun = async (req: Request, res: Response): Promise<void> => {
  try {
    const { routeId, logs, finalState, checkpoints, metadata } = req.body;
    
    if (!routeId || !logs || !finalState || !checkpoints) {
      res.status(400).json({
        success: false,
        error: 'routeId, logs, finalState, and checkpoints are required'
      } as ApiResponse<null>);
      return;
    }
    
    const goldenRunId = await validationEngine.saveGoldenRun(
      routeId,
      logs,
      finalState,
      checkpoints,
      metadata || {}
    );
    
    res.json({
      success: true,
      data: { goldenRunId },
      message: `Golden run saved with ID: ${goldenRunId}`
    } as ApiResponse<{ goldenRunId: string }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to save golden run'
    } as ApiResponse<null>);
  }
};

// Get validation result by ID
export const getValidationResult = async (req: Request, res: Response): Promise<void> => {
  try {
    const { validationId } = req.params;
    
    const result = validationEngine.getValidationResult(validationId);
    
    if (!result) {
      res.status(404).json({
        success: false,
        error: `Validation result ${validationId} not found`
      } as ApiResponse<null>);
      return;
    }
    
    res.json({
      success: true,
      data: result,
      message: 'Validation result retrieved'
    } as ApiResponse<ValidationResult>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to get validation result'
    } as ApiResponse<null>);
  }
};

// Get golden run by route ID
export const getGoldenRun = async (req: Request, res: Response): Promise<void> => {
  try {
    const { routeId } = req.params;
    
    const goldenRun = validationEngine.getGoldenRun(routeId);
    
    if (!goldenRun) {
      res.status(404).json({
        success: false,
        error: `Golden run for route ${routeId} not found`
      } as ApiResponse<null>);
      return;
    }
    
    res.json({
      success: true,
      data: goldenRun,
      message: 'Golden run retrieved'
    } as ApiResponse<GoldenRun>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to get golden run'
    } as ApiResponse<null>);
  }
};

// Export validation results
export const exportValidationResults = async (req: Request, res: Response): Promise<void> => {
  try {
    const exportData = validationEngine.exportValidationResults();
    
    res.json({
      success: true,
      data: exportData,
      message: 'Validation results exported successfully'
    } as ApiResponse<typeof exportData>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to export validation results'
    } as ApiResponse<null>);
  }
};

// Run validation with comparison
export const runValidationWithComparison = async (req: Request, res: Response): Promise<void> => {
  try {
    const { routeId, logs, finalState, compareWithGolden = true, metadata } = req.body;
    
    if (!routeId || !logs || !finalState) {
      res.status(400).json({
        success: false,
        error: 'routeId, logs, and finalState are required'
      } as ApiResponse<null>);
      return;
    }
    
    // Check if golden run exists
    const goldenRun = validationEngine.getGoldenRun(routeId);
    
    if (compareWithGolden && !goldenRun) {
      res.status(400).json({
        success: false,
        error: `No golden run found for route ${routeId}. Create a golden run first or set compareWithGolden to false.`
      } as ApiResponse<null>);
      return;
    }
    
    const result = await validationEngine.validateRoute(routeId, logs, finalState, metadata);
    
    const response = {
      validation: result,
      goldenRunAvailable: !!goldenRun,
      comparisonPerformed: !!result.goldenRunComparison
    };
    
    res.json({
      success: true,
      data: response,
      message: `Validation completed with ${result.goldenRunComparison ? 'golden run comparison' : 'no comparison'}`
    } as ApiResponse<typeof response>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Validation with comparison failed'
    } as ApiResponse<null>);
  }
};

// Generate certification report
export const generateCertificationReport = async (req: Request, res: Response): Promise<void> => {
  try {
    const { validationId } = req.params;
    
    const result = validationEngine.getValidationResult(validationId);
    
    if (!result) {
      res.status(404).json({
        success: false,
        error: `Validation result ${validationId} not found`
      } as ApiResponse<null>);
      return;
    }
    
    // Generate certification report
    const report = {
      certificationId: `cert_${Date.now()}`,
      validationId: result.id,
      routeId: result.routeId,
      timestamp: Date.now(),
      status: result.status,
      score: result.score,
      certified: result.status === 'PASS' && result.score >= 90,
      summary: {
        totalChecks: result.checks.length,
        passedChecks: result.checks.filter(c => c.status === 'pass').length,
        failedChecks: result.checks.filter(c => c.status === 'fail').length,
        warningChecks: result.checks.filter(c => c.status === 'warning').length,
        skippedChecks: result.checks.filter(c => c.status === 'skipped').length
      },
      checks: result.checks,
      goldenRunComparison: result.goldenRunComparison,
      recommendations: generateRecommendations(result),
      checksum: result.checksum,
      metadata: result.metadata
    };
    
    res.json({
      success: true,
      data: report,
      message: `Certification report generated for validation ${validationId}`
    } as ApiResponse<typeof report>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Failed to generate certification report'
    } as ApiResponse<null>);
  }
};

// Auto-validate all routes
export const autoValidateAllRoutes = async (req: Request, res: Response): Promise<void> => {
  try {
    const { routes, logs, metadata } = req.body;
    
    if (!routes || !Array.isArray(routes)) {
      res.status(400).json({
        success: false,
        error: 'routes array is required'
      } as ApiResponse<null>);
      return;
    }
    
    const validationResults = [];
    
    for (const route of routes) {
      if (route.logs && route.finalState) {
        try {
          const result = await validationEngine.validateRoute(
            route.id,
            route.logs,
            route.finalState,
            { ...metadata, routeName: route.name }
          );
          validationResults.push(result);
        } catch (error) {
          console.error(`Failed to validate route ${route.id}:`, error);
          validationResults.push({
            id: `failed_${route.id}`,
            routeId: route.id,
            status: 'FAIL',
            error: error instanceof Error ? error.message : 'Unknown error'
          });
        }
      }
    }
    
    const summary = {
      totalRoutes: routes.length,
      validatedRoutes: validationResults.length,
      passedRoutes: validationResults.filter(r => r.status === 'PASS').length,
      failedRoutes: validationResults.filter(r => r.status === 'FAIL').length,
      warningRoutes: validationResults.filter(r => r.status === 'WARNING').length,
      averageScore: validationResults.length > 0 
        ? Math.round(validationResults.reduce((sum, r) => sum + (r.score || 0), 0) / validationResults.length)
        : 0
    };
    
    res.json({
      success: true,
      data: {
        summary,
        results: validationResults
      },
      message: `Auto-validation completed: ${summary.passedRoutes}/${summary.totalRoutes} routes passed`
    } as ApiResponse<{ summary: typeof summary; results: typeof validationResults }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Auto-validation failed'
    } as ApiResponse<null>);
  }
};

// Generate recommendations based on validation result
function generateRecommendations(result: ValidationResult): string[] {
  const recommendations: string[] = [];
  
  if (result.score < 60) {
    recommendations.push('CRITICAL: Overall score below 60%. Route requires significant improvements before deployment.');
  } else if (result.score < 90) {
    recommendations.push('WARNING: Score below 90%. Consider addressing failed checks before production use.');
  }
  
  const failedChecks = result.checks.filter(c => c.status === 'fail');
  if (failedChecks.length > 0) {
    recommendations.push(`Address ${failedChecks.length} failed validation checks.`);
    
    const safetyFails = failedChecks.filter(c => c.category === 'safety');
    if (safetyFails.length > 0) {
      recommendations.push('CRITICAL: Safety checks failed. Do not deploy until resolved.');
    }
  }
  
  const warningChecks = result.checks.filter(c => c.status === 'warning');
  if (warningChecks.length > 0) {
    recommendations.push(`Consider improving ${warningChecks.length} checks with warnings.`);
  }
  
  if (result.goldenRunComparison) {
    if (result.goldenRunComparison.similarity < 80) {
      recommendations.push('Route execution differs significantly from golden run. Review deviations.');
    }
    
    const criticalDeviations = result.goldenRunComparison.deviations.filter(d => d.severity === 'critical');
    if (criticalDeviations.length > 0) {
      recommendations.push('CRITICAL: Critical deviations from golden run detected.');
    }
  }
  
  if (recommendations.length === 0) {
    recommendations.push('Route validation successful. No issues detected.');
  }
  
  return recommendations;
}