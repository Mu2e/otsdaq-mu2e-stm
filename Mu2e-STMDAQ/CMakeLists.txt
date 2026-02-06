cmake_minimum_required(VERSION 3.16)

project(STMDAQ LANGUAGES CXX)

# ---------------------------
# Define the build layout
# ---------------------------
# All libraries/executables go under a single build directory
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR}/lib)

# Optionally make the directories visible
file(MAKE_DIRECTORY
    ${BUILD_DIR}/lib
    ${BUILD_DIR}/bin
)

# Extra system libraries used throughout STMDAQ
set(STMDAQ_EXTRA_LIBS
  rt
  CACHE INTERNAL "Common extra system libraries for STMDAQ"
)

# ---------------------------
# Include centralized config
# ---------------------------
include(${CMAKE_CURRENT_LIST_DIR}/cmake/STMDAQConfig.cmake)

# ---------------------------
# Add subdirectories
# ---------------------------
add_subdirectory(utils)
# Add other modules as they are converted:
add_subdirectory(interfaces)
add_subdirectory(config)
add_subdirectory(buffers)
add_subdirectory(debug)
add_subdirectory(processing)
add_subdirectory(dqm)
add_subdirectory(simfw)
add_subdirectory(frontend)

# ---------------------------
# Optional: top-level install
# ---------------------------
# CET macros usually handle install in each module
