# Common library - Shared utilities and models across all services

# Source files
set(COMMON_SOURCES
    # Utils
    src/utils/Config.cpp
    src/utils/Logger.cpp
    
    # Models
    src/models/Money.cpp
    
    # Exceptions
    src/exceptions/ValidationException.cpp
    src/exceptions/DatabaseException.cpp
    src/exceptions/CommunicationException.cpp
)

# Header files (for IDE support)
set(COMMON_HEADERS
    # Utils
    include/utils/Config.h
    include/utils/Logger.h
    include/utils/Constants.h
    
    # Models
    include/models/Money.h
    include/models/Response.h
    
    # Database
    include/database/DatabaseManager.h
    
    # Exceptions
    include/exceptions/ValidationException.h
    include/exceptions/DatabaseException.h
    include/exceptions/CommunicationException.h
)

# Create static library
add_library(sdrs_common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

# Include directories
target_include_directories(sdrs_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Find libpqxx
find_package(libpqxx QUIET)

# Link dependencies
target_link_libraries(sdrs_common PUBLIC
    nlohmann_json::nlohmann_json
    $<$<TARGET_EXISTS:libpqxx::pqxx>:libpqxx::pqxx>
)

# Set properties
set_target_properties(sdrs_common PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)
