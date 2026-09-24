

# declare targets
add_executable( test_sockets test_sockets.cc )

# link
target_link_libraries( test_sockets PRIVATE stm )
