# API Gateway - Central entry point for all client requests

# Source files
set(API_GATEWAY_SOURCES
    src/main.cpp
    src/middleware/Middleware.cpp
    src/registry/ServiceRegistry.cpp
)

# Header files (for IDE support)
set(API_GATEWAY_HEADERS
    include/middleware/Middleware.h
    include/registry/ServiceRegistry.h
)

# Create executable
add_executable(api_gateway ${API_GATEWAY_SOURCES} ${API_GATEWAY_HEADERS})

# Include directories
target_include_directories(api_gateway PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(api_gateway PRIVATE
    PRIVATE
        sdrs_common
        nlohmann_json::nlohmann_json
        httplib::httplib
)

# Set properties
set_target_properties(api_gateway PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "api-gateway"
)
