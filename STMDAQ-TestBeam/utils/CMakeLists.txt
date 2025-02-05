# Minimum CMake version
#
# Check for STMDAQ_ROOT environment variable
# if (NOT DEFINED ENV{STMDAQ_ROOT})
#     message(FATAL_ERROR "STMDAQ_ROOT not set, must source setup.sh")
# endif()

# set(STMDAQ_ROOT $ENV{STMDAQ_ROOT})

# # Compiler warning flags
# set(WARNING_FLAGS 
#     -Wall 
#     -Wformat=2 
#     -Wno-format-nonliteral 
#     -Wno-strict-aliasing 
#     -Wuninitialized 
#     -Wno-unused-function 
#     -Wextra 
#     -Wno-unused-parameter 
#     -Wno-missing-field-initializers 
#     -Wno-pointer-arith 
#     -Wno-unused-variable 
#     -Wno-unused-local-typedefs 
#     -Wno-parentheses 
#     -Wno-deprecated-declarations 
#     -Wno-conversion-null 
#     -Wno-unused-but-set-variable 
#     -Wno-type-limits 
#     -Wno-unused-but-set-parameter 
#     -Wno-catch-value 
#     -fPIC 
#     -Wno-deprecated 
#     -Wno-sign-compare
# )

# # Compiler optimization flags (adapt these as needed)
# set(CCOPTS -O3)
# 
# # System options
# set(SYSOPTS -DRMEM_MAX=${RMEM_MAX} -DWMEM_MAX=${WMEM_MAX})
# 
# # Determine database usage
# if (USE_SQL)
#     set(INCLUDE_CC 
#         -I${BOOST_INC} 
#         -I${PQXX_INC} 
#         -I${STM_INC}
#     )
#     set(LIB_CC 
#         ziggurat.cc 
#         UDPsocket.cc 
#         queue.cc 
#         EnvVars.cc 
#         dbase.cc 
#         BinaryFile.cc 
#         dataVars.cc 
#         Random.cc 
#         Logger.cc
#     )
# else()
#     set(INCLUDE_CC 
#         -I${BOOST_INC} 
#         -I${STM_INC}
#     )
#     set(LIB_CC 
#         ziggurat.cc 
#         UDPsocket.cc 
#         queue.cc 
#         queue_write.cc 
#         EnvVars.cc 
#         BinaryFile.cc 
#         dataVars.cc 
#         Random.cc 
#         Logger.cc
#     )
# endif()

#------------------------------------------------------------------------------
# build common library
#------------------------------------------------------------------------------
cet_make_library(LIBRARY_NAME otsdaq-mu2e-stm::utils
                 SOURCE
        ziggurat.cc 
        UDPsocket.cc 
        queue.cc 
        queue_write.cc 
        EnvVars.cc 
        dataVars.cc 
        Random.cc 
        Logger.cc
	         LIBRARIES PUBLIC 
)


# # Object and library files
# set(LIB_OBJ "")
# foreach(SRC ${LIB_CC})
#     string(REPLACE ".cc" ".o" OBJ_FILE ${SRC})
#     list(APPEND LIB_OBJ "${CMAKE_BINARY_DIR}/obj/${OBJ_FILE}")
# endforeach()
# set(LIB ${CMAKE_BINARY_DIR}/lib/libUtils.so)
# 
# # Include directories
# include_directories(${BOOST_INC} ${STM_INC} ${PQXX_INC})
# 
# # Create the library
# add_library(UtilsLibrary SHARED ${LIB_CC})
# target_compile_options(UtilsLibrary PRIVATE ${CCOPTS} ${WARNING_FLAGS} ${SYSOPTS} ${INCLUDE_CC})
# target_link_libraries(UtilsLibrary rt)
# 
# Custom clean target
# add_custom_target(clean COMMAND ${CMAKE_COMMAND} -E remove ${LIB_OBJ} ${DEPENDS})

install_headers()
install_source()
