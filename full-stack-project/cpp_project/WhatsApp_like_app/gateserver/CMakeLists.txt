cmake_minimum_required(VERSION 3.15)
project(WhatsApp_GateServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =================================================================
# 🥇 1. 强行开闸：一次性捞取 vcpkg 熔炼好的重型组件
# =================================================================
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)


# =================================================================
# 📡 2. 硬核物理路径焊死：gRPC + Protobuf 强行全自动双向爆破引擎！
# =================================================================
# 1. 锁死你的公共契约绝对物理路径
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../message.proto")

# 2. 精准定位自动生成的四个肌肉文件躺着的地方（当前编译二进制沙盒）
set(GEN_PB_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc")
set(GEN_PB_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.pb.h")
set(GEN_GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.cc")
set(GEN_GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.h")

# 3. 🔥【终极硬核钢钉】：直接肉身锁死 vcpkg 编译器的绝对物理路径
set(PROTOC_EXE "C:/vcpkg/installed/x64-windows/tools/protobuf/protoc.exe")
set(GRPC_PLUGIN "C:/vcpkg/installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe")

# 4. 手写全自动空降指令：无视工具链差异，强行拉起 exe 爆破代码！
add_custom_command(
    OUTPUT "${GEN_PB_SRC}" "${GEN_PB_HDR}" "${GEN_GRPC_SRC}" "${GEN_GRPC_HDR}"
    COMMAND ${PROTOC_EXE}
    ARGS --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --plugin=protoc-gen-grpc=${GRPC_PLUGIN}
         -I "${CMAKE_CURRENT_SOURCE_DIR}/.."
         "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "📡 [硬核绝对路径流水线] 正在强行爆破生成 message.pb 和 message.grpc.pb 文件..."
)

# 将生成的源码和头文件打包成变量，准备塞进 GateServer 的身体里
set(PROTO_SRCS "${GEN_PB_SRC}" "${GEN_GRPC_SRC}")
set(PROTO_HDRS "${GEN_PB_HDR}" "${GEN_GRPC_HDR}")

# =================================================================
# 📦 3. 扫描本地后端源码
# =================================================================
file(GLOB ALL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source_file/*.cpp")

# =================================================================
# 🚀 4. 总装可执行肉身
# =================================================================
add_executable(GateServer ${ALL_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS} 
    "source_file/ConfigMgr.cpp" 
    "source_file/AsioIOServicePool.cpp"  
    "source_file/RedisMjr.cpp"  
    "source_file/MysqlPool.cpp" 
    "header_file/MysqlMgr.h" 
    "source_file/MysqlMgr.cpp")

# =================================================================
# 🛠️ 5. 链接底层二进制肌肉
# =================================================================
target_link_libraries(GateServer PRIVATE 
    Threads::Threads
    ws2_32
    mswsock
    iphlpapi
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    redis++::redis++
    unofficial::mysql-connector-cpp::connector
)

# =================================================================
# 📁 6. 头文件雷达扫描路径（新增生成的 BINARY_DIR 视觉通道）
# =================================================================
target_include_directories(
    GateServer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file
    C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR} # 👈 确保 C++ 源码能抓到刚生成的两个 .h
    ${CMAKE_BINARY_DIR}
)

# =================================================================
# 🛡️ 7. 编译器特化防御
# =================================================================
if(MSVC)    
    target_compile_options(GateServer PRIVATE "/Zc:__cplusplus")
endif()

# =================================================================
# 📦 8. config.ini 全自动物理空降流水线
# =================================================================
set(TargetConfig "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
set(OutputDir "${CMAKE_BINARY_DIR}")
if(EXISTS "${TargetConfig}")    
    add_custom_command(
        TARGET GateServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TargetConfig}"
        "${OutputDir}/config.ini"
    )
endif()

add_custom_command(TARGET GateServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/config.ini
    $<TARGET_FILE_DIR:GateServer>/config.ini
)

set(TargetConfig "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
set(OutputDir "${CMAKE_BINARY_DIR}") # 锁死 Ninja 编译输出沙盒目录

if(EXISTS "${TargetConfig}")
    add_custom_command(
        TARGET GateServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TargetConfig}"
        "${OutputDir}/config.ini"
        COMMENT "📡 [CMake] 正在全自动同步 config.ini 到可执行文件输出目录..."
    )
endif()