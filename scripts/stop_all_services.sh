#!/bin/bash

# Smart Debt Recovery System - Stop All Services

echo "=========================================="
echo "Smart Debt Recovery System - Stopping All Services"
echo "=========================================="

LOGS_DIR="../logs"

if [ ! -d "$LOGS_DIR" ]; then
    echo "No services are running (logs directory not found)."
    exit 0
fi

# Function to stop a service
stop_service() {
    local service_name=$1
    local pid_file="$LOGS_DIR/${service_name}.pid"
    
    if [ -f "$pid_file" ]; then
        local pid=$(cat "$pid_file")
        if ps -p $pid > /dev/null 2>&1; then
            echo "Stopping $service_name (PID: $pid)..."
            kill $pid
            sleep 1
            if ps -p $pid > /dev/null 2>&1; then
                echo "  Force killing $service_name..."
                kill -9 $pid
            fi
            echo "  ✓ $service_name stopped"
        else
            echo "  ⚠ $service_name is not running (PID: $pid not found)"
        fi
        rm -f "$pid_file"
    else
        echo "  ⚠ $service_name PID file not found"
    fi
}

# Stop all services
echo ""
stop_service "api-gateway"
stop_service "borrower-service"
stop_service "risk-assessment-service"
stop_service "recovery-strategy-service"
stop_service "communication-service"

echo ""
echo "=========================================="
echo "All services stopped!"
echo "=========================================="
