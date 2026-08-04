cmake_minimum_required(VERSION 3.15)
project(WhatsApp_ChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_WIN32_WINNT=0x0601)

# 1. Find Dependencies
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(jsoncpp REQUIRED)

# 2. Path Setup for .proto located outside (same level as ChatServer folder)
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(PROTO_FILE "${PROTO_DIR}/message.proto")

set(GEN_PB_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc")
set(GEN_PB_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.pb.h")
set(GEN_GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.cc")
set(GEN_GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.h")

set(PROTOC_EXE "C:/vcpkg/installed/x64-windows/tools/protobuf/protoc.exe")
set(GRPC_PLUGIN "C:/vcpkg/installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe")

# 3. Custom Command to generate files in the binary build directory
add_custom_command(
    OUTPUT "${GEN_PB_SRC}" "${GEN_PB_HDR}" "${GEN_GRPC_SRC}" "${GEN_GRPC_HDR}"
    COMMAND ${PROTOC_EXE}
    ARGS --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
         --grpc_out="${CMAKE_CURRENT_BINARY_DIR}"
         --plugin=protoc-gen-grpc="${GRPC_PLUGIN}"
         -I "${PROTO_DIR}"
         "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "📡 Generating Protobuf and gRPC files from ${PROTO_FILE}..."
)

# 4. Gather Source Files
file(GLOB ALL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source_file/*.cpp")

# 5. Build Executable (include generated .cc files in build)
add_executable(ChatServer 
    ${ALL_SOURCES}
    "${GEN_PB_SRC}"
    "${GEN_GRPC_SRC}"
)

# 6. Include Directories (Crucial for "cannot open source file")
target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}              # 👈 First priority: generated message.pb.h / message.grpc.pb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file # Your header directory
    ${CMAKE_CURRENT_SOURCE_DIR}/include     # Your include directory
    C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0
)

# 7. MSVC Specific Flags
if(MSVC)
    target_compile_options(ChatServer PRIVATE "/Zc:__cplusplus")
endif()

# 8. Link Libraries
target_link_libraries(ChatServer PRIVATE 
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    JsonCpp::JsonCpp
    Threads::Threads
    ws2_32
    mswsock
    iphlpapi
    unofficial::mysql-connector-cpp::connector
)

# 9. Sync config.ini Post-Build
set(TargetConfig "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
if(EXISTS "${TargetConfig}")
    add_custom_command(
        TARGET ChatServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TargetConfig}"
        "$<TARGET_FILE_DIR:ChatServer>/config.ini"
        COMMENT "📡 Syncing config.ini..."
    )
endif()