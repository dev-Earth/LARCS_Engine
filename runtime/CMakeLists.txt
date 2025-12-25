cmake_minimum_required(VERSION 3.25)

# Runtime library sources
set(RUNTIME_SOURCES
  src/time.cpp
  src/publisher.cpp
  src/subscriber.cpp
  src/logger.cpp
)

set(RUNTIME_HEADERS
  include/larcs/runtime/time.hpp
  include/larcs/runtime/publisher.hpp
  include/larcs/runtime/subscriber.hpp
  include/larcs/runtime/logger.hpp
)

# Create runtime library
larcs_add_library(larcs-runtime
  SOURCES ${RUNTIME_SOURCES}
  HEADERS ${RUNTIME_HEADERS}
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  DEPENDENCIES
    fmt::fmt
    spdlog::spdlog
)

# Tests
larcs_add_test(time_test
  SOURCES tests/time_test.cpp
  DEPENDENCIES
    larcs-runtime
    GTest::gtest
    GTest::gtest_main
)
