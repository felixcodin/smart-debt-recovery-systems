#!/bin/bash

# Script đơn giản để chạy một service
# Usage: ./run_service.sh borrower

SERVICE=$1
BASE_DIR="/mnt/d/smart-debt-recovery-systems"
BUILD_DIR="$BASE_DIR/build/Release/build/Release"

if [ -z "$SERVICE" ]; then
    echo "Usage: $0 <service-name>"
    echo ""
    echo "Available services:"
    echo "  borrower        - Borrower Service (port 8081)"
    echo "  risk            - Risk Assessment Service (port 8082)"
    echo "  strategy        - Recovery Strategy Service (port 8083)"
    echo "  communication   - Communication Service (port 8084)"
    echo ""
    echo "Example: $0 borrower"
    exit 1
fi

case $SERVICE in
    borrower)
        echo "Starting Borrower Service on port 8081..."
        cd "$BASE_DIR"
        exec "$BUILD_DIR/borrower-service/borrower-service"
        ;;
    risk)
        echo "Starting Risk Assessment Service on port 8082..."
        cd "$BASE_DIR"
        exec "$BUILD_DIR/risk-assessment-service/risk-assessment-service"
        ;;
    strategy)
        echo "Starting Recovery Strategy Service on port 8083..."
        cd "$BASE_DIR"
        exec "$BUILD_DIR/recovery-strategy-service/recovery-strategy-service"
        ;;
    communication)
        echo "Starting Communication Service on port 8084..."
        cd "$BASE_DIR"
        exec "$BUILD_DIR/communication-service/communication-service"
        ;;
    *)
        echo "Error: Unknown service '$SERVICE'"
        echo "Available: borrower, risk, strategy, communication"
        exit 1
        ;;
esac
