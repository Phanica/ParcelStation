add_library(cJSON
    cJSON.c
)
target_include_directories(cJSON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(server
    main.c
)
target_link_libraries(server cJSON ws2_32)
