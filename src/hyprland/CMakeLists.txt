project(ultralightwebcursor-hyprland)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
pkg_check_modules(HYPRUTILS REQUIRED IMPORTED_TARGET hyprutils)

add_executable(ultralightwebcursor_hyprland
    HyprlandApp.cpp
    StandaloneCursorEffect.cpp
    StandaloneCursorEffect.hpp
    PlatformMouseProvider.cpp
    PlatformMouseProvider.hpp
    ../kwin-plasma/UltralightHtmlEffect.cpp
    $<TARGET_OBJECTS:ultralightwebcursor_core>
)

target_include_directories(ultralightwebcursor_hyprland PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/header
    ${CMAKE_SOURCE_DIR}/src/lib
)

target_link_libraries(ultralightwebcursor_hyprland PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Ultralight::SDK
    PkgConfig::HYPRUTILS
    X11::X11
)

install(TARGETS ultralightwebcursor_hyprland RUNTIME DESTINATION bin)
