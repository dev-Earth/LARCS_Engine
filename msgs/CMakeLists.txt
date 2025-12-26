cmake_minimum_required(VERSION 3.25)

# Generate protobuf C++ sources
set(PROTO_FILES
  proto/common.proto
  proto/geometry.proto
  proto/control.proto
  proto/health.proto
  proto/esp32.proto
)

# Set up the output directory for generated files
set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${PROTO_BINARY_DIR}/larcs/msgs")

# Generate the protobuf sources
protobuf_generate_cpp(PROTO_GENERATED_SRCS PROTO_GENERATED_HDRS
  ${PROTO_FILES}
)

# Create protobuf library with actual sources
add_library(larcs-msgs ${PROTO_GENERATED_SRCS} ${PROTO_GENERATED_HDRS})

# Add generated directory to include paths
target_include_directories(larcs-msgs PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link protobuf library
target_link_libraries(larcs-msgs PUBLIC protobuf::libprotobuf)

# Set properties
set_target_properties(larcs-msgs PROPERTIES
  LINKER_LANGUAGE CXX
  PUBLIC_HEADER "${PROTO_GENERATED_HDRS}"
)
