find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


add_library(Components
    capabilitybutton.h capabilitybutton.cpp
)

target_link_libraries(Components PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
