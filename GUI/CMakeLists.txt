project(ultralightwebcursor-gui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network)

set(GUI_LINK_LIBS
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
)

# The KWin variant talks to the effect over D-Bus, the standalone variants use
# a local socket instead. Only link Qt D-Bus where it is actually used.
if(DETECTED_DESKTOP_ENVIRONMENT MATCHES "KDE")
    find_package(Qt6 6.6 REQUIRED COMPONENTS DBus)
    list(APPEND GUI_LINK_LIBS Qt6::DBus)
endif()

qt_add_executable(ultralightwebcursor-gui main.cpp)

qt_add_qml_module(ultralightwebcursor-gui
    URI WebCursorGui
    VERSION 1.0
    QML_FILES
        Main.qml
        Uistaff.qml
)

target_sources(ultralightwebcursor-gui PRIVATE
    SettingsBackend.cpp
    SettingsBackend.hpp
)

target_include_directories(ultralightwebcursor-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/header
    ${CMAKE_SOURCE_DIR}/src/lib
)

target_link_libraries(ultralightwebcursor-gui PRIVATE
    ${GUI_LINK_LIBS}
    ultralightwebcursor_core
)

install(TARGETS ultralightwebcursor-gui RUNTIME DESTINATION bin)
