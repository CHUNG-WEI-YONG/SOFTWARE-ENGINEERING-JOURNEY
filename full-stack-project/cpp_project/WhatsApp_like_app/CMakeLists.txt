cmake_minimum_required(VERSION 3.15)
project(WhatsApp_like_app)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(redis++ CONFIG REQUIRED)

# 2. 📡 全局广播：把公共的 message.proto 绝对路径宣告给所有子目录
set(GLOBAL_PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/message.proto" CACHE INTERNAL "Global Proto File Path")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

add_subdirectory(GateServer)
add_subdirectory(qtui)