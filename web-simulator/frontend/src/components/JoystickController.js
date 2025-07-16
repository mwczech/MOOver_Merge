import React, { useRef, useEffect, useState } from 'react';

const JoystickController = ({ onJoystickChange, x, y }) => {
  const canvasRef = useRef(null);
  const [dragging, setDragging] = useState(false);
  const [currentX, setCurrentX] = useState(150); // Center X
  const [currentY, setCurrentY] = useState(150); // Center Y

  const CANVAS_SIZE = 300;
  const CENTER_X = CANVAS_SIZE / 2;
  const CENTER_Y = CANVAS_SIZE / 2;
  const RADIUS = 100;
  const HANDLE_RADIUS = 20;

  useEffect(() => {
    // Update canvas position based on external x, y values
    const newX = CENTER_X + (x * RADIUS / 100);
    const newY = CENTER_Y - (y * RADIUS / 100); // Invert Y axis
    setCurrentX(newX);
    setCurrentY(newY);
  }, [x, y]);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    
    // Clear canvas
    ctx.clearRect(0, 0, CANVAS_SIZE, CANVAS_SIZE);
    
    // Draw outer circle (joystick base)
    ctx.beginPath();
    ctx.arc(CENTER_X, CENTER_Y, RADIUS, 0, 2 * Math.PI);
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.6)';
    ctx.lineWidth = 3;
    ctx.stroke();
    
    // Fill base with semi-transparent background
    ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
    ctx.fill();
    
    // Draw center cross
    ctx.beginPath();
    ctx.moveTo(CENTER_X - 10, CENTER_Y);
    ctx.lineTo(CENTER_X + 10, CENTER_Y);
    ctx.moveTo(CENTER_X, CENTER_Y - 10);
    ctx.lineTo(CENTER_X, CENTER_Y + 10);
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.4)';
    ctx.lineWidth = 2;
    ctx.stroke();
    
    // Draw handle (joystick knob)
    ctx.beginPath();
    ctx.arc(currentX, currentY, HANDLE_RADIUS, 0, 2 * Math.PI);
    ctx.fillStyle = '#4CAF50';
    ctx.fill();
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.8)';
    ctx.lineWidth = 2;
    ctx.stroke();
    
    // Draw inner highlight on handle
    ctx.beginPath();
    ctx.arc(currentX - 5, currentY - 5, HANDLE_RADIUS / 3, 0, 2 * Math.PI);
    ctx.fillStyle = 'rgba(255, 255, 255, 0.6)';
    ctx.fill();
    
  }, [currentX, currentY]);

  const getMousePos = (e) => {
    const canvas = canvasRef.current;
    const rect = canvas.getBoundingClientRect();
    return {
      x: e.clientX - rect.left,
      y: e.clientY - rect.top
    };
  };

  const getTouchPos = (e) => {
    const canvas = canvasRef.current;
    const rect = canvas.getBoundingClientRect();
    const touch = e.touches[0];
    return {
      x: touch.clientX - rect.left,
      y: touch.clientY - rect.top
    };
  };

  const updatePosition = (pos) => {
    const dx = pos.x - CENTER_X;
    const dy = pos.y - CENTER_Y;
    const distance = Math.sqrt(dx * dx + dy * dy);
    
    let newX, newY;
    
    if (distance <= RADIUS) {
      newX = pos.x;
      newY = pos.y;
    } else {
      // Constrain to circle boundary
      const angle = Math.atan2(dy, dx);
      newX = CENTER_X + Math.cos(angle) * RADIUS;
      newY = CENTER_Y + Math.sin(angle) * RADIUS;
    }
    
    setCurrentX(newX);
    setCurrentY(newY);
    
    // Convert to normalized coordinates (-100 to 100)
    const normalizedX = ((newX - CENTER_X) / RADIUS) * 100;
    const normalizedY = -((newY - CENTER_Y) / RADIUS) * 100; // Invert Y axis
    
    if (onJoystickChange) {
      onJoystickChange(Math.round(normalizedX), Math.round(normalizedY));
    }
  };

  const handleMouseDown = (e) => {
    const pos = getMousePos(e);
    const dx = pos.x - currentX;
    const dy = pos.y - currentY;
    const distance = Math.sqrt(dx * dx + dy * dy);
    
    if (distance <= HANDLE_RADIUS) {
      setDragging(true);
      e.preventDefault();
    }
  };

  const handleMouseMove = (e) => {
    if (dragging) {
      const pos = getMousePos(e);
      updatePosition(pos);
      e.preventDefault();
    }
  };

  const handleMouseUp = (e) => {
    if (dragging) {
      setDragging(false);
      // Return to center
      setCurrentX(CENTER_X);
      setCurrentY(CENTER_Y);
      if (onJoystickChange) {
        onJoystickChange(0, 0);
      }
      e.preventDefault();
    }
  };

  // Touch events
  const handleTouchStart = (e) => {
    const pos = getTouchPos(e);
    const dx = pos.x - currentX;
    const dy = pos.y - currentY;
    const distance = Math.sqrt(dx * dx + dy * dy);
    
    if (distance <= HANDLE_RADIUS) {
      setDragging(true);
      e.preventDefault();
    }
  };

  const handleTouchMove = (e) => {
    if (dragging) {
      const pos = getTouchPos(e);
      updatePosition(pos);
      e.preventDefault();
    }
  };

  const handleTouchEnd = (e) => {
    if (dragging) {
      setDragging(false);
      // Return to center
      setCurrentX(CENTER_X);
      setCurrentY(CENTER_Y);
      if (onJoystickChange) {
        onJoystickChange(0, 0);
      }
      e.preventDefault();
    }
  };

  return (
    <div className="joystick-container">
      <canvas
        ref={canvasRef}
        width={CANVAS_SIZE}
        height={CANVAS_SIZE}
        style={{
          border: '1px solid rgba(255, 255, 255, 0.3)',
          borderRadius: '10px',
          cursor: dragging ? 'grabbing' : 'grab',
          touchAction: 'none'
        }}
        onMouseDown={handleMouseDown}
        onMouseMove={handleMouseMove}
        onMouseUp={handleMouseUp}
        onMouseLeave={handleMouseUp}
        onTouchStart={handleTouchStart}
        onTouchMove={handleTouchMove}
        onTouchEnd={handleTouchEnd}
      />
      
      <div className="joystick-values">
        <div>X: <span style={{ fontWeight: 'bold', color: '#4CAF50' }}>{Math.round(x)}</span></div>
        <div>Y: <span style={{ fontWeight: 'bold', color: '#4CAF50' }}>{Math.round(y)}</span></div>
      </div>
    </div>
  );
};

export default JoystickController;