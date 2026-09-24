# ======================================================================
#  otsdaq_mu2e_stm main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/otsdaq-mu2e-stm/ups/setup_for_development
#  buildtool
# ======================================================================

cmake_minimum_required (VERSION 3.19 FATAL_ERROR)

find_package(cetmodules 3.16.00 REQUIRED)

project(otsdaq-mu2e-stm VERSION 5.02.01)

include(CetCMakeEnv)
cet_cmake_env()
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

cet_set_compiler_flags(DIAGS VIGILANT
 WERROR
 NO_UNDEFINED
 EXTRA_FLAGS -pedantic -Wno-unused-parameter
 )


#cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_package(otsdaq-mu2e 1.02.00 REQUIRED)

#exeption stack trace
if(DEFINED ENV{USE_EXCEPTIONSTACKTRACE})
	if($ENV{USE_EXCEPTIONSTACKTRACE} GREATER 0)
		message("!!! Exception StackTrace Activated!!!")
		add_definitions(-D EXCEPTIONSTACKTRACE)
		set( EXCEPTIONSTACKTRACE_LIB  artdaq-core_Utilities_ExceptionStackTrace )
		add_compile_options( -rdynamic -Wno-unused-command-line-argument )
		add_link_options( -rdynamic )
	endif()
endif()

 # XDAQ Extra setup
include_directories($ENV{XDAQ_ROOT}/include/linux $ENV{XDAQ_ROOT}/include)
link_directories($ENV{XDAQ_ROOT}/lib)

# these are minimum required versions, not the actual product versions
find_package(otsdaq-mu2e 1.02.00 REQUIRED)

include(BuildPlugins)

# Main STM DAQ
add_subdirectory(Mu2e-STMDAQ)

# Code
add_subdirectory(otsdaq-mu2e-stm)

# Useful scripts
#add_subdirectory(tools)

# Test Programs
#add_subdirectory(test)

# Documentation (to comment out if not needed DOXYGEN locally)
if( $ENV{OTS_DOXY} MATCHES "DOIT" )
	add_subdirectory(doc)
endif()

cet_cmake_config()
