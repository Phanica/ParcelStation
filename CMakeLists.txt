# 在文件开头添加
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)
# 在set(CMAKE_RUNTIME_OUTPUT_DIRECTORY)后添加以下设置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 确保在project()之后设置
project(ParcelStation)

set(CMAKE_C_STANDARD 11)

# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 查找需要的库
find_package(Threads REQUIRED)

# 服务器端配置
add_executable(server
    server/main.c
    server/cJSON.c
)
target_include_directories(server PRIVATE server)
target_link_libraries(server ws2_32)

# 客户端配置
add_subdirectory(client)
# add_executable(postman_client
#     client/postman_client.c
#     client/common.c
#     client/main.c
# )
# target_include_directories(postman_client PRIVATE client)
# target_link_libraries(postman_client ws2_32)

# 可选：安装规则
install(TARGETS server postman_client
    RUNTIME DESTINATION bin
)

# 复制数据文件
# configure_file(server/data.json ${CMAKE_BINARY_DIR}/bin/data.json COPYONLY)
