include("C:/Users/ASUSTUF/Desktop/SE_JOURNEY/build/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/qtui-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase;qtdeclarative")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "C:/Users/ASUSTUF/Desktop/SE_JOURNEY/build/qtui.exe"
    GENERATE_QT_CONF
)
