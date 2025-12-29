# Recovery Strategy Service - Determines optimal debt recovery strategies

# Source files
set(RECOVERY_STRATEGY_SERVICE_SOURCES
    # Main entry point
    src/main.cpp
    
    # Strategies
    src/strategies/RecoveryStrategy.cpp
    src/strategies/AutomatedReminderStrategy.cpp
    src/strategies/SettlementOfferStrategy.cpp
    src/strategies/LegalActionStrategy.cpp
    
    # Managers - TODO Phase 3: Uncomment when RiskScorer is linked
    # src/managers/StrategyManager.cpp
)

# Header files (for IDE support)
set(RECOVERY_STRATEGY_SERVICE_HEADERS
    # Strategies
    include/strategies/RecoveryStrategy.h
    include/strategies/AutomatedReminderStrategy.h
    include/strategies/SettlementOfferStrategy.h
    include/strategies/LegalActionStrategy.h
    
    # Interfaces
    include/interfaces/IPaymentChecker.h
    
    # Managers
    include/managers/StrategyManager.h
)

# Create executable
add_executable(recovery_strategy_service ${RECOVERY_STRATEGY_SERVICE_SOURCES} ${RECOVERY_STRATEGY_SERVICE_HEADERS})

# Include directories
target_include_directories(recovery_strategy_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(recovery_strategy_service PRIVATE
    sdrs_common
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# Set properties
set_target_properties(recovery_strategy_service PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "recovery-strategy-service"
)
