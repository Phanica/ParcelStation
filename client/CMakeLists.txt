# 客户端公共库
add_library(client_common
    common.c
    main.c
)
target_include_directories(client_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 各客户端程序
add_executable(postman_client
    postman_client.c
)
target_link_libraries(postman_client client_common ws2_32)

add_executable(manager_client
    manager_client.c
)
target_link_libraries(manager_client client_common ws2_32)

add_executable(user_client
    user_client.c
)
target_link_libraries(user_client client_common ws2_32)
