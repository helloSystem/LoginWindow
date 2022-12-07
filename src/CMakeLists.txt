cmake_minimum_required(VERSION 3.5)

project(LoginManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Network is required for the login manager to show the hostname
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# Needed to draw the desktop picture on all screens
find_package(Qt5X11Extras REQUIRED)

# Include the PAM header files
include_directories(/usr/include/security)

set(PROJECT_SOURCES
        main.cpp
        loginwindow.cpp
        loginwindow.h
        )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(LoginManager
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            )

else()
    add_executable(LoginManager
            ${PROJECT_SOURCES}
            )
endif()

target_link_libraries(LoginManager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::X11Extras pam)

set_target_properties(LoginManager PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        )

install(TARGETS LoginManager
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LoginManager)
endif()
