#!/bin/bash

# Demo: Complete Debt Recovery User Story - Simple Version
# Shows end-to-end flow without requiring jq

set -e

API_URL="http://localhost:8080"

echo "========================================="
echo "Smart Debt Recovery System (SDRS)"
echo "Complete User Story Demonstration"
echo "========================================="
echo ""
echo "Scenario: Automated Recovery for Overdue Loan Payment"
echo ""
sleep 2

# STEP 1: Borrower Registration
echo ""
echo "========================================="
echo "STEP 1: Borrower Registration"
echo "========================================="
echo ""
echo ">>> Creating borrower 'Nguyen Van A'..."

RESPONSE=$(curl -s -X POST "$API_URL/api/borrowers" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Nguyen Van A",
    "email": "nguyenvana@example.com",
    "phone_number": "0901234567",
    "date_of_birth": "1985-05-15",
    "address": "123 Le Loi, District 1, HCMC",
    "monthly_income": 25000000,
    "employment_status": "Employed"
  }')

echo "Response: $RESPONSE"
echo ""

# Extract borrower_id using grep/sed
BORROWER_ID=$(echo "$RESPONSE" | grep -o '"borrower_id":[0-9]*' | grep -o '[0-9]*')
if [ -z "$BORROWER_ID" ]; then
    echo "ERROR: Failed to create borrower!"
    exit 1
fi

echo "✓ Borrower created successfully with ID: $BORROWER_ID"
sleep 2

# STEP 2: Verify Borrower
echo ""
echo "========================================="
echo "STEP 2: Verify Borrower in Database"
echo "========================================="
echo ""

RESPONSE=$(curl -s "$API_URL/api/borrowers/$BORROWER_ID")
echo "Borrower details: $RESPONSE"
echo ""
echo "✓ Borrower verified in system"
sleep 2

# STEP 3: Loan Account Status
echo ""
echo "========================================="
echo "STEP 3: Loan Account Status"
echo "========================================="
echo ""
echo "Simulating overdue loan account:"
echo "  - Loan Account ID: 200"
echo "  - Outstanding Amount: 5,000,000 VND"
echo "  - Days Overdue: 15"
echo "  - Status: OVERDUE"
echo ""
echo "⚠ Payment is 15 days overdue!"
sleep 2

# STEP 4: Risk Assessment
echo ""
echo "========================================="
echo "STEP 4: Credit Risk Assessment"
echo "========================================="
echo ""
echo ">>> Assessing credit risk..."

RESPONSE=$(curl -s -X POST "$API_URL/api/risk/assess" \
  -H "Content-Type: application/json" \
  -d '{
    "account_id": 200,
    "borrower_id": '"$BORROWER_ID"',
    "days_past_due": 15,
    "missed_payments": 1,
    "loan_amount": 50000000,
    "remaining_amount": 5000000,
    "interest_rate": 0.125,
    "monthly_income": 25000000,
    "account_age_months": 6
  }')

echo "Risk assessment: $RESPONSE"
echo ""
echo "✓ Risk assessment completed"
sleep 2

# STEP 5: Available Strategies
echo ""
echo "========================================="
echo "STEP 5: Available Recovery Strategies"
echo "========================================="
echo ""

RESPONSE=$(curl -s "$API_URL/api/strategy/list")
echo "Available strategies: $RESPONSE"
echo ""
echo "✓ Retrieved recovery strategies"
sleep 2

# STEP 6: Execute Strategy
echo ""
echo "========================================="
echo "STEP 6: Execute Recovery Strategy"
echo "========================================="
echo ""
echo ">>> Executing 'Automated Reminder' strategy..."

RESPONSE=$(curl -s -X POST "$API_URL/api/strategy/execute" \
  -H "Content-Type: application/json" \
  -d '{
    "strategy_type": "automated_reminder",
    "account_id": 200,
    "borrower_id": '"$BORROWER_ID"',
    "overdue_days": 15,
    "outstanding_amount": 5000000
  }')

echo "Strategy result: $RESPONSE"
echo ""
echo "✓ Strategy executed successfully"
sleep 2

# STEP 7: Send Communications
echo ""
echo "========================================="
echo "STEP 7: Send Automated Communications"
echo "========================================="
echo ""
echo ">>> Sending email reminder..."

RESPONSE=$(curl -s -X POST "$API_URL/api/communication/email" \
  -H "Content-Type: application/json" \
  -d '{
    "to": "nguyenvana@example.com",
    "subject": "Payment Reminder - Loan Account #200",
    "body": "Dear Nguyen Van A, your payment of 5,000,000 VND is overdue by 15 days."
  }')

echo "Email response: $RESPONSE"
echo "✓ Email sent"
echo ""

echo ">>> Sending SMS reminder..."

RESPONSE=$(curl -s -X POST "$API_URL/api/communication/sms" \
  -H "Content-Type: application/json" \
  -d '{
    "phone": "0901234567",
    "message": "SDRS: Payment of 5,000,000 VND is 15 days overdue."
  }')

echo "SMS response: $RESPONSE"
echo "✓ SMS sent"
sleep 2

# STEP 8: Communication History
echo ""
echo "========================================="
echo "STEP 8: Communication History"
echo "========================================="
echo ""

RESPONSE=$(curl -s "$API_URL/api/communication/history/$BORROWER_ID")
echo "Communication history: $RESPONSE"
echo ""
echo "✓ Communication history retrieved"
sleep 2

# SUMMARY
echo ""
echo "========================================="
echo "DEMONSTRATION COMPLETE"
echo "========================================="
echo ""
echo "Summary of Actions:"
echo "  ✓ Step 1: Borrower registered (ID: $BORROWER_ID)"
echo "  ✓ Step 2: Borrower verified in database"
echo "  ✓ Step 3: Overdue loan identified (15 days)"
echo "  ✓ Step 4: Credit risk assessed"
echo "  ✓ Step 5: Recovery strategies retrieved"
echo "  ✓ Step 6: Automated Reminder strategy executed"
echo "  ✓ Step 7: Email and SMS sent"
echo "  ✓ Step 8: Communication history tracked"
echo ""
echo "All microservices worked together successfully!"
echo ""

# Database verification
echo "========================================="
echo "Database Verification"
echo "========================================="
echo ""
docker exec sdrs-postgres psql -U sdrs_user -d sdrs_db -c \
  "SELECT borrower_id, first_name, last_name, email FROM borrowers WHERE borrower_id = $BORROWER_ID;"

echo ""
echo "✓ Demo completed successfully!"
