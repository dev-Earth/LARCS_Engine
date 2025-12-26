cmake_minimum_required(VERSION 3.25)

# Runtime library sources
set(RUNTIME_SOURCES
  src/time.cpp
  src/transport/zenoh_transport.cpp
  src/transport/publisher.cpp
  src/transport/subscriber.cpp
  src/logger.cpp
)

set(RUNTIME_HEADERS
  include/larcs/runtime/time.hpp
  include/larcs/runtime/transport.hpp
  include/larcs/runtime/zenoh_transport.hpp
  include/larcs/runtime/publisher.hpp
  include/larcs/runtime/subscriber.hpp
  include/larcs/runtime/logger.hpp
)

# Find Zenoh
find_package(Zenoh REQUIRED)

# Create runtime library
larcs_add_library(larcs-runtime
  SOURCES ${RUNTIME_SOURCES}
  HEADERS ${RUNTIME_HEADERS}
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  DEPENDENCIES
    fmt::fmt
    spdlog::spdlog
    zenohc::lib
    protobuf::libprotobuf
    larcs-msgs
)

# Tests
larcs_add_test(time_test
  SOURCES tests/time_test.cpp
  DEPENDENCIES
    larcs-runtime
    GTest::gtest
    GTest::gtest_main
)

larcs_add_test(transport_test
  SOURCES tests/transport_test.cpp
  DEPENDENCIES
    larcs-runtime
    GTest::gtest
    GTest::gtest_main
    zenohc::lib
)

larcs_add_test(pubsub_test
  SOURCES tests/pubsub_test.cpp
  DEPENDENCIES
    larcs-runtime
    larcs-msgs
    GTest::gtest
    GTest::gtest_main
    zenohc::lib
)
