cmake_minimum_required(VERSION 3.25)

# Find dependencies
find_package(Eigen3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# PhysX and USD are optional for now (will be required when installed)
# find_package(PhysX REQUIRED)
# find_package(pxr REQUIRED)

# Simulation library sources
set(SIM_SOURCES
  src/world.cpp
  src/sensor/sensor_base.cpp
  src/sensor/encoder_sensor.cpp
  src/sensor/imu_sensor.cpp
  src/sensor/sensor_factory.cpp
  src/robot/robot_assembly.cpp
  src/usd_loader.cpp
)

set(SIM_HEADERS
  include/larcs/sim/world.hpp
  include/larcs/sim/sensor/sensor_base.hpp
  include/larcs/sim/sensor/encoder_sensor.hpp
  include/larcs/sim/sensor/imu_sensor.hpp
  include/larcs/sim/sensor/sensor_factory.hpp
  include/larcs/sim/robot/robot_assembly.hpp
  include/larcs/sim/usd_loader.hpp
)

# Create simulation library
larcs_add_library(larcs-sim
  SOURCES ${SIM_SOURCES}
  HEADERS ${SIM_HEADERS}
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  DEPENDENCIES
    larcs-runtime
    Eigen3::Eigen
    yaml-cpp
    spdlog::spdlog
)

# Add PhysX when available
# target_link_libraries(larcs-sim PUBLIC ${PhysX_LIBRARIES})
# target_include_directories(larcs-sim PUBLIC ${PhysX_INCLUDE_DIRS})

# Add USD when available
# target_link_libraries(larcs-sim PUBLIC ${USD_LIBRARIES})
# target_include_directories(larcs-sim PUBLIC ${USD_INCLUDE_DIRS})

# Tests
larcs_add_test(world_test
  SOURCES tests/world_test.cpp
  DEPENDENCIES
    larcs-sim
    GTest::gtest
    GTest::gtest_main
)

larcs_add_test(sensor_test
  SOURCES tests/sensor_test.cpp
  DEPENDENCIES
    larcs-sim
    GTest::gtest
    GTest::gtest_main
)
