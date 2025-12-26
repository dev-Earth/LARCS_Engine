# FindZenoh.cmake - Find Zenoh library
# This module defines:
#  Zenoh_FOUND - System has Zenoh
#  Zenoh_INCLUDE_DIRS - The Zenoh include directories
#  Zenoh_LIBRARIES - The libraries needed to use Zenoh
#  zenohc::lib - Imported target for Zenoh

# Prefer the vendored zenoh-c (if present) over any system installation.
# This avoids accidental ABI/API mismatches when /usr/local has a different zenoh version.
set(_LARCS_VENDORED_ZENOH_ROOT "${CMAKE_SOURCE_DIR}/zenoh-c-1.7.1")
set(_LARCS_VENDORED_ZENOH_HINTS "")
if(EXISTS "${_LARCS_VENDORED_ZENOH_ROOT}")
  # Common locations for zenoh-c when built from this repository checkout.
  list(APPEND _LARCS_VENDORED_ZENOH_HINTS
    "${_LARCS_VENDORED_ZENOH_ROOT}"
    "${_LARCS_VENDORED_ZENOH_ROOT}/build"
    "${_LARCS_VENDORED_ZENOH_ROOT}/build/default"
    "${_LARCS_VENDORED_ZENOH_ROOT}/target"
    "${_LARCS_VENDORED_ZENOH_ROOT}/target/release"
    "${_LARCS_VENDORED_ZENOH_ROOT}/target/debug"
  )
endif()

# Try to find zenoh using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_Zenoh QUIET zenohc)
endif()

# Find the include directory
find_path(Zenoh_INCLUDE_DIR
  NAMES zenoh.h
  HINTS
    ${_LARCS_VENDORED_ZENOH_HINTS}
    ${PC_Zenoh_INCLUDE_DIRS}
    /usr/local/include
    /usr/include
  PATH_SUFFIXES
    include
    zenoh
)

# Find the library
find_library(Zenoh_LIBRARY
  NAMES zenohc
  HINTS
    ${_LARCS_VENDORED_ZENOH_HINTS}
    ${PC_Zenoh_LIBRARY_DIRS}
    /usr/local/lib
    /usr/lib
  PATH_SUFFIXES
    lib
    release
    debug
)

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Zenoh
  FOUND_VAR Zenoh_FOUND
  REQUIRED_VARS
    Zenoh_LIBRARY
    Zenoh_INCLUDE_DIR
  VERSION_VAR PC_Zenoh_VERSION
)

if(Zenoh_FOUND)
  set(Zenoh_LIBRARIES ${Zenoh_LIBRARY})
  set(Zenoh_INCLUDE_DIRS ${Zenoh_INCLUDE_DIR})

  # Create imported target
  if(NOT TARGET zenohc::lib)
    add_library(zenohc::lib UNKNOWN IMPORTED)
    set_target_properties(zenohc::lib PROPERTIES
      IMPORTED_LOCATION "${Zenoh_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${Zenoh_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(Zenoh_INCLUDE_DIR Zenoh_LIBRARY)
