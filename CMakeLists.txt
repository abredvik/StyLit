cmake_minimum_required(VERSION 3.5)

project(StyLit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(StyLit
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        rgba.h
        util.h
        stylit.h stylit.cpp
        patchmatch.h patchmatch.cpp
        util.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET StyLit APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(StyLit SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(StyLit
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(StyLit PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.StyLit)
endif()
set_target_properties(StyLit PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS StyLit
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

file(GLOB EIGEN_DIR_CONTENTS ${CMAKE_CURRENT_LIST_DIR}/Eigen/*)
list(LENGTH EIGEN_DIR_CONTENTS EIGEN_DIR_SIZE)
if(EIGEN_DIR_SIZE EQUAL 0)
    message(FATAL_ERROR "Eigen dependency not pulled, please run `git submodule update --init --recursive`")
endif()

# This allows you to `#include <Eigen/...>`
target_include_directories(${PROJECT_NAME} PRIVATE
    Eigen
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(StyLit)
endif()
