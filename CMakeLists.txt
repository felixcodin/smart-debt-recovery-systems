cmake_minimum_required(VERSION 3.20)
project(SmartDebtRecoverySystem VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Conan dependencies
find_package(nlohmann_json REQUIRED)
find_package(libpqxx QUIET)
find_package(httplib REQUIRED)

# Subdirectories
add_subdirectory(common)
add_subdirectory(borrower-service)
add_subdirectory(communication-service)
add_subdirectory(api-gateway)
add_subdirectory(recovery-strategy-service)
add_subdirectory(risk-assessment-service)
