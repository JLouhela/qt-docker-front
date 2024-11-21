cmake_minimum_required(VERSION 3.16)

project(qt-docker-front VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appqt-docker-front
    main.cpp
)

qt_add_qml_module(appqt-docker-front
    URI QtDockerFront
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES dockerbackend.h dockerbackend.cpp
        SOURCES
        SOURCES dockerapi.h dockerapi.cpp
        SOURCES overviewupdateworker.h overviewupdateworker.cpp
        SOURCES container.h
        SOURCES containerupdateworker.h containerupdateworker.cpp
        SOURCES containerinfo.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an

# explicit, fixed bundle identifier manually though.
set_target_properties(appqt-docker-front PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appqt-docker-front
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appqt-docker-front
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appqt-docker-front
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
