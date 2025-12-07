cmake_minimum_required(VERSION 3.16)
project(UITPad VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Add include directories so the compiler can find header files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Header Files")

set(PROJECT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/Source Files/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source Files/MainWindow.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Header Files/MainWindow.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Form Files/mainwindow.ui"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source Files/Document.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Header Files/Document.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source Files/TextEditor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Header Files/TextEditor.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source Files/SpellChecker.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Header Files/SpellChecker.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source Files/Settings.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Header Files/Settings.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources Files/resources.qrc"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source Files/highlighter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Header Files/highlighter.h"
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(UITPad
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(UITPad SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(UITPad
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(UITPad PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Configuration MacOS / iOS
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.UITPad)
endif()

set_target_properties(UITPad PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS UITPad
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(UITPad)
endif()