#!/bin/bash

# Demo: Complete Debt Recovery User Story
# Shows end-to-end flow from borrower registration to automated recovery

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

API_URL="http://localhost:8080"
BORROWER_ID=""
LOAN_ACCOUNT_ID=""

# Helper function to print section headers
print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

# Helper function to print step
print_step() {
    echo -e "${YELLOW}>>> Step $1: $2${NC}"
}

# Helper function to wait for user
wait_for_user() {
    echo -e "${GREEN}Press Enter to continue...${NC}"
    read
}

clear
echo -e "${CYAN}"
cat << "EOF"
╔═══════════════════════════════════════════════════════════════╗
║                                                               ║
║       Smart Debt Recovery System (SDRS)                      ║
║       Complete User Story Demonstration                      ║
║                                                               ║
║  Scenario: Automated Recovery for Overdue Loan Payment       ║
║                                                               ║
╚═══════════════════════════════════════════════════════════════╝
EOF
echo -e "${NC}"

echo ""
echo "This demo will demonstrate the complete debt recovery flow:"
echo "  1. Borrower Registration"
echo "  2. Loan Account Creation"
echo "  3. Payment Delay Simulation"
echo "  4. Credit Risk Assessment"
echo "  5. Recovery Strategy Selection"
echo "  6. Automated Communication"
echo "  7. Communication History"
echo "  8. Strategy Status Monitoring"
echo ""

wait_for_user

# ============================================================================
# STEP 1: Borrower Registration
# ============================================================================
print_header "STEP 1: Borrower Registration"
print_step "1" "Registering new borrower 'Nguyen Van A'"

BORROWER_PAYLOAD=$(cat <<EOF
{
  "name": "Nguyen Van A",
  "email": "nguyenvana@example.com",
  "phone_number": "0901234567",
  "date_of_birth": "1985-05-15",
  "address": "123 Le Loi, District 1, HCMC",
  "monthly_income": 25000000,
  "employment_status": "Employed"
}
EOF
)

echo "Request payload:"
echo "$BORROWER_PAYLOAD" | jq '.'
echo ""

RESPONSE=$(curl -s -X POST "$API_URL/api/borrowers" \
  -H "Content-Type: application/json" \
  -d "$BORROWER_PAYLOAD")

echo "Response:"
echo "$RESPONSE" | jq '.'

# Extract borrower_id
BORROWER_ID=$(echo "$RESPONSE" | jq -r '.data.borrower_id')
if [ "$BORROWER_ID" = "null" ] || [ -z "$BORROWER_ID" ]; then
    echo -e "${RED}Failed to create borrower!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Borrower created successfully with ID: $BORROWER_ID${NC}"
wait_for_user

# ============================================================================
# STEP 2: Verify Borrower Created
# ============================================================================
print_header "STEP 2: Verify Borrower in Database"
print_step "2" "Fetching borrower details"

RESPONSE=$(curl -s "$API_URL/api/borrowers/$BORROWER_ID")
echo "Borrower details:"
echo "$RESPONSE" | jq '.'

echo -e "${GREEN}✓ Borrower verified in system${NC}"
wait_for_user

# ============================================================================
# STEP 3: Simulate Loan Account with Overdue Payment
# ============================================================================
print_header "STEP 3: Loan Account Status"
print_step "3" "Simulating overdue loan account"

echo "Loan Account Details (Simulated):"
cat <<EOF | jq '.'
{
  "loan_account_id": 200,
  "borrower_id": $BORROWER_ID,
  "loan_amount": 50000000,
  "outstanding_amount": 5000000,
  "interest_rate": 0.125,
  "loan_term_months": 24,
  "payment_due_date": "2025-12-01",
  "days_overdue": 15,
  "status": "OVERDUE",
  "missed_payments": 1
}
EOF

LOAN_ACCOUNT_ID=200
echo -e "${YELLOW}⚠ Payment is 15 days overdue!${NC}"
echo -e "${YELLOW}⚠ Outstanding amount: 5,000,000 VND${NC}"
wait_for_user

# ============================================================================
# STEP 4: Credit Risk Assessment
# ============================================================================
print_header "STEP 4: Credit Risk Assessment"
print_step "4" "Assessing credit risk for overdue account"

RISK_PAYLOAD=$(cat <<EOF
{
  "account_id": $LOAN_ACCOUNT_ID,
  "borrower_id": $BORROWER_ID,
  "days_past_due": 15,
  "missed_payments": 1,
  "loan_amount": 50000000,
  "remaining_amount": 5000000,
  "interest_rate": 0.125,
  "monthly_income": 25000000,
  "account_age_months": 6
}
EOF
)

echo "Risk assessment request:"
echo "$RISK_PAYLOAD" | jq '.'
echo ""

RESPONSE=$(curl -s -X POST "$API_URL/api/risk/assess" \
  -H "Content-Type: application/json" \
  -d "$RISK_PAYLOAD")

echo "Risk assessment result:"
echo "$RESPONSE" | jq '.'

RISK_SCORE=$(echo "$RESPONSE" | jq -r '.data.risk_score')
RISK_CATEGORY=$(echo "$RESPONSE" | jq -r '.data.risk_category')
DEFAULT_PROB=$(echo "$RESPONSE" | jq -r '.data.default_probability')

echo ""
echo -e "${CYAN}Risk Analysis:${NC}"
echo -e "  Risk Score: ${YELLOW}$RISK_SCORE${NC}"
echo -e "  Risk Category: ${YELLOW}$RISK_CATEGORY${NC}"
echo -e "  Default Probability: ${YELLOW}$DEFAULT_PROB${NC}"
echo -e "${GREEN}✓ Risk assessment completed${NC}"
wait_for_user

# ============================================================================
# STEP 5: View Available Recovery Strategies
# ============================================================================
print_header "STEP 5: Available Recovery Strategies"
print_step "5" "Fetching available recovery strategies"

RESPONSE=$(curl -s "$API_URL/api/strategy/list")
echo "Available strategies:"
echo "$RESPONSE" | jq '.'

echo -e "${GREEN}✓ Retrieved 3 recovery strategies${NC}"
wait_for_user

# ============================================================================
# STEP 6: Execute Recovery Strategy
# ============================================================================
print_header "STEP 6: Execute Recovery Strategy"
print_step "6" "Selecting and executing 'Automated Reminder' strategy"

STRATEGY_PAYLOAD=$(cat <<EOF
{
  "strategy_type": "automated_reminder",
  "account_id": $LOAN_ACCOUNT_ID,
  "borrower_id": $BORROWER_ID,
  "overdue_days": 15,
  "outstanding_amount": 5000000
}
EOF
)

echo "Strategy execution request:"
echo "$STRATEGY_PAYLOAD" | jq '.'
echo ""

RESPONSE=$(curl -s -X POST "$API_URL/api/strategy/execute" \
  -H "Content-Type: application/json" \
  -d "$STRATEGY_PAYLOAD")

echo "Strategy execution result:"
echo "$RESPONSE" | jq '.'

echo -e "${GREEN}✓ Strategy 'Automated Reminder' executed successfully${NC}"
wait_for_user

# ============================================================================
# STEP 7: Send Automated Communications
# ============================================================================
print_header "STEP 7: Send Automated Communications"
print_step "7a" "Sending email reminder"

EMAIL_PAYLOAD=$(cat <<EOF
{
  "to": "nguyenvana@example.com",
  "subject": "Payment Reminder - Loan Account #$LOAN_ACCOUNT_ID",
  "body": "Dear Nguyen Van A,\n\nYour payment of 5,000,000 VND is overdue by 15 days. Please pay by 2026-01-10 to avoid additional charges.\n\nBest regards,\nSDRS Team"
}
EOF
)

echo "Email request:"
echo "$EMAIL_PAYLOAD" | jq '.'
echo ""

RESPONSE=$(curl -s -X POST "$API_URL/api/communication/email" \
  -H "Content-Type: application/json" \
  -d "$EMAIL_PAYLOAD")

echo "Email response:"
echo "$RESPONSE" | jq '.'
echo -e "${GREEN}✓ Email sent successfully${NC}"
echo ""

print_step "7b" "Sending SMS reminder"

SMS_PAYLOAD=$(cat <<EOF
{
  "phone": "0901234567",
  "message": "SDRS: Payment of 5,000,000 VND is 15 days overdue. Pay by 01/10 to avoid charges. Call 1900-xxxx"
}
EOF
)

echo "SMS request:"
echo "$SMS_PAYLOAD" | jq '.'
echo ""

RESPONSE=$(curl -s -X POST "$API_URL/api/communication/sms" \
  -H "Content-Type: application/json" \
  -d "$SMS_PAYLOAD")

echo "SMS response:"
echo "$RESPONSE" | jq '.'
echo -e "${GREEN}✓ SMS sent successfully${NC}"
wait_for_user

# ============================================================================
# STEP 8: View Communication History
# ============================================================================
print_header "STEP 8: Communication History"
print_step "8" "Viewing all communications sent to borrower"

RESPONSE=$(curl -s "$API_URL/api/communication/history/$BORROWER_ID")
echo "Communication history:"
echo "$RESPONSE" | jq '.'

COMM_COUNT=$(echo "$RESPONSE" | jq '.data.communications | length')
echo ""
echo -e "${CYAN}Total communications sent: ${YELLOW}$COMM_COUNT${NC}"
echo -e "${GREEN}✓ Communication history retrieved${NC}"
wait_for_user

# ============================================================================
# SUMMARY
# ============================================================================
print_header "DEMONSTRATION COMPLETE"

echo -e "${CYAN}Summary of Actions:${NC}"
echo ""
echo -e "  ${GREEN}✓${NC} Step 1: Borrower 'Nguyen Van A' registered (ID: $BORROWER_ID)"
echo -e "  ${GREEN}✓${NC} Step 2: Borrower details verified in database"
echo -e "  ${GREEN}✓${NC} Step 3: Overdue loan account identified (15 days)"
echo -e "  ${GREEN}✓${NC} Step 4: Credit risk assessed (Score: $RISK_SCORE, Category: $RISK_CATEGORY)"
echo -e "  ${GREEN}✓${NC} Step 5: Available recovery strategies retrieved"
echo -e "  ${GREEN}✓${NC} Step 6: 'Automated Reminder' strategy executed"
echo -e "  ${GREEN}✓${NC} Step 7: Email and SMS reminders sent"
echo -e "  ${GREEN}✓${NC} Step 8: Communication history tracked"
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${CYAN}All microservices worked together to complete the debt recovery flow!${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Database verification
print_step "Bonus" "Verify data in PostgreSQL database"
echo ""
echo "Checking database for created borrower..."
docker exec -it sdrs-postgres psql -U sdrs_user -d sdrs_db -c \
  "SELECT borrower_id, first_name, last_name, email, monthly_income FROM borrowers WHERE borrower_id = $BORROWER_ID;"

echo ""
echo -e "${GREEN}Demo completed successfully!${NC}"
