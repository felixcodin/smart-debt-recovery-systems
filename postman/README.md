# Postman Collection for Smart Debt Recovery System

## Collection File

- **File:** `SDRS_API_Collection.postman_collection.json`
- **Services:** 4 microservices
- **Total Requests:** 20+ API endpoints

---

## Usage Guide

### 1. Import into Postman

1. Open **Postman**
2. Click **Import** (top left corner)
3. Select file: `SDRS_API_Collection.postman_collection.json`
4. Click **Import**

### 2. Start Services

Before testing, ensure all services are running:

```bash
# Terminal 1
./scripts/run_service.sh borrower

# Terminal 2
./scripts/run_service.sh risk

# Terminal 3
./scripts/run_service.sh strategy

# Terminal 4
./scripts/run_service.sh communication
```

### 3. Test APIs

In Postman, you will see 4 folders with the following endpoints:

---

## Collection Structure

### Borrower Service (Port 8081)
- Get All Borrowers
- Get Borrower by ID
- Create Borrower
- Update Borrower
- Delete Borrower
- Get All Loans
- Get Delinquent Loans
- Record Payment
- Get All Payments

### Risk Assessment Service (Port 8082)
- Health Check
- Calculate Risk Score

### Recovery Strategy Service (Port 8083)
- Health Check
- Get Strategy for Loan
- Execute Strategy

### Communication Service (Port 8084)
- Health Check
- Send Email
- Send SMS

---

## Testing Workflows

### Scenario 1: Borrower Management
1. **Create Borrower** - Create a new borrower
2. **Get All Borrowers** - View borrower list
3. **Get Borrower by ID** - Get detailed information
4. **Update Borrower** - Update borrower information
5. **Delete Borrower** - Delete borrower (if needed)

### Scenario 2: Loan Management
1. **Get All Loans** - View all loans
2. **Get Delinquent Loans** - Filter overdue loans
3. **Record Payment** - Record a payment
4. **Get All Payments** - View payment history

### Scenario 3: Risk Assessment
1. **Calculate Risk Score** - Calculate risk score for a borrower

### Scenario 4: Debt Recovery
1. **Get Strategy for Loan** - Get appropriate recovery strategy
2. **Execute Strategy** - Execute the strategy
3. **Send Email/SMS** - Send reminder notifications

---

## Sample Request Data

### Create Borrower
```json
{
  "firstName": "John",
  "lastName": "Doe",
  "email": "john.doe@example.com",
  "phoneNumber": "1234567890",
  "monthlyIncome": 5000.00
}
```

### Record Payment
```json
{
  "amount": 1000.00,
  "paymentMethod": "BankTransfer"
}
```

### Calculate Risk Score
```json
{
  "borrowerId": 1,
  "loanAmount": 50000.00,
  "monthlyIncome": 5000.00,
  "daysPastDue": 30,
  "missedPayments": 2
}
```

### Send Email
```json
{
  "borrowerId": 1,
  "channel": "Email",
  "recipient": "john.doe@example.com",
  "subject": "Payment Reminder",
  "message": "Your payment is due soon"
}
```

---

## Tips and Notes

1. **Health Check First:** Always test `/health` endpoints first to ensure services are running
2. **Mock Mode:** Services run in mock mode by default (no database required)
3. **Database Mode:** Set `SDRS_USE_DATABASE=1` environment variable to use PostgreSQL
4. **HTTP Response Codes:**
   - `200 OK` - Successful request
   - `201 Created` - Resource created successfully
   - `404 Not Found` - Resource not found
   - `500 Internal Server Error` - Server error

---

## Additional Documentation

For more information, see:
- [HUONG_DAN_CHAY.md](../HUONG_DAN_CHAY.md) - Service startup guide (Vietnamese)
- [README.md](../README.md) - Project overview
- [DATABASE_INTEGRATION.md](../docs/DATABASE_INTEGRATION.md) - Database setup guide

---

**Happy Testing!**
