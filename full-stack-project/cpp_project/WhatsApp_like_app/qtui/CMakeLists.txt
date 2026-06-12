cmake_minimum_required(VERSION 3.19)
project(qtui LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

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


)

target_link_libraries(qtui
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

set_target_properties(qtui PROPERTIES QT_WINDOWS_FINAL_EXECUTABLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")

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
