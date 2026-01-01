#!/bin/bash

# ========================================
# CÁCH CHẠY ĐƠN GIẢN NHẤT
# ========================================

echo "Smart Debt Recovery System - Quick Start"
echo "=========================================="
echo ""

cd /mnt/d/smart-debt-recovery-systems

echo "Bạn cần mở 4 terminal WSL riêng biệt:"
echo ""
echo "Terminal 1 (Borrower Service - Port 8081):"
echo "  cd /mnt/d/smart-debt-recovery-systems"
echo "  ./build/Release/build/Release/borrower-service/borrower-service"
echo ""
echo "Terminal 2 (Risk Assessment - Port 8082):"
echo "  cd /mnt/d/smart-debt-recovery-systems"
echo "  ./build/Release/build/Release/risk-assessment-service/risk-assessment-service"
echo ""
echo "Terminal 3 (Recovery Strategy - Port 8083):"
echo "  cd /mnt/d/smart-debt-recovery-systems"
echo "  ./build/Release/build/Release/recovery-strategy-service/recovery-strategy-service"
echo ""
echo "Terminal 4 (Communication - Port 8084):"
echo "  cd /mnt/d/smart-debt-recovery-systems"
echo "  ./build/Release/build/Release/communication-service/communication-service"
echo ""
echo "=========================================="
echo "Sau khi chạy, test bằng trình duyệt:"
echo "   http://localhost:8081/borrowers"
echo ""
echo "Hoặc dùng curl trong terminal mới:"
echo "   curl http://localhost:8081/borrowers"
echo ""
echo "Nhấn Ctrl+C trong mỗi terminal để tắt service"
echo "=========================================="
