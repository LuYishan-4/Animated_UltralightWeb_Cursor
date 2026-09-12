set(ENGINE_TARGET ultralightwebcursor-engine)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Network)

set(ENGINE_SOURCES
    main.cpp
    CursorOverlay.cpp
    CursorOverlay.hpp
    MouseProvider.hpp
    IpcServer.cpp
    IpcServer.hpp
    ../renderer/HtmlRenderer.cpp
    ../renderer/HtmlRenderer.hpp
    ../renderer/LoadListener.hpp
    ../renderer/WebBridge.hpp
)

if(WIN32)
    list(APPEND ENGINE_SOURCES platform/WindowsMouseProvider.cpp)
else()
    find_package(X11 REQUIRED)
    list(APPEND ENGINE_SOURCES platform/LinuxMouseProvider.cpp)
endif()

qt_add_executable(${ENGINE_TARGET} ${ENGINE_SOURCES})

target_include_directories(${ENGINE_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../config
    ${CMAKE_CURRENT_SOURCE_DIR}/../renderer
)

target_link_libraries(${ENGINE_TARGET} PRIVATE
    ultralightwebcursor-core
    Ultralight::SDK
    Qt6::Core
    Qt6::Gui
    Qt6::Network
)

if(WIN32)
    target_sources(${ENGINE_TARGET} PRIVATE ${UWC_WINDOWS_RESOURCE})
    target_compile_definitions(${ENGINE_TARGET} PRIVATE WIN32_LEAN_AND_MEAN)
    target_link_libraries(${ENGINE_TARGET} PRIVATE user32)
    set_target_properties(${ENGINE_TARGET} PROPERTIES WIN32_EXECUTABLE TRUE)
else()
    target_link_libraries(${ENGINE_TARGET} PRIVATE X11::X11 X11::Xfixes)
    set_target_properties(${ENGINE_TARGET} PROPERTIES
        BUILD_RPATH "${ULTRALIGHT_BIN}"
        INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/ultralightwebcursor"
    )
endif()

# Make build-tree binaries directly runnable without installing. AppPaths finds
# this `data` directory next to the executables.
add_custom_command(TARGET ${ENGINE_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ULTRALIGHT_ROOT}/resources"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/WebCursor"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data"
    COMMENT "Staging cursor resources for build-tree testing"
)

install(TARGETS ${ENGINE_TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
