# Smart Debt Recovery System - Automated Demo Script
# Bash version for WSL/Linux

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
GRAY='\033[0;37m'
NC='\033[0m' # No Color

API_GATEWAY="http://localhost:8080"
TEST_PASSED=0
TEST_FAILED=0

echo -e "${CYAN}"
echo "========================================="
echo "  SMART DEBT RECOVERY SYSTEM - DEMO"
echo "  Microservices Architecture with Docker"
echo "========================================="
echo -e "${NC}"

function test_endpoint() {
    local name="$1"
    local method="$2"
    local url="$3"
    local body="$4"
    local expected_status="${5:-200}"
    
    echo -e "\n${YELLOW}[TEST] $name${NC}"
    echo -e "${GRAY}  → $method $url${NC}"
    
    if [ "$method" = "GET" ]; then
        response=$(curl -s -w "\n%{http_code}" "$url")
    else
        response=$(curl -s -w "\n%{http_code}" -X POST "$url" \
            -H "Content-Type: application/json" \
            -d "$body")
    fi
    
    status_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | sed '$d')
    
    if [ "$status_code" -eq "$expected_status" ]; then
        echo -e "${GREEN}   PASSED (HTTP $status_code)${NC}"
        ((TEST_PASSED++))
        echo -e "${GRAY}  Response:${NC}"
        echo "$body" | jq -C '.' 2>/dev/null || echo "$body" | sed 's/^/    /'
        return 0
    else
        echo -e "${RED}   FAILED (Expected $expected_status, got $status_code)${NC}"
        ((TEST_FAILED++))
        return 1
    fi
}

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo -e "${YELLOW}[WARNING] jq not installed, JSON output will be unformatted${NC}"
fi

# Wait for services
echo -e "\n${CYAN}[SETUP] Checking if services are running...${NC}"
sleep 2

# ========================================
# PHASE 1: Health Checks
# ========================================
echo -e "\n${CYAN}=========================================${NC}"
echo -e "${CYAN}  PHASE 1: HEALTH CHECKS${NC}"
echo -e "${CYAN}=========================================${NC}"

test_endpoint "API Gateway Health" "GET" "$API_GATEWAY/health"
test_endpoint "API Gateway Info" "GET" "$API_GATEWAY/"

# ========================================
# PHASE 2: Borrower Service
# ========================================
echo -e "\n${CYAN}=========================================${NC}"
echo -e "${CYAN}  PHASE 2: BORROWER SERVICE (Mock)${NC}"
echo -e "${CYAN}=========================================${NC}"

test_endpoint "Create New Borrower" "POST" "$API_GATEWAY/api/borrowers" '{
    "name": "Nguyen Van Demo",
    "email": "demo@example.com",
    "phone_number": "0901234567",
    "address": "123 Demo Street, Hanoi"
}'

test_endpoint "Get Borrower by ID" "GET" "$API_GATEWAY/api/borrowers/1"
test_endpoint "Get All Borrowers" "GET" "$API_GATEWAY/api/borrowers"

# ========================================
# PHASE 3: Risk Assessment Service
# ========================================
echo -e "\n${CYAN}=========================================${NC}"
echo -e "${CYAN}  PHASE 3: RISK ASSESSMENT (Mock)${NC}"
echo -e "${CYAN}=========================================${NC}"

test_endpoint "Assess Credit Risk" "POST" "$API_GATEWAY/api/risk/assess" '{
    "account_id": 1,
    "borrower_id": 1,
    "days_past_due": 15,
    "missed_payments": 2,
    "loan_amount": 50000000.0,
    "remaining_amount": 45000000.0,
    "interest_rate": 0.125,
    "monthly_income": 20000000.0,
    "account_age_months": 12
}'

echo -e "${GRAY}  Note: Other risk endpoints not yet implemented in mock mode${NC}"

# ========================================
# PHASE 4: Recovery Strategy Service
# ========================================
echo -e "\n${CYAN}=========================================${NC}"
echo -e "${CYAN}  PHASE 4: RECOVERY STRATEGY (Mock)${NC}"
echo -e "${CYAN}=========================================${NC}"

test_endpoint "Execute Recovery Strategy" "POST" "$API_GATEWAY/api/strategy/execute" '{
    "borrower_id": 1,
    "account_id": 1,
    "strategy_type": "automated_reminder",
    "overdue_days": 15,
    "outstanding_amount": 5000000
}'

echo -e "${GRAY}  Note: Other strategy endpoints not yet implemented in mock mode${NC}"

# ========================================
# PHASE 5: Communication Service
# ========================================
echo -e "\n${CYAN}=========================================${NC}"
echo -e "${CYAN}  PHASE 5: COMMUNICATION SERVICE (Mock)${NC}"
echo -e "${CYAN}=========================================${NC}"

test_endpoint "Send Email Notification" "POST" "$API_GATEWAY/api/communication/email" '{
    "to": "demo@example.com",
    "subject": "Payment Reminder",
    "body": "Dear Nguyen Van Demo, your payment of 5,000,000 VND is due on 2026-01-15"
}'

echo -e "${GRAY}  Note: SMS and communication history endpoints not yet implemented in mock mode${NC}"

# ========================================
# PHASE 6: Middleware Features
# ========================================
echo -e "\n${CYAN}=========================================${NC}"
echo -e "${CYAN}  PHASE 6: MIDDLEWARE FEATURES${NC}"
echo -e "${CYAN}=========================================${NC}"

echo -e "\n${YELLOW}[TEST] Rate Limiting (100 requests/minute)${NC}"
echo -e "${GRAY}  → Sending 105 rapid requests...${NC}"

rate_limit_hit=false
for i in {1..105}; do
    status=$(curl -s -o /dev/null -w "%{http_code}" "$API_GATEWAY/health")
    if [ "$status" -eq 429 ]; then
        echo -e "${GREEN}   RATE LIMIT TRIGGERED at request $i (HTTP 429)${NC}"
        ((TEST_PASSED++))
        rate_limit_hit=true
        break
    fi
    if [ $((i % 20)) -eq 0 ]; then
        echo -e "${GRAY}    Request $i: HTTP $status${NC}"
    fi
done

if [ "$rate_limit_hit" = false ]; then
    echo -e "${YELLOW}   Rate limit not triggered (may need more requests)${NC}"
fi

echo -e "\n${YELLOW}[TEST] CORS Headers${NC}"
cors_headers=$(curl -s -I "$API_GATEWAY/health" | grep -i "access-control")
if [ -n "$cors_headers" ]; then
    echo -e "${GREEN}   CORS Headers present${NC}"
    echo "$cors_headers" | sed 's/^/    /'
    ((TEST_PASSED++))
else
    echo -e "${RED}   CORS Headers missing${NC}"
    ((TEST_FAILED++))
fi

# ========================================
# FINAL REPORT
# ========================================
echo -e "\n${CYAN}=========================================${NC}"
echo -e "${CYAN}  DEMO COMPLETE - FINAL REPORT${NC}"
echo -e "${CYAN}=========================================${NC}"

TOTAL_TESTS=$((TEST_PASSED + TEST_FAILED))
PASS_RATE=$(echo "scale=1; ($TEST_PASSED * 100) / $TOTAL_TESTS" | bc)

echo -e "\nTest Results:"
echo -e "${GREEN}   Passed: $TEST_PASSED${NC}"
echo -e "${RED}   Failed: $TEST_FAILED${NC}"
echo -e "  Total:    $TOTAL_TESTS"
echo -e "  Success Rate: $PASS_RATE%"

echo -e "\nArchitecture Highlights:"
echo -e "${GRAY}  • 6 Microservices (API Gateway + 5 backend)${NC}"
echo -e "${GRAY}  • Docker Containerization with health checks${NC}"
echo -e "${GRAY}  • Middleware: CORS, Rate Limiting, Logging, Auth${NC}"
echo -e "${GRAY}  • Circuit Breaker Pattern for resilience${NC}"
echo -e "${GRAY}  • PostgreSQL database with init scripts${NC}"
echo -e "${GRAY}  • C++23 with modern async patterns${NC}"

echo -e "\nServices Status:"
echo -e "${GRAY}  → API Gateway:       http://localhost:8080${NC}"
echo -e "${GRAY}  → Borrower Service:  http://localhost:8081${NC}"
echo -e "${GRAY}  → Risk Assessment:   http://localhost:8082${NC}"
echo -e "${GRAY}  → Recovery Strategy: http://localhost:8083${NC}"
echo -e "${GRAY}  → Communication:     http://localhost:8084${NC}"
echo -e "${GRAY}  → PostgreSQL:        localhost:5432${NC}"

echo -e "\n${CYAN}=========================================${NC}"
echo -e "${GREEN}  Demo script completed successfully!${NC}"
echo -e "${CYAN}=========================================${NC}"
echo ""
