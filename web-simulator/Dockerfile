# Multi-stage Dockerfile for MELKENS HIL Simulator
# Stage 1: Build frontend (prepare static files)
FROM node:18-alpine AS frontend-builder

WORKDIR /app/frontend
COPY frontend/ .

# Create a simple build process for static frontend
RUN mkdir -p build && \
    cp index.html build/ && \
    cp style.css build/ && \
    cp main.js build/

# Stage 2: Python backend with static frontend serving
FROM python:3.11-slim

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    gcc \
    g++ \
    make \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Copy Python requirements and install dependencies
COPY backend/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy backend code
COPY backend/ ./backend/

# Copy built frontend from previous stage
COPY --from=frontend-builder /app/frontend/build ./frontend/

# Create logs directory
RUN mkdir -p logs logs/scenarios

# Set environment variables
ENV PYTHONPATH=/app/backend
ENV PYTHONUNBUFFERED=1
ENV PORT=3000

# Expose port
EXPOSE $PORT

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:3000/api/health || exit 1

# Copy and setup startup script
COPY start.sh /app/start.sh
RUN chmod +x /app/start.sh

# Run the application
CMD ["/app/start.sh"]