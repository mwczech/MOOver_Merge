# Multi-stage build for MELKENS WB Simulator

# Stage 1: Build shared library
FROM node:18-alpine AS shared-builder
WORKDIR /app/shared
COPY web-simulator/shared/package*.json ./
RUN npm ci --only=production
COPY web-simulator/shared/ ./
RUN npm run build

# Stage 2: Build backend
FROM node:18-alpine AS backend-builder
WORKDIR /app/backend
COPY web-simulator/backend/package*.json ./
RUN npm ci --only=production

# Copy shared library
COPY --from=shared-builder /app/shared/dist /app/shared/dist
COPY --from=shared-builder /app/shared/package.json /app/shared/package.json

COPY web-simulator/backend/ ./
RUN npm run build

# Stage 3: Build frontend
FROM node:18-alpine AS frontend-builder
WORKDIR /app/frontend

# Copy shared library
COPY --from=shared-builder /app/shared/dist /app/shared/dist
COPY --from=shared-builder /app/shared/package.json /app/shared/package.json

COPY web-simulator/frontend/package*.json ./
RUN npm ci --only=production
COPY web-simulator/frontend/ ./
RUN npm run build

# Stage 4: Production image
FROM node:18-alpine AS production

WORKDIR /app

# Install dumb-init for proper signal handling
RUN apk add --no-cache dumb-init

# Create non-root user
RUN addgroup -g 1001 -S nodejs
RUN adduser -S nextjs -u 1001

# Copy built backend
COPY --from=backend-builder /app/backend/dist ./backend/dist
COPY --from=backend-builder /app/backend/package.json ./backend/
COPY --from=backend-builder /app/backend/node_modules ./backend/node_modules

# Copy built frontend
COPY --from=frontend-builder /app/frontend/dist ./frontend/dist

# Copy shared library
COPY --from=shared-builder /app/shared ./shared

# Create startup script
COPY web-simulator/scripts/start.sh ./start.sh
RUN chmod +x ./start.sh

# Change ownership
RUN chown -R nextjs:nodejs /app
USER nextjs

# Expose ports
EXPOSE 3001 3002

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
  CMD wget --no-verbose --tries=1 --spider http://localhost:3001/health || exit 1

# Use dumb-init to handle signals properly
ENTRYPOINT ["dumb-init", "--"]
CMD ["./start.sh"]