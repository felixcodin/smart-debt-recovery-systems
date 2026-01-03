#!/bin/bash
curl -X POST http://localhost:8082/assess-risk \
  -H 'Content-Type: application/json' \
  -d '{
    "account_id": 1,
    "borrower_id": 1,
    "days_past_due": 15,
    "missed_payments": 2,
    "loan_amount": 50000000.0,
    "remaining_amount": 45000000.0,
    "interest_rate": 12.5,
    "monthly_income": 20000000.0,
    "account_age_months": 12
  }'
