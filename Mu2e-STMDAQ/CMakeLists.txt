cmake_minimum_required(VERSION 3.20)

project(STMDAQ LANGUAGES C CXX)

# -----------------------------------------------------------------------------
# Require environment setup (setup.sh)
# -----------------------------------------------------------------------------
if(NOT DEFINED ENV{STMDAQ_ROOT})
  message(FATAL_ERROR "STMDAQ_ROOT not set. You must source setup.sh before configuring.")
endif()
set(STMDAQ_ROOT "$ENV{STMDAQ_ROOT}")
set(STM_INC "${STMDAQ_ROOT}/include")

# Optional
if(DEFINED ENV{CACTUS_ROOT})
  set(CACTUS_ROOT "$ENV{CACTUS_ROOT}")
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
    "In-source builds are not allowed.\n"
    "Use: cmake -S . -B build-cmake"
  )
endif()

# -----------------------------------------------------------------------------
# Are we using a spack/CET buiid?
# -----------------------------------------------------------------------------
option(USE_CET "Build using CET instead of plain CMake" ON)

if(USE_CET)
  message(STATUS "Build mode: CET")
else()
  message(STATUS "Build mode: standard CMake")
endif()

# -----------------------------------------------------------------------------
# Output directories 
# -----------------------------------------------------------------------------
# Put all built binaries and libraries under the build directory
set(STMDAQ_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(STMDAQ_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
set(STMDAQ_LOG_DIR "${CMAKE_BINARY_DIR}/log")  # runtime logs (optional)

# Standard CMake output locations (single-config generators)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${STMDAQ_BIN_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${STMDAQ_LIB_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${STMDAQ_LIB_DIR}")

# Multi-config generators (VS/Xcode) use per-config variables
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER "${cfg}" CFGU)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFGU} "${STMDAQ_BIN_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFGU} "${STMDAQ_LIB_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFGU} "${STMDAQ_LIB_DIR}")
endforeach()

# -----------------------------------------------------------------------------
# Language / default build behavior
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -----------------------------------------------------------------------------
# Global compile flags + warnings + core includes
# Everything inherits these by linking stmdaq_build_options
# -----------------------------------------------------------------------------
add_library(stmdaq_build_options INTERFACE)

target_compile_options(stmdaq_build_options INTERFACE
  -O3
  -march=native
  -fPIC
  $<$<COMPILE_LANGUAGE:CXX>:
    -Wall
    -Wextra
    -Wpedantic  
    -Wformat=2
    -Wno-format-nonliteral
    -Wno-strict-aliasing
    -Wuninitialized
    -Wno-unused-function
    -Wno-unused-parameter
    -Wno-missing-field-initializers
    -Wno-pointer-arith
    -Wno-unused-variable
    -Wno-unused-local-typedefs
    -Wno-parentheses
    -Wno-deprecated-declarations
    -Wno-conversion-null
    -Wno-unused-but-set-variable
    -Wno-type-limits
    -Wno-unused-but-set-parameter
    -Wno-catch-value
    -Wno-deprecated
    -Wno-sign-compare
  >
)

# Equivalent to -I$(STM_INC) everywhere
# Add parent of repo so includes like "Mu2e-STMDAQ/..." resolve
get_filename_component(STMDAQ_REPO_PARENT "${CMAKE_SOURCE_DIR}" DIRECTORY)

target_include_directories(stmdaq_build_options INTERFACE
  "${STM_INC}"
  "${STMDAQ_REPO_PARENT}"
)

# -----------------------------------------------------------------------------
# Common link dependencies (Boost, Threads, rt, pq, etc.)
# Everything inherits these by linking stmdaq_common_deps
# -----------------------------------------------------------------------------
add_library(stmdaq_common_deps INTERFACE)

# Threads (Make often adds -pthread implicitly)
find_package(Threads REQUIRED)
target_link_libraries(stmdaq_common_deps INTERFACE Threads::Threads)

# librt (your Makefiles link -lrt often; keep it conditional)
find_library(RT_LIB rt)
if(RT_LIB)
  target_link_libraries(stmdaq_common_deps INTERFACE ${RT_LIB})
endif()

# ---- Boost ----
# Prefer system Boost (dnf install boost-devel). If BOOST_ROOT is set, treat it as an override.
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)

# Do NOT force NO_SYSTEM_PATHS; we want /usr to be searched.
# (So remove any set(Boost_NO_SYSTEM_PATHS ON).)

find_package(Boost REQUIRED COMPONENTS program_options chrono random system)

# Prefer imported targets if available
if(TARGET Boost::program_options)
  target_link_libraries(stmdaq_common_deps INTERFACE
    Boost::program_options
    Boost::chrono
    Boost::random
    Boost::system
  )
else()
  # Fallback for older FindBoost mode
  target_include_directories(stmdaq_common_deps INTERFACE ${Boost_INCLUDE_DIRS})
  target_link_libraries(stmdaq_common_deps INTERFACE ${Boost_LIBRARIES})
endif()
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries   : ${Boost_LIBRARIES}")

# Your Makefile.inc listed: program_options chrono random system
# (add more components here if needed)
find_package(Boost REQUIRED COMPONENTS program_options chrono random system)

# ---- PugiXML ----
find_package(PugiXML QUIET)

if(USE_CET)
 find_package(pugixml REQUIRED EXPORT)
 if(TARGET pugixml)
  target_link_libraries(stmdaq_common_deps INTERFACE pugixml)
 elseif(DEFINED PUGIXML_LIBRARIES)
  target_link_libraries(stmdaq_common_deps INTERFACE ${PUGIXML_LIBRARIES})
  if(DEFINED PUGIXML_INCLUDE_DIRS)
   target_include_directories(stmdaq_common_deps INTERFACE ${PUGIXML_INCLUDE_DIRS})
  endif()
 endif()
elseif(PugiXML_FOUND)
  # Depending on the package, the imported target name can vary.
  # Prefer the target if it exists; otherwise fall back to variables.
  if(TARGET PugiXML::pugixml)
    target_link_libraries(stmdaq_common_deps INTERFACE PugiXML::pugixml)
  elseif(DEFINED PUGIXML_LIBRARIES)
    target_link_libraries(stmdaq_common_deps INTERFACE ${PUGIXML_LIBRARIES})
    if(DEFINED PUGIXML_INCLUDE_DIRS)
      target_include_directories(stmdaq_common_deps INTERFACE ${PUGIXML_INCLUDE_DIRS})
    endif()
  else()
    # Fallback: locate the library directly
    find_library(PUGIXML_LIB pugixml)
    if(PUGIXML_LIB)
      target_link_libraries(stmdaq_common_deps INTERFACE ${PUGIXML_LIB})
    else()
      message(WARNING "pugixml not found; install pugixml-devel")
    endif()
  endif()
else()
  # Fallback: locate the library directly
  find_library(PUGIXML_LIB pugixml)
  if(PUGIXML_LIB)
    target_link_libraries(stmdaq_common_deps INTERFACE ${PUGIXML_LIB})
  else()
    message(WARNING "pugixml not found; install pugixml-devel")
  endif()
endif()



# ---- PostgreSQL libpq (-lpq) ----
find_package(PostgreSQL QUIET)
if(PostgreSQL_FOUND)
  target_link_libraries(stmdaq_common_deps INTERFACE PostgreSQL::PostgreSQL)
else()
  find_library(PQ_LIB pq)
  if(PQ_LIB)
    target_link_libraries(stmdaq_common_deps INTERFACE ${PQ_LIB})
  else()
    message(WARNING "libpq not found (needed for -lpq). Install libpq-dev (or provide PostgreSQL_ROOT).")
  endif()
endif()

# -----------------------------------------------------------------------------
# Python embedding/development (only link where needed)
# -----------------------------------------------------------------------------
add_library(stmdaq_python_deps INTERFACE)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
target_link_libraries(stmdaq_python_deps INTERFACE Python3::Python)
target_include_directories(stmdaq_python_deps INTERFACE ${Python3_INCLUDE_DIRS})

add_library(fftw_deps INTERFACE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW REQUIRED fftw3)

target_include_directories(fftw_deps INTERFACE ${FFTW_INCLUDE_DIRS})
target_link_libraries(fftw_deps INTERFACE ${FFTW_LIBRARIES})

# Try to get pybind11 include path from the active Python
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -c "import pybind11; print(pybind11.get_include())"
  RESULT_VARIABLE _pybind11_rc
  OUTPUT_VARIABLE _pybind11_inc
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(_pybind11_rc EQUAL 0 AND EXISTS "${_pybind11_inc}")
  message(STATUS "Found pybind11 include via Python: ${_pybind11_inc}")
  target_include_directories(stmdaq_python_deps INTERFACE "${_pybind11_inc}")
else()
  message(WARNING "Could not import pybind11 in ${Python3_EXECUTABLE}; pybind11/embed.h may be missing.")
endif()

# -----------------------------------------------------------------------------
# CACTUS / uHAL (only if CACTUS_ROOT is available)
# Makefile: -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal
# -----------------------------------------------------------------------------
add_library(stmdaq_uhal_deps INTERFACE)

if(DEFINED CACTUS_ROOT)
  if(EXISTS "${CACTUS_ROOT}/include")
    target_include_directories(stmdaq_uhal_deps INTERFACE "${CACTUS_ROOT}/include")
  endif()

  find_library(UHAL_LOG_LIB      cactus_uhal_log      PATHS "${CACTUS_ROOT}/lib" NO_DEFAULT_PATH)
  find_library(UHAL_GRAMMARS_LIB cactus_uhal_grammars PATHS "${CACTUS_ROOT}/lib" NO_DEFAULT_PATH)
  find_library(UHAL_UHAL_LIB     cactus_uhal_uhal     PATHS "${CACTUS_ROOT}/lib" NO_DEFAULT_PATH)

  if(UHAL_LOG_LIB AND UHAL_GRAMMARS_LIB AND UHAL_UHAL_LIB)
    target_link_libraries(stmdaq_uhal_deps INTERFACE
      ${UHAL_LOG_LIB}
      ${UHAL_GRAMMARS_LIB}
      ${UHAL_UHAL_LIB}
    )
  else()
    message(WARNING "CACTUS_ROOT is set but uHAL libs not fully found in ${CACTUS_ROOT}/lib")
  endif()
else()
  message(STATUS "CACTUS_ROOT not set; uHAL-dependent targets should not link stmdaq_uhal_deps.")
endif()

# -----------------------------------------------------------------------------
# Convenience: One “everything” deps target (optional)
# Many targets can just link stmdaq_all_deps if they want common+build options.
# -----------------------------------------------------------------------------
add_library(stmdaq_all_deps INTERFACE)
target_link_libraries(stmdaq_all_deps INTERFACE
  stmdaq_build_options
  stmdaq_common_deps
)

# -----------------------------------------------------------------------------
# Subdirectories (convert one-by-one; keep list matching your structure)
# -----------------------------------------------------------------------------
add_subdirectory(utils)
add_subdirectory(config)
add_subdirectory(hardware)
add_subdirectory(buffers)
add_subdirectory(debug)
add_subdirectory(processing)
#add_subdirectory(dqm)
add_subdirectory(sim)
add_subdirectory(frontend)
add_subdirectory(exes)

# -----------------------------------------------------------------------------
# Optional convenience clean target (CMake already has 'clean' via build tool)
# -----------------------------------------------------------------------------
add_custom_target(clean_all
  COMMAND ${CMAKE_COMMAND} -E rm -rf
    "${STMDAQ_BIN_DIR}" "${STMDAQ_LIB_DIR}" "${STMDAQ_OBJ_DIR}" "${STMDAQ_DEP_DIR}"
  COMMENT "Removing STMDAQ build output directories"
)
