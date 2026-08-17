cmake_minimum_required(VERSION 3.15)
project(ChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_WIN32_WINNT=0x0601)

# 1. 查找第三方依赖包
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system) # 引入 vcpkg 安装的 Boost

# 2. Proto 文件与生成路径
set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/message.proto)

set(GEN_PB_SRC ${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc)
set(GEN_PB_HDR ${CMAKE_CURRENT_BINARY_DIR}/message.pb.h)
set(GEN_GRPC_SRC ${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.cc)
set(GEN_GRPC_HDR ${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.h)

# 从 vcpkg 工具链路径调用 protoc 与插件
set(PROTOC_EXE "C:/vcpkg/installed/x64-windows/tools/protobuf/protoc.exe")
set(GRPC_PLUGIN "C:/vcpkg/installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe")

add_custom_command(
    OUTPUT
        ${GEN_PB_SRC}
        ${GEN_PB_HDR}
        ${GEN_GRPC_SRC}
        ${GEN_GRPC_HDR}
    COMMAND ${PROTOC_EXE}
    ARGS
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
        --plugin=protoc-gen-grpc=${GRPC_PLUGIN}
        -I=${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf and gRPC files"
)

# 3. 收集源文件与头文件
file(GLOB ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source_file/*.cpp)
file(GLOB ALL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/header_file/*.h)

# 4. 生成可执行程序 Target
add_executable(ChatServer
    ${ALL_SOURCES}
    ${ALL_HEADERS}
    ${GEN_PB_SRC}
    ${GEN_GRPC_SRC}
)

# 5. 头文件搜索目录 (已彻底移除 Downloads 硬编码路径)
target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 6. 链接库
target_link_libraries(ChatServer PRIVATE
    Boost::headers
    Boost::system
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    JsonCpp::JsonCpp
    Threads::Threads
    unofficial::mysql-connector-cpp::connector
    redis++::redis++
    hiredis::hiredis
    ws2_32
    mswsock
    iphlpapi
)

# 7. 自动拷贝 config.ini 到输出二进制目录
set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/config.ini)
if(EXISTS ${CONFIG_FILE})
    add_custom_command(
        TARGET ChatServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CONFIG_FILE}
            $<TARGET_FILE_DIR:ChatServer>/config.ini
    )
endif()