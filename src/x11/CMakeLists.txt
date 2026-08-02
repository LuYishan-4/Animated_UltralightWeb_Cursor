project(ultralightwebcursor-x11)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(X11 REQUIRED)

add_executable(ultralightwebcursor_x11
    main.cpp
    QtCursorEffect.cpp
    ../header/QtCursorEffect.hpp
    QtMouseProvider.cpp
    ../header/QtMouseProvider.hpp
    ../lib/UltralightHtmlEffect.cpp
    ../header/MainCursorStaff.hpp
    ../lib/X11MouseProvider.cpp
    ../lib/BlackList/BlacklistManager.cpp
    ../lib/UltralightPl/WebListener.hpp
    ../lib/WebCall/WebCall.hpp
    $<TARGET_OBJECTS:ultralightwebcursor_core>
)

target_include_directories(ultralightwebcursor_x11 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/header
    ${CMAKE_SOURCE_DIR}/src/lib
)

target_link_libraries(ultralightwebcursor_x11 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Ultralight::SDK
    X11::X11
)

install(TARGETS ultralightwebcursor_x11 RUNTIME DESTINATION bin)
