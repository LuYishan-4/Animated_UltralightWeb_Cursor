project(ultralightwebcursor-gnome)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(ultralightwebcursor_gnome
    GnomeApp.cpp
    StandaloneCursorEffect.cpp
    StandaloneCursorEffect.hpp
    PlatformMouseProvider.cpp
    PlatformMouseProvider.hpp
    $<TARGET_OBJECTS:ultralightwebcursor_core>
)

target_include_directories(ultralightwebcursor_gnome PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/header
    ${CMAKE_SOURCE_DIR}/src/lib
)

target_link_libraries(ultralightwebcursor_gnome PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Ultralight::SDK
)

install(TARGETS ultralightwebcursor_gnome RUNTIME DESTINATION bin)
