add_executable(larcs-matrix main.cpp)

target_link_libraries(larcs-matrix PRIVATE
  larcs-sim
  larcs-runtime
  CLI11::CLI11
  spdlog::spdlog
)

install(TARGETS larcs-matrix DESTINATION bin)
