# Smart Debt Recovery System (SDRS)

A microservices-based debt recovery management system built with C++23, featuring machine learning algorithms for risk assessment and borrower segmentation.

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Features](#features)
- [Technology Stack](#technology-stack)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Project Structure](#project-structure)
- [Testing](#testing)
- [Contributing](#contributing)
- [License](#license)

## Overview

Smart Debt Recovery System automates the debt collection lifecycle through:

- **Risk Assessment**: Dual algorithm approach using Rule-Based (default) and Random Forest models
- **Borrower Segmentation**: K-Means clustering for categorizing borrowers (Low/Medium/High risk)
- **Recovery Strategies**: Dynamic strategy assignment based on risk profiles
- **Multi-channel Communication**: SMS, Email, and Voice Call notifications

### OOP Principles Applied

| Principle | Implementation |
|-----------|----------------|
| Encapsulation | Private members with getters/setters (Borrower, LoanAccount) |
| Inheritance | RecoveryStrategy base class with concrete implementations |
| Polymorphism | CommunicationChannel hierarchy (SMS/Email/Voice) |
| Abstraction | Pure virtual functions in interfaces (IPaymentChecker, ICommunicationService) |
| Operator Overloading | Money class with arithmetic operators (+, -, *, /) |

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        Web UI (Static Files / Port 3000)                    │
└─────────────────────────────────────┬───────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           API Gateway (Port 8080)                           │
│                    [Routing] [CORS] [Logging] [Proxy]                       │
└───────┬─────────────────┬─────────────────┬─────────────────┬───────────────┘
        │                 │                 │                 │
        ▼                 ▼                 ▼                 ▼
┌───────────────┐ ┌───────────────┐ ┌───────────────┐ ┌───────────────┐
│   Borrower    │ │     Risk      │ │   Recovery    │ │Communication  │
│   Service     │ │  Assessment   │ │   Strategy    │ │   Service     │
│  (Port 8081)  │ │  (Port 8082)  │ │  (Port 8083)  │ │  (Port 8084)  │
└───────┬───────┘ └───────┬───────┘ └───────┬───────┘ └───────┬───────┘
        │                 │                 │                 │
        └─────────────────┴────────┬────────┴─────────────────┘
                                   │
                                   ▼
                        ┌─────────────────────┐
                        │     PostgreSQL      │
                        │     (Port 5432)     │
                        └─────────────────────┘
```

### Services

1. **API Gateway** (Port 8080): Request routing, CORS handling, logging
2. **Borrower Service** (Port 8081): Borrower CRUD, loan accounts, payment history
3. **Risk Assessment Service** (Port 8082): Risk scoring, K-Means clustering
4. **Recovery Strategy Service** (Port 8083): Strategy execution based on risk level
5. **Communication Service** (Port 8084): Multi-channel message delivery

## Features

### Risk Assessment

The system uses a dual algorithm approach:

**Rule-Based Algorithm** (Production Default):
- DTI (Debt-to-Income) ratio calculation
- Zero income detection: +0.35 risk penalty
- Account age factors: New accounts (less than 3 months) receive higher risk scores
- Employment status weighting

**Random Forest Classifier**:
- Trained on 5,000 synthetic samples
- 9 features: days_past_due, missed_payments, debt_ratio, interest_rate, monthly_income, account_age_months, age, employment_status, account_status
- Edge case coverage: 10% of training data includes zero/low income scenarios

### Borrower Segmentation

K-Means clustering algorithm assigns borrowers to risk segments:
- **Low Risk**: Consistent payment history, stable income
- **Medium Risk**: Occasional delays, moderate DTI
- **High Risk**: Multiple missed payments, high DTI or zero income

### Recovery Strategies

| Risk Level | Strategy | Actions |
|------------|----------|---------|
| Low (less than 0.3) | AutomatedReminder | Email/SMS reminders |
| Medium (0.3-0.7) | SettlementOffer | Negotiate payment plans |
| High (greater than 0.7) | LegalAction | Escalate to collections |

## Technology Stack

**Backend:**
- C++23
- CMake 3.20+
- cpp-httplib (HTTP server)
- libpqxx 7.9.2 (PostgreSQL client)
- nlohmann/json (JSON parsing)

**Machine Learning:**
- Custom Random Forest implementation
- K-Means clustering (Lloyd's algorithm)

**Frontend:**
- HTML5/CSS3/JavaScript
- Bootstrap 5.3
- Chart.js

**Infrastructure:**
- Docker and Docker Compose
- PostgreSQL 16

## Prerequisites

- Docker 24.0+ and Docker Compose
- Git
- (Optional) CMake 3.20+ for local development

## Installation

### Using Docker (Recommended)

1. Clone the repository:
```bash
git clone https://github.com/yourusername/smart-debt-recovery-systems.git
cd smart-debt-recovery-systems
```

2. Build and start services:
```bash
docker-compose up --build -d
```

3. Verify services are running:
```bash
docker-compose ps
```

4. Access the application:
- Web UI: http://localhost:3000 (run `python -m http.server 3000` in web/ folder)
- API Gateway: http://localhost:8080
- Direct services: Ports 8081-8084

### Local Development (Without Docker)

1. Install dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libpq-dev libssl-dev nlohmann-json3-dev

# Build libpqxx from source (version 7.9.2 required)
```

2. Build the project:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

3. Run services individually:
```bash
./api-gateway/api-gateway &
./borrower-service/borrower-service &
./risk-assessment-service/risk-assessment-service &
./recovery-strategy-service/recovery-strategy-service &
./communication-service/communication-service &
```

## Usage

### Web Interface

1. Start the web server:
```bash
cd web
python -m http.server 3000
```

2. Navigate to http://localhost:3000 to access:

- **Dashboard**: Overview statistics and charts
- **Borrowers**: Manage borrower profiles, run K-Means segmentation
- **Recovery Actions**: View and execute recovery strategies
- **Demo**: Interactive demonstration of system capabilities

### API Examples

**Create a borrower:**
```bash
curl -X POST http://localhost:8080/api/borrowers \
  -H "Content-Type: application/json" \
  -d '{
    "name": "John Doe",
    "email": "john@example.com",
    "phone_number": "0901234567",
    "monthly_income": 15000000,
    "address": "123 Main St"
  }'
```

**Assess risk:**
```bash
curl -X POST http://localhost:8080/api/risk/assess \
  -H "Content-Type: application/json" \
  -d '{
    "borrower_id": 1,
    "account_id": 1,
    "days_past_due": 30,
    "missed_payments": 2,
    "loan_amount": 50000000,
    "remaining_amount": 45000000,
    "monthly_income": 10000000,
    "account_age_months": 6
  }'
```

**Run K-Means segmentation:**
```bash
curl -X POST http://localhost:8080/api/borrowers/segment
```

## API Reference

### Borrower Service (Port 8081)

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | /borrowers | List all borrowers |
| GET | /borrowers/{id} | Get borrower by ID |
| POST | /borrowers | Create new borrower |
| PUT | /borrowers/{id} | Update borrower |
| DELETE | /borrowers/{id} | Delete borrower |
| GET | /borrowers/{id}/loan-accounts | Get loan accounts |
| GET | /borrowers/{id}/payment-history | Get payment history |

### Risk Assessment Service (Port 8082)

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | /assess-risk | Calculate risk score |
| POST | /segment | Run K-Means clustering |
| GET | /model/status | Get algorithm status |

### Recovery Strategy Service (Port 8083)

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | /execute-strategy | Execute recovery strategy |
| GET | /strategies | List available strategies |

### Communication Service (Port 8084)

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | /send | Send communication |
| GET | /logs/{borrower_id} | Get communication logs |

## Project Structure

```
smart-debt-recovery-systems/
├── api-gateway/                 # Request routing service
│   ├── include/
│   └── src/
├── borrower-service/            # Borrower management
│   ├── include/
│   │   ├── interfaces/
│   │   ├── models/
│   │   ├── repositories/
│   │   └── services/
│   ├── src/
│   └── tests/
├── risk-assessment-service/     # ML-based risk scoring
│   ├── include/
│   │   ├── algorithms/
│   │   ├── models/
│   │   └── repositories/
│   ├── src/
│   └── tests/
├── recovery-strategy-service/   # Strategy pattern implementation
│   ├── include/
│   │   ├── interfaces/
│   │   ├── managers/
│   │   └── strategies/
│   ├── src/
│   └── tests/
├── communication-service/       # Multi-channel messaging
│   ├── include/
│   │   ├── channels/
│   │   ├── interfaces/
│   │   └── managers/
│   ├── src/
│   └── tests/
├── common/                      # Shared utilities
│   ├── include/
│   │   ├── database/
│   │   ├── exceptions/
│   │   ├── models/
│   │   └── utils/
│   └── src/
├── database/
│   └── schema.sql               # Database schema
├── web/                         # Frontend application
│   ├── css/
│   ├── js/
│   └── *.html
├── scripts/                     # Utility scripts
├── docker-compose.yml           # Container orchestration
├── Dockerfile                   # Multi-stage build
└── CMakeLists.txt               # Root build configuration
```

## Testing

### Manual Testing

```bash
# Test borrower service
curl http://localhost:8081/health

# Test risk assessment with edge case (zero income)
curl -X POST http://localhost:8082/assess-risk \
  -H "Content-Type: application/json" \
  -d '{"monthly_income":0,"loan_amount":50000000,"days_past_due":15}'
```

### Unit Tests

```bash
cd build
ctest --output-on-failure
```

### Integration Tests

The scripts/ directory contains integration test scripts:
```bash
./scripts/test_services.sh
./scripts/test_borrower.sh
./scripts/test_risk.sh
```

## Portability Notes

**Binary Distribution:**
The compiled binaries are platform-specific (Linux x86_64 when built with Docker). To run on a different machine:

1. **Recommended**: Clone the repository and run `docker-compose up --build`
2. **Alternative**: Copy the entire project directory and rebuild locally

The following directories should NOT be included in distribution:
- build/ - Local build artifacts
- .venv/ - Python virtual environment
- logs/ - Runtime logs
- __pycache__/ - Python cache

## Contributing

1. Fork the repository
2. Create a feature branch (git checkout -b feature/new-feature)
3. Commit changes (git commit -am 'Add new feature')
4. Push to branch (git push origin feature/new-feature)
5. Create a Pull Request

Please follow the C++ Core Guidelines (https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) for code style.

## License

This project is licensed under the MIT License. See LICENSE for details.

## Authors

- **24120395** - Nguyen Van Nguyen (Team Lead)
- **24120326** - Pham Ba Quoc Huy
