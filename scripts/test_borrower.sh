#!/bin/bash

# Test borrower service directly
curl -X POST http://localhost:8081/borrowers \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Test User",
    "email": "test@example.com",
    "phone_number": "0901234567",
    "address": "Test Address"
  }'

echo ""
echo "---"

# Test get all borrowers
curl http://localhost:8081/borrowers

echo ""
