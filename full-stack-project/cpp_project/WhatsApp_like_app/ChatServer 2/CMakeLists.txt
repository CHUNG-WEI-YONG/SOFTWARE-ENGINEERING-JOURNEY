cmake_minimum_required(VERSION 3.15)
project(ChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_WIN32_WINNT=0x0601)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)

# Proto file
set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/message.proto)

# Generated files
set(GEN_PB_SRC ${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc)
set(GEN_PB_HDR ${CMAKE_CURRENT_BINARY_DIR}/message.pb.h)
set(GEN_GRPC_SRC ${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.cc)
set(GEN_GRPC_HDR ${CMAKE_CURRENT_BINARY_DIR}/message.grpc.pb.h)

set(PROTOC_EXE
    "C:/vcpkg/installed/x64-windows/tools/protobuf/protoc.exe")

set(GRPC_PLUGIN
    "C:/vcpkg/installed/x64-windows/tools/grpc/grpc_cpp_plugin.exe")

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

file(GLOB ALL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source_file/*.cpp
)

file(GLOB ALL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/header_file/*.h)
file(GLOB ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source_file/*.cpp)

add_executable(ChatServer
    ${ALL_SOURCES}
    ${ALL_HEADERS}
    ${GEN_PB_SRC}
    ${GEN_GRPC_SRC}
 "header_file/data.h")

target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0
)

target_link_libraries(ChatServer PRIVATE
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    JsonCpp::JsonCpp
    Threads::Threads
    unofficial::mysql-connector-cpp::connector
    ws2_32
    mswsock
    iphlpapi
    redis++::redis++
    hiredis::hiredis
)

# Copy config.ini
set(CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/config.ini)

if(EXISTS ${CONFIG_FILE})
    add_custom_command(
        TARGET ChatServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CONFIG_FILE}
            $<TARGET_FILE_DIR:ChatServer>/config.ini
    )
endif()