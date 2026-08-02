cmake_minimum_required(VERSION 3.15)
project(ChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =================================================================
# 🥇 1. 查找 vcpkg 组件 (添加 hiredis 显式支持与 JsonCpp 目标匹配)
# =================================================================
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED) # 👈 显式引入 hiredis，解决 redisFree 底层链接符号缺失

# =================================================================
# 📡 2. gRPC + Protobuf 全自动生成代码
# =================================================================
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/message.proto")

set(GEN_PB_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc")
set(GEN_PB_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.pb.h")
set(GEN_GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.cc")
set(GEN_GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.h")

set(PROTO_SRCS "${GEN_PB_SRC}" "${GEN_GRPC_SRC}")
set(PROTO_HDRS "${GEN_PB_HDR}" "${GEN_GRPC_HDR}")

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND protobuf::protoc
    ARGS --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I "${CMAKE_CURRENT_SOURCE_DIR}"
         "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "📡 [自动化流水线] 正在生成 message.pb 和 message.grpc.pb 文件..."
)

# =================================================================
# 📦 3. 扫描本地源码
# =================================================================
file(GLOB ALL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source_file/*.cpp")
file(GLOB ALL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/header_file/*.h")

# =================================================================
# 🚀 4. 构建可执行文件
# =================================================================
add_executable(ChatServer
    ${ALL_SOURCES}
    ${ALL_HEADERS}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# =================================================================
# 📁 5. 头文件搜索路径
# =================================================================
target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    "C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0"
    "C:/Users/ASUSTUF/Downloads/boost_1_91_0"
)

# =================================================================
# 🛠️ 6. 链接二进制库 (解决 14 个 LNK2019 unresolved externals)
# =================================================================
target_link_libraries(ChatServer PRIVATE
    Threads::Threads
    ws2_32
    mswsock
    iphlpapi
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    unofficial::mysql-connector-cpp::connector
    JsonCpp::JsonCpp
    redis++::redis++
    hiredis::hiredis # 👈 确保补上 hiredis
)

# =================================================================
# 🛡️ 7. Windows/MSVC 平台特化防御宏与选项
# =================================================================
if(MSVC)
    # 强制将 Windows API 版本提升至 Windows 10 (0x0A00)，解决高版本 Boost/Asio 警告或宏冲突
    target_compile_definitions(ChatServer PRIVATE 
        _WIN32_WINNT=0x0A00
        _SILENCE_ASAN_C_RUNTIME_WARNINGS
    )
    target_compile_options(ChatServer PRIVATE 
        "/Zc:__cplusplus"
        "/wd4996"  
        "/wd4251"  
        "/wd4267"  
        "/wd4244"
    )
endif()

# =================================================================
# 📦 8. 自动同步配置文件 config.ini
# =================================================================
set(CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
if(EXISTS "${CONFIG_FILE}")
    add_custom_command(
        TARGET ChatServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CONFIG_FILE}"
            "$<TARGET_FILE_DIR:ChatServer>/config.ini"
    )
endif()