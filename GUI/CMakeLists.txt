find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network)

qt_policy(SET QTP0001 NEW)
qt_add_executable(ultralightwebcursor-gui main.cpp)

qt_add_qml_module(ultralightwebcursor-gui
    URI WebCursorGui
    VERSION 1.0
    QML_FILES
        Main.qml
        StatusBanner.qml
        ThemeCard.qml
    RESOURCES
        icons/io.github.luyishan4.ultralightwebcursor.svg
)

target_sources(ultralightwebcursor-gui PRIVATE
    SettingsBackend.cpp
    SettingsBackend.hpp
)

target_include_directories(ultralightwebcursor-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/engine
)

target_link_libraries(ultralightwebcursor-gui PRIVATE
    ultralightwebcursor-core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
)

if(WIN32)
    target_sources(ultralightwebcursor-gui PRIVATE ${UWC_WINDOWS_RESOURCE})
    set_target_properties(ultralightwebcursor-gui PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

uwc_enable_warnings(ultralightwebcursor-gui)
install(TARGETS ultralightwebcursor-gui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
