kcoreaddons_add_plugin(
    ultralightwebcursor
    INSTALL_NAMESPACE "kwin/effects/plugins"
)



target_sources(ultralightwebcursor PRIVATE
    UltralightCursorEffect.cpp
    UltralightHtmlEffect.cpp
    KwinMouseProvider.cpp
    MouseUpload.cpp
    lib/BlackList/BlacklistManager.cpp
    header/UltralightCursorEffect.hpp
    header/KwinMouseProvider.hpp
    header/UltralightHtmlEffect.hpp
    lib/BlackList/BlacklistManager.hpp
    lib/UltralightPl/WebListener.hpp
    lib/WebCall/WebCall.hpp
     $<TARGET_OBJECTS:ultralightwebcursor_core>
)
target_include_directories(ultralightwebcursor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/header
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
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
)


set_target_properties(ultralightwebcursor PROPERTIES PREFIX "")

install(
    FILES metadata.json
    DESTINATION "${CMAKE_INSTALL_DATADIR}/kwin/effects/ultralightwebcursor"
)
