option(USE_CET "Build using CET instead of plain CMake" OFF)

if(USE_CET)
  message(STATUS "Build mode: CET")
else()
  message(STATUS "Build mode: standard CMake")
endif()
