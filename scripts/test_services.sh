#!/bin/bash

# Smart Debt Recovery System - Test All Services
# Run this script to test all microservices endpoints

echo "=========================================="
echo "Smart Debt Recovery System - Testing Services"
echo "=========================================="

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to test endpoint
test_endpoint() {
    local name=$1
    local url=$2
    local method=${3:-GET}
    local data=${4:-}
    
    echo -e "\n${BLUE}Testing: $name${NC}"
    echo "  URL: $url"
    
    if [ "$method" = "POST" ]; then
        response=$(curl -s -w "\n%{http_code}" -X POST -H "Content-Type: application/json" -d "$data" "$url")
    else
        response=$(curl -s -w "\n%{http_code}" "$url")
    fi
    
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    
    if [ "$http_code" -eq 200 ] || [ "$http_code" -eq 201 ]; then
        echo -e "  ${GREEN}✓ SUCCESS${NC} (HTTP $http_code)"
        echo "  Response: $body" | head -c 200
        [ ${#body} -gt 200 ] && echo "..."
    else
        echo -e "  ${RED}✗ FAILED${NC} (HTTP $http_code)"
        echo "  Response: $body"
    fi
}

echo ""
echo "1. Testing Health Endpoints"
echo "----------------------------"
test_endpoint "API Gateway Health" "http://localhost:8080/health"
test_endpoint "Borrower Service Health" "http://localhost:8081/health"
test_endpoint "Risk Assessment Health" "http://localhost:8082/health"
test_endpoint "Recovery Strategy Health" "http://localhost:8083/health"
test_endpoint "Communication Service Health" "http://localhost:8084/health"

echo ""
echo "2. Testing API Gateway Root"
echo "----------------------------"
test_endpoint "API Gateway Info" "http://localhost:8080/"

echo ""
echo "3. Testing Borrower Service"
echo "----------------------------"
test_endpoint "Get All Borrowers" "http://localhost:8081/borrowers"

# Create a borrower
borrower_data='{
  "first_name": "John",
  "last_name": "Doe",
  "email": "john.doe@example.com",
  "phone": "+1234567890",
  "monthly_income": 5000.00,
  "employment_status": "FullTime"
}'
test_endpoint "Create Borrower" "http://localhost:8081/borrowers" "POST" "$borrower_data"

echo ""
echo "4. Testing Risk Assessment"
echo "----------------------------"
risk_data='{
  "account_id": 1,
  "borrower_id": 1,
  "days_past_due": 45,
  "missed_payments": 2,
  "loan_amount": 10000.0,
  "remaining_amount": 8000.0,
  "interest_rate": 0.05,
  "monthly_income": 5000.0,
  "account_age_months": 12
}'
test_endpoint "Assess Risk" "http://localhost:8082/assess-risk" "POST" "$risk_data"

echo ""
echo "5. Testing Recovery Strategy"
echo "----------------------------"
strategy_data='{
  "strategy_type": "AutomatedReminder",
  "account_id": 1
}'
test_endpoint "Execute Strategy" "http://localhost:8083/execute-strategy" "POST" "$strategy_data"

echo ""
echo "6. Testing Communication Service"
echo "----------------------------"
email_data='{
  "to": "borrower@example.com",
  "subject": "Payment Reminder",
  "body": "Your payment is overdue."
}'
test_endpoint "Send Email" "http://localhost:8084/send-email" "POST" "$email_data"

sms_data='{
  "phone": "+1234567890",
  "message": "Payment reminder: Your payment is overdue."
}'
test_endpoint "Send SMS" "http://localhost:8084/send-sms" "POST" "$sms_data"

echo ""
echo "7. Testing API Gateway Routing"
echo "----------------------------"
test_endpoint "Gateway -> Borrowers" "http://localhost:8080/api/borrowers"
test_endpoint "Gateway -> Risk Assessment" "http://localhost:8080/api/risk/assess" "POST" "$risk_data"
test_endpoint "Gateway -> Strategy" "http://localhost:8080/api/strategy/execute" "POST" "$strategy_data"
test_endpoint "Gateway -> Email" "http://localhost:8080/api/communication/email" "POST" "$email_data"

echo ""
echo "8. Testing Service-to-Service Communication"
echo "----------------------------"
assess_via_borrower='{
  "account_id": 1,
  "days_past_due": 60,
  "missed_payments": 3,
  "loan_amount": 15000.0,
  "remaining_amount": 12000.0,
  "interest_rate": 0.06,
  "monthly_income": 4500.0,
  "account_age_months": 18
}'
test_endpoint "Borrower->Risk Service" "http://localhost:8081/borrowers/1/assess-risk" "POST" "$assess_via_borrower"

echo ""
echo "=========================================="
echo "Testing completed!"
echo "=========================================="
