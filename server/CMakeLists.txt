add_executable(server
    binary_server.c
)
target_link_libraries(server ws2_32)
