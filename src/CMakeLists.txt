kcoreaddons_add_plugin(
    ultralightwebcursor
    INSTALL_NAMESPACE "kwin/effects/plugins"
)



target_sources(ultralightwebcursor PRIVATE
    UltralightCursorEffect.cpp
     $<TARGET_OBJECTS:ultralightwebcursor_core>
)
target_include_directories(ultralightwebcursor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/header
)



target_link_libraries(ultralightwebcursor
    PRIVATE
    Qt6::Core
    Qt6::Gui
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
