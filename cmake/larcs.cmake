# LARCS CMake Common Functions
# This file contains reusable CMake functions for LARCS modules

# Function to add a LARCS library with common settings
function(larcs_add_library TARGET_NAME)
  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SOURCES HEADERS DEPENDENCIES INCLUDE_DIRS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_library(${TARGET_NAME} ${ARG_SOURCES} ${ARG_HEADERS})
  
  target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)
  
  target_compile_options(${TARGET_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
  )
  
  if(ARG_INCLUDE_DIRS)
    target_include_directories(${TARGET_NAME} PUBLIC ${ARG_INCLUDE_DIRS})
  endif()
  
  if(ARG_DEPENDENCIES)
    target_link_libraries(${TARGET_NAME} PUBLIC ${ARG_DEPENDENCIES})
  endif()
endfunction()

# Function to add a LARCS executable with common settings
function(larcs_add_executable TARGET_NAME)
  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SOURCES DEPENDENCIES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${TARGET_NAME} ${ARG_SOURCES})
  
  target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)
  
  target_compile_options(${TARGET_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
  )
  
  if(ARG_DEPENDENCIES)
    target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_DEPENDENCIES})
  endif()
endfunction()

# Function to add a LARCS test with common settings
function(larcs_add_test TEST_NAME)
  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SOURCES DEPENDENCIES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${TEST_NAME} ${ARG_SOURCES})
  
  target_compile_features(${TEST_NAME} PUBLIC cxx_std_20)
  
  target_compile_options(${TEST_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
  )
  
  if(ARG_DEPENDENCIES)
    target_link_libraries(${TEST_NAME} PRIVATE ${ARG_DEPENDENCIES})
  endif()
  
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()
