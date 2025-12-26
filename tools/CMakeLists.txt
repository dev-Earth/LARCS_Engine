cmake_minimum_required(VERSION 3.25)

# Ping tool
larcs_add_executable(larcs-ping
  SOURCES src/ping.cpp
  DEPENDENCIES
    fmt::fmt
    CLI11::CLI11
)

# Record tool
larcs_add_executable(larcs-record
  SOURCES src/record.cpp
  DEPENDENCIES
    larcs-runtime
    larcs-msgs
    fmt::fmt
    CLI11::CLI11
    spdlog::spdlog
)

# Replay tool
larcs_add_executable(larcs-replay
  SOURCES src/replay.cpp
  DEPENDENCIES
    larcs-runtime
    larcs-msgs
    fmt::fmt
    CLI11::CLI11
    spdlog::spdlog
)

# Pub tool
larcs_add_executable(larcs-pub
  SOURCES src/pub.cpp
  DEPENDENCIES
    larcs-runtime
    larcs-msgs
    fmt::fmt
    CLI11::CLI11
    spdlog::spdlog
)

# Sub tool
larcs_add_executable(larcs-sub
  SOURCES src/sub.cpp
  DEPENDENCIES
    larcs-runtime
    larcs-msgs
    fmt::fmt
    CLI11::CLI11
    spdlog::spdlog
)
