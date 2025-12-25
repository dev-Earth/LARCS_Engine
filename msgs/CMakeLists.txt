cmake_minimum_required(VERSION 3.25)

# Generate protobuf C++ sources
set(PROTO_FILES
  proto/common.proto
  proto/geometry.proto
  proto/control.proto
  proto/health.proto
)

# Create protobuf library
add_library(larcs-msgs INTERFACE)

# Generate protobuf files
protobuf_generate(
  TARGET larcs-msgs
  LANGUAGE cpp
  PROTOS ${PROTO_FILES}
  IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/proto
  PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated
)

# Add generated directory to include paths
target_include_directories(larcs-msgs INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}/generated
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link protobuf library
target_link_libraries(larcs-msgs INTERFACE protobuf::libprotobuf)

# Set properties to ensure generated files are created
set_target_properties(larcs-msgs PROPERTIES
  LINKER_LANGUAGE CXX
)
