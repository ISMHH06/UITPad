cmake_minimum_required(VERSION 3.16)

project(UITPad VERSION 0.1 LANGUAGES CXX)

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
        mainwindow.ui
        document.cpp
        document.h
        texteditor.cpp
        texteditor.h
        spellchecker.cpp
        spellchecker.h
        settings.cpp
        settings.h
        resources.qrc
        highlighter.cpp
        highlighter.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(UITPad
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        settings.h settings.cpp
        highlighter.h highlighter.cpp
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
