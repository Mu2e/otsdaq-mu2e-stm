# cmake/STMDAQConfig.cmake
# Central configuration for STMDAQ modules
# Uses BUILD_DIR layout under CMAKE_BINARY_DIR

# ---- Build directories ----
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)

# Core STMDAQ headers (config/include)
set(STMDAQ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/config/include)

# ---- Boost ----
if(NOT DEFINED ENV{CACTUS_ROOT})
    message(FATAL_ERROR "BOOST_ROOT environment variable not set")
endif()

set(BOOST_ROOT $ENV{CACTUS_ROOT})
set(BOOST_INCLUDE_DIR ${BOOST_ROOT}/include)
set(BOOST_LIBRARY_DIR ${BOOST_ROOT}/lib)

# Components commonly used
set(BOOST_COMPONENTS
    program_options
    chrono
    random
    system
)

# ---- PugiXML ----
find_package(PugiXML REQUIRED)
# Optional: show the version
message(STATUS "Found PugiXML: ${PUGIXML_VERSION}")
set(PUGIXML_INCLUDE_DIR /usr/include)
set(PUGIXML_LIBRARY_DIR /usr/lib64/libpugixml.so)

# ---- Python / pybind11 ----
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
#find_package(pybind11 REQUIRED)

set(PYTHON_INCLUDE_DIRS
    ${Python3_INCLUDE_DIRS}
#    ${pybind11_INCLUDE_DIRS}
)

set(PYTHON_LIBRARIES ${Python3_LIBRARIES})

# ---- STMDAQ include/library paths ----
# Libraries will be built under ${BUILD_DIR}/lib
set(STMDAQ_INCLUDE_DIR ${CMAKE_SOURCE_DIR})
set(STMDAQ_LIBRARY_DIR ${BUILD_DIR}/lib)

# Prebuilt STMDAQ libraries (CET targets)
# These are the names of cet_make_library targets
set(STMDAQ_LIBS
    UtilsLibraryX
    #ConfigLibrary
    #BuffersLibrary
    #DebugLibrary
    #ProcessingLibrary
    #DQMLibrary
    #SimLibrary
)
# Make STMDAQ_INCLUDE_DIR visible to all subdirectories
include_directories(${STMDAQ_INCLUDE_DIR})

macro(stmdaq_target_includes target)
  target_include_directories(${target}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../>
      ${BOOST_INCLUDE_DIR}
      ${PYTHON_INCLUDE_DIRS}
      ${PUGIXML_INCLUDE_DIR}
    )
endmacro()

# ---- Optional: extra libraries (IPBus, RT, etc) ----
# Extra system libraries used throughout STMDAQ
set(STMDAQ_EXTRA_LIBS
  rt
  CACHE INTERNAL "Common extra system libraries for STMDAQ"
)

