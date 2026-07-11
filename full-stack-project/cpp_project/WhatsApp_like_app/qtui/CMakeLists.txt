cmake_minimum_required(VERSION 3.19)
project(qtui LANGUAGES CXX)

# 1. 强制锁死 C++20 战备标准，确保与后端完美的时空对齐
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. 开启 Qt 自动化核心机甲（⚠️ 调整至 find_package 之前，确保元对象编译万无一失）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 2. 捞取 Qt6 前端核心组件 + vcpkg 的 JsonCpp 组件
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network QuickWidgets)
#find_package(JsonCpp CONFIG REQUIRED) # 👈 【新增】打捞 vcpkg 熔炼好的 Json 核心资产

# 4. 🏭 告诉 CMake：你之前大炼钢铁产出的 Boost 静态库（.a/.lib）住在哪个军火库里
link_directories("C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0/stage/lib")

# ------------------------------------------------------------------
qt_standard_project_setup()

# 5. 总装可执行目标文件（纯粹的本地源文件大军）
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
    timerbtn.h timerbtn.cpp
    clickedlabel.h clickedlabel.cpp
    resetdialog.h resetdialog.cpp resetdialog.ui
    tcpmgr.h tcpmgr.cpp
    usermgr.h usermgr.cpp
    chatdialog.h chatdialog.cpp chatdialog.ui
    clickedbutton.h clickedbutton.cpp
    customizeedit.h customizeedit.cpp
    chatuserlist.h chatuserlist.cpp

    chatuserwid.h chatuserwid.cpp chatuserwid.ui
    listitembase.h listitembase.cpp
    loadingdialog.h loadingdialog.cpp loadingdialog.ui
    chatpage.h chatpage.cpp chatpage.ui
    qmlchatpage.h qmlchatpage.cpp qmlchatpage.ui
    chatbridge.h chatbridge.cpp
)

# 6. 🛰️【规范化现代 CMake】管理头文件包含路径
target_include_directories(qtui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/header_file
    ${CMAKE_CURRENT_SOURCE_DIR}/include

    # 👈 【新增】强制保底 vcpkg 包含路径（防止本地 Json 头文件标红）
    #"C:/vcpkg/installed/x64-windows/include"

    # 锁死你手动下载的最干净的 Boost 1.91.0 路径
    #"C:/Users/ASUSTUF/Downloads/boost_1_91_0/boost_1_91_0"
)

# 7. ⚡【终极链接缝合】把 Qt 的核心内脏、vcpkg 资产和 Boost 静态库直接焊死！
target_link_libraries(qtui
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Network
    Qt::QuickWidgets

    # 👈 【核心修复 1】：真正将舞台（stage/lib）下编译出的 Boost 静态库灌入程序
    # Windows 下通常只需要链接 libboost_system 即可，Asio 绝大部分是 Header-only
    #libboost_system

    # 👈 【核心修复 2】：将 Json 编译肌肉无缝对接到前端
    #JsonCpp::JsonCpp
)

# 8. 📡 全自动资产拷贝: config.ini -> 输出 bin 目录
set(TargetConfig "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
set(OutputDir "${CMAKE_BINARY_DIR}")

add_custom_command(
    TARGET qtui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${TargetConfig}"
    "${OutputDir}/config.ini"
    COMMENT "📡 [CMake] 正在执行全自动资产拷贝: config.ini -> 输出 bin 目录"
)

# 9. 挂载 Windows 原生软件图标
set_target_properties(qtui PROPERTIES QT_WINDOWS_FINAL_EXECUTABLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")

# 10. 工业级打包分发流水线（防止在别人电脑上双击闪退）
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