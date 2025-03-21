cmake_minimum_required(VERSION 3.16)
project(STMDAQ)

# Define the build directories
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)

file(MAKE_DIRECTORY ${BUILD_DIR}/lib)
file(MAKE_DIRECTORY ${BUILD_DIR}/obj)
file(MAKE_DIRECTORY ${BUILD_DIR}/bin)
file(MAKE_DIRECTORY ${BUILD_DIR}/dep)

add_subdirectory(utils)

add_subdirectory(processData)

add_subdirectory(frontEnd)

