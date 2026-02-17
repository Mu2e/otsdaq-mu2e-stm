cmake_minimum_required(VERSION 3.0...3.14)
project (PlotFun)
set( CMAKE_CXX_STANDARD 11 )

set ( INC_PATH $ENV{MIDASSYS}/src # contains midas.cxx mfe.cxx 
      	       $ENV{MIDASSYS}/include # contains midas.c mfe.h midas.h
	       $ENV{MIDASSYS}/mxml # contains strlcpy.cxx strlcpy.h mxml.cxx mxml.h
	       $ENV{MIDAS_LIB} # contains libmidas.a
	       $ENV{MU2EDAQ_ROOT}/deps/ipbus-software/extern/boost/boost_1_53_0/libs/iostreams/src # boost libraries
	       $ENV{MU2EDAQ_ROOT}/deps/include # zmq.h zmq.hpp
	       $ENV{CONDA_PREFIX}/envs/mu2estm/include
)
set(CMAKE_BUILD_TYPE Debug)

if (${CMAKE_SYSTEM_NAME} MATCHES Linux)
   set( LIBS pthread util rt z )
   message(STATUS "Libraries set correctly")
else()
   message(FATAL_ERROR "Working in unfamiliar OS, aborting.")
endif()

find_package(Boost REQUIRED QUIET COMPONENTS system filesystem regex thread chrono)
find_package(ZLIB REQUIRED)

add_executable(extract_HPGe_emulator_data midas_to_csv.cc)
target_include_directories(extract_HPGe_emulator_data PRIVATE ${INC_PATH})
target_link_directories(extract_HPGe_emulator_data PRIVATE ${INC_PATH})
target_link_libraries(extract_HPGe_emulator_data PRIVATE midas ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBS})
target_compile_options(extract_HPGe_emulator_data PRIVATE -DHAVE_ZLIB)

add_executable(extract_HPGe_data midas_to_csv2.cc)
target_include_directories(extract_HPGe_data PRIVATE ${INC_PATH})
target_link_directories(extract_HPGe_data PRIVATE ${INC_PATH})
target_link_libraries(extract_HPGe_data PRIVATE midas ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBS})
target_compile_options(extract_HPGe_data PRIVATE -DHAVE_ZLIB)

#[[
add_executable(connect_to_experiment socket.cc)
target_include_directories(connect_to_experiment PRIVATE ${INC_PATH})
target_link_directories(connect_to_experiment PRIVATE ${INC_PATH})
target_link_libraries(connect_to_experiment PRIVATE midas ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBS})
target_compile_options(connect_to_experiment PRIVATE -DHAVE_ZLIB)
]]
