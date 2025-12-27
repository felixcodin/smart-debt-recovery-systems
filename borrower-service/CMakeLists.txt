# Borrower Service - Manages borrower/debtor information

# Source files
set(BORROWER_SERVICE_SOURCES
    # Main entry point
    src/main.cpp
    
    # Models
    src/models/Borrower.cpp
    src/models/LoanAccount.cpp
    src/models/PaymentHistory.cpp
    
    # Repositories
    src/repositories/BorrowerRepository.cpp
    
    # Services
    src/services/PaymentCheckerImpl.cpp
)

# Header files (for IDE support)
set(BORROWER_SERVICE_HEADERS
    # Models
    include/models/Borrower.h
    include/models/LoanAccount.h
    include/models/PaymentHistory.h
    
    # Repositories
    include/repositories/BorrowerRepository.h
    include/interfaces/IBorrowerRepository.h
    
    # Services
    include/services/PaymentCheckerImpl.h
)

# Create executable
add_executable(borrower_service ${BORROWER_SERVICE_SOURCES} ${BORROWER_SERVICE_HEADERS})

# Include directories
target_include_directories(borrower_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(borrower_service PRIVATE
    sdrs_common
    nlohmann_json::nlohmann_json
    # libpqxx::pqxx  # TODO: Add when libpqxx is available
    httplib::httplib
)

# Set properties
set_target_properties(borrower_service PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "borrower-service"
)

# Installation (optional)
install(TARGETS borrower_service
    RUNTIME DESTINATION bin
)
