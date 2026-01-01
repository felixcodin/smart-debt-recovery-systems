# Communication Service - Handles all borrower communications (Email, SMS, Voice)

# Source files
set(COMMUNICATION_SERVICE_SOURCES
    # Main entry point
    src/main.cpp
    
    # Channels
    src/channels/EmailChannel.cpp
    src/channels/SMSChannel.cpp
    src/channels/VoiceCallChannel.cpp
    
    # Managers
    src/managers/CommunicationManager.cpp
    
    # Models
    src/models/CommunicationLog.cpp
    
    # Repositories
    src/repositories/CommunicationLogRepository.cpp
)

# Header files (for IDE support)
set(COMMUNICATION_SERVICE_HEADERS
    # Channels
    include/channels/CommunicationChannel.h
    include/channels/EmailChannel.h
    include/channels/SMSChannel.h
    include/channels/VoiceCallChannel.h
    
    # Interfaces
    include/interfaces/ICommunicationService.h
    
    # Managers
    include/managers/CommunicationManager.h
    
    # Models
    include/models/CommunicationLog.h
    
    # Repositories
    include/repositories/CommunicationLogRepository.h
)

# Create executable
add_executable(communication_service ${COMMUNICATION_SERVICE_SOURCES} ${COMMUNICATION_SERVICE_HEADERS})

# Include directories
target_include_directories(communication_service PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(communication_service PRIVATE
    sdrs_common
    nlohmann_json::nlohmann_json
    httplib::httplib
)

# Set properties
set_target_properties(communication_service PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "communication-service"
)
