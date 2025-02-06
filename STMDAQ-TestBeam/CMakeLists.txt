cmake_minimum_required(VERSION 3.16)
project(STMDAQ)

# Define the build directories
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)

file(MAKE_DIRECTORY ${BUILD_DIR}/lib)
file(MAKE_DIRECTORY ${BUILD_DIR}/obj)
file(MAKE_DIRECTORY ${BUILD_DIR}/bin)
file(MAKE_DIRECTORY ${BUILD_DIR}/dep)

# Custom targets for submodules
add_custom_target(utils_code
    COMMAND ${CMAKE_COMMAND} -E echo "<**Building utils directory**>"
    COMMAND make -C ${CMAKE_SOURCE_DIR}/utils all
)

## Clean-up
#add_custom_target(clean
#    COMMAND ${CMAKE_COMMAND} -E rm -rf ${BUILD_DIR}/bin ${BUILD_DIR}/obj ${BUILD_DIR}/lib ${BUILD_DIR}/dep
#    COMMENT "Cleaning build directories"
#)

# Define all target to build everything
#add_custom_target(all
#    DEPENDS utils_code
#)

# Uncomment below for optional targets
# add_custom_target(online_code
#     COMMAND ${CMAKE_COMMAND} -E echo "<**Building online directory**>"
#     COMMAND make -C ${CMAKE_SOURCE_DIR}/online all
# )
