#!/bin/bash

ROOT="."

# ===== Helper =====
create_dir() {
    mkdir -p "$ROOT/$1"
}

create_file() {
    mkdir -p "$(dirname "$ROOT/$1")"
    touch "$ROOT/$1"
}

# ===== Root structure =====
mkdir -p $ROOT
touch $ROOT/docker-compose.yml
touch $ROOT/.gitignore
touch $ROOT/.dockerignore
touch $ROOT/CMakeLists.txt
touch $ROOT/conanfile.txt
touch $ROOT/README.md

# ===== API Gateway =====
create_file "api-gateway/Dockerfile"
create_file "api-gateway/CMakeLists.txt"
create_file "api-gateway/src/main.cpp"

# ===== Borrower Service =====
create_file "borrower-service/Dockerfile"
create_file "borrower-service/CMakeLists.txt"
create_file "borrower-service/include/models/Borrower.h"
create_file "borrower-service/include/models/LoanAccount.h"
create_file "borrower-service/include/models/PaymentHistory.h"
create_file "borrower-service/include/repositories/BorrowerRepository.h"

create_file "borrower-service/src/main.cpp"
create_file "borrower-service/src/models/Borrower.cpp"
create_file "borrower-service/src/models/LoanAccount.cpp"
create_file "borrower-service/src/models/PaymentHistory.cpp"
create_file "borrower-service/src/repositories/BorrowerRepository.cpp"

create_file "borrower-service/tests/test_borrower.cpp"

# ===== Risk Assessment Service =====
create_file "risk-assessment-service/Dockerfile"
create_file "risk-assessment-service/CMakeLists.txt"
create_file "risk-assessment-service/include/models/RiskScorer.h"
create_file "risk-assessment-service/include/algorithms/RandomForest.h"
create_file "risk-assessment-service/include/algorithms/KMeansClustering.h"

create_file "risk-assessment-service/src/main.cpp"
create_file "risk-assessment-service/src/models/RiskScorer.cpp"
create_file "risk-assessment-service/src/algorithms/RandomForest.cpp"
create_file "risk-assessment-service/src/algorithms/KMeansClustering.cpp"

create_file "risk-assessment-service/tests/test_risk_scorer.cpp"

# ===== Recovery Strategy Service =====
create_file "recovery-strategy-service/Dockerfile"
create_file "recovery-strategy-service/CMakeLists.txt"
create_file "recovery-strategy-service/include/strategies/RecoveryStrategy.h"
create_file "recovery-strategy-service/include/strategies/AutomatedReminderStrategy.h"
create_file "recovery-strategy-service/include/strategies/SettlementOfferStrategy.h"
create_file "recovery-strategy-service/include/strategies/LegalActionStrategy.h"
create_file "recovery-strategy-service/include/managers/StrategyManager.h"

create_file "recovery-strategy-service/src/main.cpp"
create_file "recovery-strategy-service/src/strategies/RecoveryStrategy.cpp"
create_file "recovery-strategy-service/src/strategies/AutomatedReminderStrategy.cpp"
create_file "recovery-strategy-service/src/strategies/SettlementOfferStrategy.cpp"
create_file "recovery-strategy-service/src/strategies/LegalActionStrategy.cpp"
create_file "recovery-strategy-service/src/managers/StrategyManager.cpp"

create_file "recovery-strategy-service/tests/test_strategies.cpp"

# ===== Communication Service =====
create_file "communication-service/Dockerfile"
create_file "communication-service/CMakeLists.txt"
create_file "communication-service/include/channels/CommunicationChannel.h"
create_file "communication-service/include/channels/SMSChannel.h"
create_file "communication-service/include/channels/EmailChannel.h"
create_file "communication-service/include/channels/VoiceCallChannel.h"
create_file "communication-service/include/managers/CommunicationManager.h"

create_file "communication-service/src/main.cpp"
create_file "communication-service/src/channels/CommunicationChannel.cpp"
create_file "communication-service/src/channels/SMSChannel.cpp"
create_file "communication-service/src/channels/EmailChannel.cpp"
create_file "communication-service/src/channels/VoiceCallChannel.cpp"
create_file "communication-service/src/managers/CommunicationManager.cpp"

create_file "communication-service/tests/test_communication.cpp"

# ===== Database =====
create_file "database/schema.sql"
create_file "database/init.sql"

# ===== Common =====
create_file "common/CMakeLists.txt"
create_file "common/include/utils/Logger.h"
create_file "common/include/utils/Config.h"
create_file "common/include/utils/Constants.h"
create_file "common/include/exceptions/DatabaseException.h"
create_file "common/include/exceptions/CommunicationException.h"
create_file "common/include/exceptions/ValidationException.h"
create_file "common/include/models/Money.h"
create_file "common/include/models/Response.h"

create_file "common/src/utils/Logger.cpp"
create_file "common/src/utils/Config.cpp"
create_file "common/src/utils/Constants.cpp"
create_file "common/src/exceptions/DatabaseException.cpp"
create_file "common/src/exceptions/CommunicationException.cpp"
create_file "common/src/exceptions/ValidationException.cpp"

echo "Project skeleton generated successfully!"