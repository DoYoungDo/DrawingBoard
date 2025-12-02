cmake_minimum_required(VERSION 3.16)

project(DrawingBoard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_subdirectory(Components/)
add_subdirectory(Third/QHotkey/)

include_directories(BEFORE Components/)
include_directories(BEFORE Third/QHotkey/)

set(PROJECT_SOURCES
    trayicon.h trayicon.cpp
    boardprivate.h
    board.h board.cpp
    drawerprivate.h
    drawer.h drawer.cpp
    pen.h pen.cpp
    preview.h preview.cpp
    tools.h tools.cpp
    dbapplication.h dbapplication.cpp
    config.h config.cpp

)

add_executable(DrawingBoard
    ${PROJECT_SOURCES}
    res.qrc
    main.cpp
    README.md
    settingview.h settingview.cpp settingview.ui
    translator.h translator.cpp
    res/i18n.qrc
)

target_link_libraries(DrawingBoard PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(DrawingBoard PRIVATE Components)
target_link_libraries(DrawingBoard PRIVATE qhotkey)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.DrawingBoard)
endif()
set_target_properties(DrawingBoard PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS DrawingBoard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DrawingBoard)
endif()
