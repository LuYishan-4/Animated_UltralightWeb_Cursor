kcoreaddons_add_plugin(
    ultralightwebcursor
    INSTALL_NAMESPACE "kwin/effects/plugins"
)

file(GLOB_RECURSE GL_SOURCES "../lib/gl/*.cpp" "../lib/gl/*.h")


target_sources(ultralightwebcursor PRIVATE
    main.cpp
    ../lib/UltralightHtmlEffect.cpp
    KwinMouseProvider.cpp
    ../lib/BlackList/BlacklistManager.cpp
    ../header/KwinCursorEffect.hpp
    ../header/KwinMouseProvider.hpp
    ../header/UltralightHtmlEffect.hpp
    ../lib/BlackList/BlacklistManager.hpp
    ../lib/CrashReport/CrashReport.hpp
    ${GL_SOURCES}
    ../lib/UltralightPl/WebListener.hpp
    ../lib/WebCall/WebCall.hpp
    ../header/MainCursorStaff.hpp
     $<TARGET_OBJECTS:ultralightwebcursor_core>
)
target_include_directories(ultralightwebcursor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/header
    ${CMAKE_SOURCE_DIR}/src/lib
    /usr/include/kwin
    /usr/include/kwin/core
)



target_link_libraries(ultralightwebcursor
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    KF6::CoreAddons
    KF6::ConfigCore
    KWin::kwin
    Ultralight::SDK
     ${GLFW_LIBRARIES}
)


set_target_properties(ultralightwebcursor PROPERTIES PREFIX "")

install(
    FILES metadata.json
    DESTINATION "${CMAKE_INSTALL_DATADIR}/kwin/effects/ultralightwebcursor"
)
