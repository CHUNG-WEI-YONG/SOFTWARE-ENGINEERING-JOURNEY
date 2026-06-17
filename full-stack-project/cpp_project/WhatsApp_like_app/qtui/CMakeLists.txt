cmake_minimum_required(VERSION 3.19)
#set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe")
#set(CMAKE_C_COMPILER   "C:/Qt/Tools/mingw1310_64/bin/gcc.exe")
project(qtui LANGUAGES CXX)

# 1. 强制锁死 C++20 战备标准，确保与你后端的全局单例、Asio 状态机完美的时空对齐
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 捞取 Qt6 前端核心物理资产
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

# 3. 开启 Qt 自动化核心机甲
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 4. 🛰️ 引入 Boost 1.91.0 编译期头文件骨架
include_directories("C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0")

# 5. 🏭 告诉 CMake：你刚刚大炼钢产出的 64位静态库二进制文件住在哪个军火库里
link_directories("C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0/stage/lib")

# ------------------------------------------------------------------
qt_standard_project_setup()

# 6. 总装可执行目标文件，吃下所有的本地源文件拼图
qt_add_executable(qtui
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    icon.ico
    logindialog.h logindialog.cpp logindialog.ui
    resources.qrc
    signindialog.ui
    signindialog.h
    signindialog.cpp
    style/stylesheet.qss
    global.h
    global.cpp
    singleton.h
    httpmgr.h httpmgr.cpp
    config.ini
)



# 1. TargetConfig = $${PWD}/config.ini
set(TargetConfig "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")

# 2. OutputDir = $${OUT_PWD}/$${DESTDIR}
# (注：CMake 不需要像旧版那样痛苦地用 $$replace 替换斜杠，底层全自动处理 Windows 盘符)
set(OutputDir "${CMAKE_BINARY_DIR}")

# 3. QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\"
# (注：请把这里的 "GateServer" 或 "qtui" 替换为你 add_executable() 里定义的目标程序名字)
add_custom_command(
    TARGET qtui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${TargetConfig}"
    "${OutputDir}/config.ini"
    COMMENT "📡 [CMake] 正在执行全自动资产拷贝: config.ini -> 输出 bin 目录"
)

set(QT_PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../message.proto")

protobuf_generate_cpp(QT_PROTO_SRCS QT_PROTO_HDRS ${QT_PROTO_FILE})



# 7. ⚡【核心修正点】终极总装电缆缝合大闸！
# 必须在这里，明确命令编译器把 Qt 的内脏和 Boost 的物理二进制同时焊死到 qtui 身上！
target_link_libraries(qtui
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        # 🥇 物理救赎：直接把你在 stage/lib 里大炼钢铁产出的具体静态库名字拍在这里！
        # 如果你使用的是 MinGW(gcc)，填入 boost_system 
        # 如果你使用的是 MSVC(cl.exe)，它会自动根据名字识别，这里填入 boost_system 即可
)

# 8. 挂载 Windows 原生软件图标
set_target_properties(qtui PROPERTIES QT_WINDOWS_FINAL_EXECUTABLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")

# 9. 工业级打包分发流水线（用于把 Qt 各种乱七八糟的 .dll 自动化打包到一起，防止在别人电脑上闪退）
install(TARGETS qtui
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET qtui
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})


target_include_directories(
    qtui
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file
    C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)