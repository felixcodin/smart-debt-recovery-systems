#!/bin/bash

# Smart Debt Recovery System - Start All Services
# Run this script to start all microservices

BUILD_DIR="../build/Release"

echo "=========================================="
echo "Smart Debt Recovery System - Starting All Services"
echo "=========================================="

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Please run 'cmake --build build' first."
    exit 1
fi

# Function to start a service in background
start_service() {
    local service_name=$1
    local service_path=$2
    local port=$3
    
    echo "Starting $service_name on port $port..."
    "$BUILD_DIR/$service_path" > "../logs/${service_name}.log" 2>&1 &
    local pid=$!
    echo "$pid" > "../logs/${service_name}.pid"
    echo "  ✓ $service_name started (PID: $pid)"
}

# Create logs directory if not exists
mkdir -p logs

# Start all services
echo ""
echo "Starting services..."
echo ""

start_service "api-gateway" "api-gateway/api-gateway" 8080
sleep 1

start_service "borrower-service" "borrower-service/borrower-service" 8081
sleep 1

start_service "risk-assessment-service" "risk-assessment-service/risk-assessment-service" 8082
sleep 1

start_service "recovery-strategy-service" "recovery-strategy-service/recovery-strategy-service" 8083
sleep 1

start_service "communication-service" "communication-service/communication-service" 8084
sleep 1

echo ""
echo "=========================================="
echo "All services started successfully!"
echo "=========================================="
echo ""
echo "Service URLs:"
echo "  API Gateway:              http://localhost:8080"
echo "  Borrower Service:         http://localhost:8081"
echo "  Risk Assessment Service:  http://localhost:8082"
echo "  Recovery Strategy Service: http://localhost:8083"
echo "  Communication Service:    http://localhost:8084"
echo ""
echo "Health checks:"
echo "  curl http://localhost:8080/health"
echo "  curl http://localhost:8081/health"
echo "  curl http://localhost:8082/health"
echo "  curl http://localhost:8083/health"
echo "  curl http://localhost:8084/health"
echo ""
echo "Logs are saved in: logs/"
echo "To stop all services, run: ./scripts/stop_all_services.sh"
echo "=========================================="
