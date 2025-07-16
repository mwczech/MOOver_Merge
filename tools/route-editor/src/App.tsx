import React, { useState, useRef, useCallback } from 'react';
import { 
  Save, 
  FolderOpen, 
  Download, 
  Upload, 
  Play, 
  Settings, 
  Grid,
  Trash2,
  RotateCcw,
  Info
} from 'lucide-react';

interface RoutePoint {
  id: string;
  x: number;
  y: number;
  type: 'normal' | 'turn_left' | 'turn_right' | 'dock' | 'stop';
  speed: number;
  duration?: number;
  magneticCorrection?: number;
}

interface Route {
  id: string;
  name: string;
  description: string;
  points: RoutePoint[];
  settings: {
    defaultSpeed: number;
    turnRadius: number;
    safetyDistance: number;
  };
  metadata: {
    created: Date;
    modified: Date;
    version: string;
  };
}

const POINT_TYPES = [
  { value: 'normal', label: 'Normal', color: '#3b82f6' },
  { value: 'turn_left', label: 'Turn Left', color: '#ef4444' },
  { value: 'turn_right', label: 'Turn Right', color: '#22c55e' },
  { value: 'dock', label: 'Dock', color: '#f59e0b' },
  { value: 'stop', label: 'Stop', color: '#8b5cf6' }
];

export default function App() {
  const [route, setRoute] = useState<Route>({
    id: 'route_' + Date.now(),
    name: 'New Route',
    description: 'Created with Route Editor',
    points: [],
    settings: {
      defaultSpeed: 0.3,
      turnRadius: 0.5,
      safetyDistance: 0.1
    },
    metadata: {
      created: new Date(),
      modified: new Date(),
      version: '1.0'
    }
  });

  const [selectedPoint, setSelectedPoint] = useState<string | null>(null);
  const [selectedTool, setSelectedTool] = useState<'normal' | 'turn_left' | 'turn_right' | 'dock' | 'stop'>('normal');
  const [showGrid, setShowGrid] = useState(true);
  const [zoom, setZoom] = useState(1);
  const [pan, setPan] = useState({ x: 0, y: 0 });
  const [showSettings, setShowSettings] = useState(false);

  const canvasRef = useRef<HTMLDivElement>(null);
  const fileInputRef = useRef<HTMLInputElement>(null);

  const handleCanvasClick = useCallback((e: React.MouseEvent) => {
    if (!canvasRef.current) return;

    const rect = canvasRef.current.getBoundingClientRect();
    const x = (e.clientX - rect.left - pan.x) / zoom;
    const y = (e.clientY - rect.top - pan.y) / zoom;

    // Check if clicking on existing point
    const clickedPoint = route.points.find(point => {
      const dx = point.x - x;
      const dy = point.y - y;
      return Math.sqrt(dx * dx + dy * dy) < 15;
    });

    if (clickedPoint) {
      setSelectedPoint(clickedPoint.id);
    } else {
      // Add new point
      const newPoint: RoutePoint = {
        id: 'point_' + Date.now(),
        x,
        y,
        type: selectedTool,
        speed: route.settings.defaultSpeed
      };

      setRoute(prev => ({
        ...prev,
        points: [...prev.points, newPoint],
        metadata: { ...prev.metadata, modified: new Date() }
      }));
    }
  }, [route.points, selectedTool, zoom, pan, route.settings.defaultSpeed]);

  const deletePoint = useCallback((pointId: string) => {
    setRoute(prev => ({
      ...prev,
      points: prev.points.filter(p => p.id !== pointId),
      metadata: { ...prev.metadata, modified: new Date() }
    }));
    setSelectedPoint(null);
  }, []);

  const updatePoint = useCallback((pointId: string, updates: Partial<RoutePoint>) => {
    setRoute(prev => ({
      ...prev,
      points: prev.points.map(p => p.id === pointId ? { ...p, ...updates } : p),
      metadata: { ...prev.metadata, modified: new Date() }
    }));
  }, []);

  const exportRoute = useCallback(() => {
    // Convert to firmware-compatible format
    const exportData = {
      route: {
        name: route.name,
        description: route.description,
        steps: route.points.map((point, index) => ({
          id: index,
          operation: point.type === 'normal' ? 'NORM' : 
                    point.type === 'turn_left' ? 'TU_L' :
                    point.type === 'turn_right' ? 'TU_R' :
                    point.type === 'dock' ? 'DOCK' :
                    point.type === 'stop' ? 'STOP' : 'NORM',
          position: { x: Math.round(point.x), y: Math.round(point.y) },
          speed: point.speed,
          duration: point.duration || 0,
          magneticCorrection: point.magneticCorrection || 0
        })),
        settings: route.settings
      },
      metadata: {
        ...route.metadata,
        exportedAt: new Date(),
        format: 'melkens_route_v1'
      }
    };

    const blob = new Blob([JSON.stringify(exportData, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `${route.name.replace(/\s+/g, '_')}.route.json`;
    a.click();
    URL.revokeObjectURL(url);
  }, [route]);

  const importRoute = useCallback((event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const data = JSON.parse(e.target?.result as string);
        if (data.route) {
          const importedRoute: Route = {
            id: 'route_' + Date.now(),
            name: data.route.name || 'Imported Route',
            description: data.route.description || 'Imported from file',
            points: data.route.steps?.map((step: any, index: number) => ({
              id: 'point_' + Date.now() + '_' + index,
              x: step.position?.x || 0,
              y: step.position?.y || 0,
              type: step.operation === 'TU_L' ? 'turn_left' :
                    step.operation === 'TU_R' ? 'turn_right' :
                    step.operation === 'DOCK' ? 'dock' :
                    step.operation === 'STOP' ? 'stop' : 'normal',
              speed: step.speed || 0.3,
              duration: step.duration,
              magneticCorrection: step.magneticCorrection
            })) || [],
            settings: data.route.settings || route.settings,
            metadata: {
              created: new Date(),
              modified: new Date(),
              version: data.metadata?.version || '1.0'
            }
          };
          setRoute(importedRoute);
        }
      } catch (error) {
        alert('Failed to import route: Invalid file format');
      }
    };
    reader.readAsText(file);
  }, [route.settings]);

  const clearRoute = useCallback(() => {
    if (confirm('Clear all points? This cannot be undone.')) {
      setRoute(prev => ({
        ...prev,
        points: [],
        metadata: { ...prev.metadata, modified: new Date() }
      }));
      setSelectedPoint(null);
    }
  }, []);

  const selectedPointData = selectedPoint ? route.points.find(p => p.id === selectedPoint) : null;

  return (
    <div className="h-screen flex flex-col bg-gray-100">
      {/* Header */}
      <header className="bg-white border-b border-gray-200 px-6 py-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center space-x-4">
            <h1 className="text-2xl font-bold text-gray-900">MELKENS Route Editor</h1>
            <span className="px-3 py-1 bg-blue-100 text-blue-800 text-sm rounded-full">
              v1.0.0
            </span>
          </div>
          
          <div className="flex items-center space-x-2">
            <button
              onClick={() => fileInputRef.current?.click()}
              className="flex items-center space-x-2 px-4 py-2 bg-gray-100 hover:bg-gray-200 rounded-lg"
            >
              <FolderOpen size={16} />
              <span>Open</span>
            </button>
            
            <button
              onClick={() => {/* Save functionality */}}
              className="flex items-center space-x-2 px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-lg"
            >
              <Save size={16} />
              <span>Save</span>
            </button>
            
            <button
              onClick={exportRoute}
              className="flex items-center space-x-2 px-4 py-2 bg-green-600 hover:bg-green-700 text-white rounded-lg"
            >
              <Download size={16} />
              <span>Export</span>
            </button>
            
            <button
              onClick={() => setShowSettings(!showSettings)}
              className="flex items-center space-x-2 px-4 py-2 bg-gray-100 hover:bg-gray-200 rounded-lg"
            >
              <Settings size={16} />
              <span>Settings</span>
            </button>
          </div>
        </div>
        
        <input
          ref={fileInputRef}
          type="file"
          accept=".json,.route.json"
          onChange={importRoute}
          className="hidden"
        />
      </header>

      <div className="flex flex-1">
        {/* Toolbar */}
        <div className="w-64 bg-white border-r border-gray-200 p-4">
          <div className="space-y-6">
            {/* Route Info */}
            <div>
              <h3 className="text-lg font-semibold mb-3">Route Info</h3>
              <div className="space-y-2">
                <input
                  type="text"
                  value={route.name}
                  onChange={(e) => setRoute(prev => ({ ...prev, name: e.target.value }))}
                  className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                  placeholder="Route name"
                />
                <textarea
                  value={route.description}
                  onChange={(e) => setRoute(prev => ({ ...prev, description: e.target.value }))}
                  className="w-full px-3 py-2 border border-gray-300 rounded-lg h-20 resize-none"
                  placeholder="Route description"
                />
                <div className="text-sm text-gray-500">
                  Points: {route.points.length}
                </div>
              </div>
            </div>

            {/* Tools */}
            <div>
              <h3 className="text-lg font-semibold mb-3">Tools</h3>
              <div className="space-y-2">
                {POINT_TYPES.map(type => (
                  <button
                    key={type.value}
                    onClick={() => setSelectedTool(type.value as any)}
                    className={`w-full flex items-center space-x-3 px-3 py-2 rounded-lg text-left ${
                      selectedTool === type.value 
                        ? 'bg-blue-100 border-2 border-blue-300' 
                        : 'bg-gray-50 hover:bg-gray-100 border-2 border-transparent'
                    }`}
                  >
                    <div 
                      className="w-4 h-4 rounded-full"
                      style={{ backgroundColor: type.color }}
                    />
                    <span>{type.label}</span>
                  </button>
                ))}
              </div>
            </div>

            {/* Canvas Controls */}
            <div>
              <h3 className="text-lg font-semibold mb-3">View</h3>
              <div className="space-y-2">
                <button
                  onClick={() => setShowGrid(!showGrid)}
                  className={`w-full flex items-center space-x-2 px-3 py-2 rounded-lg ${
                    showGrid ? 'bg-blue-100 text-blue-700' : 'bg-gray-100'
                  }`}
                >
                  <Grid size={16} />
                  <span>Show Grid</span>
                </button>
                
                <div className="flex items-center space-x-2">
                  <span className="text-sm">Zoom:</span>
                  <input
                    type="range"
                    min="0.5"
                    max="3"
                    step="0.1"
                    value={zoom}
                    onChange={(e) => setZoom(parseFloat(e.target.value))}
                    className="flex-1"
                  />
                  <span className="text-sm w-12">{Math.round(zoom * 100)}%</span>
                </div>
              </div>
            </div>

            {/* Actions */}
            <div>
              <h3 className="text-lg font-semibold mb-3">Actions</h3>
              <div className="space-y-2">
                <button
                  onClick={clearRoute}
                  className="w-full flex items-center space-x-2 px-3 py-2 bg-red-100 hover:bg-red-200 text-red-700 rounded-lg"
                >
                  <Trash2 size={16} />
                  <span>Clear Route</span>
                </button>
              </div>
            </div>
          </div>
        </div>

        {/* Canvas */}
        <div className="flex-1 relative overflow-hidden">
          <div
            ref={canvasRef}
            className="w-full h-full cursor-crosshair"
            onClick={handleCanvasClick}
            style={{
              backgroundImage: showGrid ? `
                linear-gradient(to right, #e5e7eb 1px, transparent 1px),
                linear-gradient(to bottom, #e5e7eb 1px, transparent 1px)
              ` : undefined,
              backgroundSize: showGrid ? `${20 * zoom}px ${20 * zoom}px` : undefined,
              backgroundPosition: `${pan.x}px ${pan.y}px`
            }}
          >
            {/* Route visualization */}
            <svg 
              className="absolute inset-0 w-full h-full pointer-events-none"
              style={{ transform: `translate(${pan.x}px, ${pan.y}px) scale(${zoom})` }}
            >
              {/* Route path */}
              {route.points.length > 1 && (
                <polyline
                  points={route.points.map(p => `${p.x},${p.y}`).join(' ')}
                  fill="none"
                  stroke="#3b82f6"
                  strokeWidth="2"
                  strokeDasharray="5,5"
                />
              )}
              
              {/* Points */}
              {route.points.map((point, index) => {
                const pointType = POINT_TYPES.find(t => t.value === point.type);
                const isSelected = selectedPoint === point.id;
                
                return (
                  <g key={point.id}>
                    <circle
                      cx={point.x}
                      cy={point.y}
                      r={isSelected ? 12 : 8}
                      fill={pointType?.color}
                      stroke={isSelected ? "#1f2937" : "white"}
                      strokeWidth={isSelected ? 3 : 2}
                      className="pointer-events-auto cursor-pointer"
                      onClick={(e) => {
                        e.stopPropagation();
                        setSelectedPoint(point.id);
                      }}
                    />
                    <text
                      x={point.x}
                      y={point.y + 25}
                      textAnchor="middle"
                      className="text-xs fill-gray-600 pointer-events-none"
                    >
                      {index + 1}
                    </text>
                  </g>
                );
              })}
            </svg>
          </div>

          {/* Point details panel */}
          {selectedPointData && (
            <div className="absolute top-4 right-4 w-80 bg-white rounded-lg shadow-lg border border-gray-200 p-4">
              <div className="flex items-center justify-between mb-4">
                <h3 className="text-lg font-semibold">Point Settings</h3>
                <button
                  onClick={() => deletePoint(selectedPointData.id)}
                  className="p-2 text-red-600 hover:bg-red-100 rounded-lg"
                >
                  <Trash2 size={16} />
                </button>
              </div>
              
              <div className="space-y-4">
                <div>
                  <label className="block text-sm font-medium mb-1">Type</label>
                  <select
                    value={selectedPointData.type}
                    onChange={(e) => updatePoint(selectedPointData.id, { type: e.target.value as any })}
                    className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                  >
                    {POINT_TYPES.map(type => (
                      <option key={type.value} value={type.value}>
                        {type.label}
                      </option>
                    ))}
                  </select>
                </div>
                
                <div>
                  <label className="block text-sm font-medium mb-1">Speed (m/s)</label>
                  <input
                    type="number"
                    min="0.1"
                    max="2.0"
                    step="0.1"
                    value={selectedPointData.speed}
                    onChange={(e) => updatePoint(selectedPointData.id, { speed: parseFloat(e.target.value) })}
                    className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                  />
                </div>
                
                <div>
                  <label className="block text-sm font-medium mb-1">Position</label>
                  <div className="flex space-x-2">
                    <input
                      type="number"
                      value={Math.round(selectedPointData.x)}
                      onChange={(e) => updatePoint(selectedPointData.id, { x: parseInt(e.target.value) })}
                      className="flex-1 px-3 py-2 border border-gray-300 rounded-lg"
                      placeholder="X"
                    />
                    <input
                      type="number"
                      value={Math.round(selectedPointData.y)}
                      onChange={(e) => updatePoint(selectedPointData.id, { y: parseInt(e.target.value) })}
                      className="flex-1 px-3 py-2 border border-gray-300 rounded-lg"
                      placeholder="Y"
                    />
                  </div>
                </div>
                
                {(selectedPointData.type === 'turn_left' || selectedPointData.type === 'turn_right') && (
                  <div>
                    <label className="block text-sm font-medium mb-1">Duration (ms)</label>
                    <input
                      type="number"
                      min="100"
                      max="5000"
                      step="100"
                      value={selectedPointData.duration || 1000}
                      onChange={(e) => updatePoint(selectedPointData.id, { duration: parseInt(e.target.value) })}
                      className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                    />
                  </div>
                )}
                
                <div>
                  <label className="block text-sm font-medium mb-1">Magnetic Correction</label>
                  <input
                    type="number"
                    min="-50"
                    max="50"
                    step="1"
                    value={selectedPointData.magneticCorrection || 0}
                    onChange={(e) => updatePoint(selectedPointData.id, { magneticCorrection: parseInt(e.target.value) })}
                    className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                  />
                </div>
              </div>
            </div>
          )}
        </div>
      </div>

      {/* Settings Modal */}
      {showSettings && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
          <div className="bg-white rounded-lg w-96 p-6">
            <div className="flex items-center justify-between mb-4">
              <h3 className="text-lg font-semibold">Route Settings</h3>
              <button
                onClick={() => setShowSettings(false)}
                className="text-gray-400 hover:text-gray-600"
              >
                ×
              </button>
            </div>
            
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium mb-1">Default Speed (m/s)</label>
                <input
                  type="number"
                  min="0.1"
                  max="2.0"
                  step="0.1"
                  value={route.settings.defaultSpeed}
                  onChange={(e) => setRoute(prev => ({
                    ...prev,
                    settings: { ...prev.settings, defaultSpeed: parseFloat(e.target.value) }
                  }))}
                  className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium mb-1">Turn Radius (m)</label>
                <input
                  type="number"
                  min="0.1"
                  max="2.0"
                  step="0.1"
                  value={route.settings.turnRadius}
                  onChange={(e) => setRoute(prev => ({
                    ...prev,
                    settings: { ...prev.settings, turnRadius: parseFloat(e.target.value) }
                  }))}
                  className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                />
              </div>
              
              <div>
                <label className="block text-sm font-medium mb-1">Safety Distance (m)</label>
                <input
                  type="number"
                  min="0.05"
                  max="1.0"
                  step="0.05"
                  value={route.settings.safetyDistance}
                  onChange={(e) => setRoute(prev => ({
                    ...prev,
                    settings: { ...prev.settings, safetyDistance: parseFloat(e.target.value) }
                  }))}
                  className="w-full px-3 py-2 border border-gray-300 rounded-lg"
                />
              </div>
            </div>
            
            <div className="flex justify-end space-x-2 mt-6">
              <button
                onClick={() => setShowSettings(false)}
                className="px-4 py-2 bg-gray-100 hover:bg-gray-200 rounded-lg"
              >
                Cancel
              </button>
              <button
                onClick={() => setShowSettings(false)}
                className="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-lg"
              >
                Save
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}