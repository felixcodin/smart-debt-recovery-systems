# Risk Assessment Service - Evaluates borrower risk using ML algorithms

# Source files
set(RISK_ASSESSMENT_SERVICE_SOURCES
    # Main entry point
    src/main.cpp
    
    # Algorithms
    src/algorithms/KMeansClustering.cpp
    src/algorithms/RandomForest.cpp
    
    # Models
    src/models/RiskScorer.cpp
)

# Header files (for IDE support)
set(RISK_ASSESSMENT_SERVICE_HEADERS
    # Algorithms
    include/algorithms/KMeansClustering.h
    include/algorithms/RandomForest.h
    
    # Models
    include/models/RiskScorer.h
)

# Create executable
add_executable(risk_assessment_service ${RISK_ASSESSMENT_SERVICE_SOURCES} ${RISK_ASSESSMENT_SERVICE_HEADERS})

# Include directories
target_include_directories(risk_assessment_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(risk_assessment_service PRIVATE
    sdrs_common
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# Set properties
set_target_properties(risk_assessment_service PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "risk-assessment-service"
)
