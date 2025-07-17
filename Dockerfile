# Multi-stage Dockerfile for MOOver MELKENS Web Simulator
# Stage 1: Build React frontend
FROM node:18-alpine AS frontend-builder

WORKDIR /app/frontend

# Copy package files first for better caching
COPY frontend/package*.json ./

# Install dependencies with cache mount
RUN --mount=type=cache,target=/root/.npm \
    npm ci --omit=dev

# Copy source code
COPY frontend/ ./

# Build the application
RUN npm run build

# Stage 2: Setup backend with frontend
FROM node:18-alpine AS production

# Add labels for Railway
LABEL railway.project="moover-simulator"
LABEL railway.service="web-simulator"

WORKDIR /app

# Install backend dependencies
COPY backend/package*.json ./backend/
WORKDIR /app/backend

# Install with cache mount
RUN --mount=type=cache,target=/root/.npm \
    npm ci --omit=dev

# Copy backend source
COPY backend/ ./

# Copy built frontend to correct location for backend
COPY --from=frontend-builder /app/frontend/build ../frontend/build

# Create necessary directories and set permissions
RUN mkdir -p uploads logs && \
    chown -R node:node /app

# Switch to non-root user
USER node

# Expose port (Railway will override this)
EXPOSE 3001

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
  CMD node -e "http.get('http://localhost:3001/api/health', (res) => { process.exit(res.statusCode === 200 ? 0 : 1) })" || exit 1

# Start the application
CMD ["npm", "start"]
