cmake_minimum_required(VERSION 3.15)
project(WhatsApp_StatusServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =================================================================
# 🥇 1. 一次性捞取 vcpkg 熔炼好的重型组件
# =================================================================
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(JsonCpp CONFIG REQUIRED)

# =================================================================
# 📡 2. gRPC + Protobuf 全自动生成双向爆破引擎
# =================================================================
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../message.proto")

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
         -I "${CMAKE_CURRENT_SOURCE_DIR}/.."
         "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "📡 [自动化流水线] 正在通过 vcpkg 生成 message.pb 和 message.grpc.pb 文件..."
)

# =================================================================
# 📦 3. 扫描本地后端源码
# =================================================================
file(GLOB ALL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source_file/*.cpp")
file(GLOB ALL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/header_file/*.h")

# =================================================================
# 🚀 4. 总装可执行肉身
# =================================================================
add_executable(StatusServer 
    ${ALL_SOURCES} 
    ${ALL_HEADERS}
    ${PROTO_SRCS} 
    ${PROTO_HDRS}
 "header_file/StatusServiceImpl.h" "source_file/StatusServiceImpl.cpp" "header_file/ChatGrpcClient.h" "source_file/ChatGrpcClient.cpp")

# =================================================================
# 🛠️ 5. 链接底层二进制肌肉
# =================================================================
target_link_libraries(StatusServer PRIVATE 
    Threads::Threads
    ws2_32
    mswsock
    iphlpapi
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    redis++::redis++          
    unofficial::mysql-connector-cpp::connector
    JsonCpp::JsonCpp
)

# =================================================================
# 📁 6. 头文件雷达扫描路径（新增 vcpkg 导出的 Redis 头文件安全通道）
# =================================================================
target_include_directories(StatusServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR} 
    
    # 👈 核心防御：强行提取 vcpkg 缓存中编译好的 hiredis 和 redis++ 的绝对头文件路径
    ${hiredis_INCLUDE_DIR}
    ${redis++_INCLUDE_DIR}
    "C:/vcpkg/installed/x64-windows/include" # 强制保底视觉通道

    "C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0/libs/uuid/include"
    "C:/Users/ASUSTUF/Downloads/boost_1_91_0/libs/uuid/include"
    
    # 多重 Boost 路径轰炸
    "C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0"
    "C:/Users/ASUSTUF/Downloads/boost_1_91_0"
    "C:/Users/ASUSTUF/Downloads"
)

# =================================================================
# 🛡️ 7. 编译器特化防御
# =================================================================
if(MSVC)
    target_compile_definitions(StatusServer PRIVATE _WIN32_WINNT=0x0A00)
    target_compile_definitions(StatusServer PRIVATE _SILENCE_ASAN_C_RUNTIME_WARNINGS)
    
    target_compile_options(StatusServer PRIVATE 
        "/Zc:__cplusplus"
        "/wd4996"  
        "/wd4251"  
        "/wd4267"  
        "/wd4244"
    )
endif()

# =================================================================
# 📦 8. config.ini 全自动同步
# =================================================================
set(TargetConfig "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
if(EXISTS "${TargetConfig}")
    add_custom_command(
        TARGET StatusServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TargetConfig}"
        "$<TARGET_FILE_DIR:StatusServer>/config.ini"
    )
endif()