cmake_minimum_required(VERSION 3.25)
project(LARCS_Engine VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add custom module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Include common CMake functions
include(cmake/larcs.cmake)

# Find required packages
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Zenoh REQUIRED)
find_package(unofficial-mcap CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(msgs)
add_subdirectory(runtime)
add_subdirectory(tools)
