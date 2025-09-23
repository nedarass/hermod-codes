cmake_minimum_required(VERSION 3.16)

project(desktop VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appdesktop
    main.cpp
)

set(QT_QML_GENERATE_QMLLS_INI ON)

qt_add_qml_module(appdesktop
    URI desktop
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES
        QML_FILES ui/views/CustomButton.qml
        QML_FILES ui/views/TitleBarRoundButton.qml
        RESOURCES images/hyperloop_icon.png images/hyperloop_icon_inverted.png images/logo_app.png images/logo_app_inverted.png images/logo_team_inverted.png
        QML_FILES ui/pages/ConnectPage.qml
        QML_FILES ui/pages/EntryPage.qml
        QML_FILES ui/views/CustomConnectionProgressBar.qml
        QML_FILES ui/views/NavigationPillList.qml
        QML_FILES ui/pages/DashboardPage.qml
        RESOURCES images/logo_app_inverted_tp.png
        RESOURCES images/hyperloop_icon_tp.png
        QML_FILES ui/pages/MonitoringPage.qml
        SOURCES
        RESOURCES
QML_FILES
QML_FILES ui/views/NavigationMediator.qml
QML_FILES ui/pages/ManuelControlPage.qml
QML_FILES ui/pages/HermodPulsePage.qml
SOURCES include/udp_listener.h
SOURCES src/udp_listener.cpp
SOURCES include/tcp_client.h
SOURCES src/tcp_client.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appdesktop PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appdesktop
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appdesktop
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)

include_directories(desktop include)

install(TARGETS appdesktop
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
