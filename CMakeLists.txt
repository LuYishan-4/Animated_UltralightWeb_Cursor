cmake_minimum_required(VERSION 3.16)

project(cursor-ani LANGUAGES C CXX)

if(MSVC)
    add_compile_options(/EHsc)
else()
    add_compile_options(-fexceptions)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)



set(ULTRALIGHT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/sdk/ultralight-free-sdk-1.4.0-linux-x64")


set(ULTRALIGHT_BIN "${ULTRALIGHT_ROOT}/bin")

if(NOT EXISTS "${ULTRALIGHT_ROOT}/include/AppCore/App.h")
    message(FATAL_ERROR "Ultralight SDK not found: ${ULTRALIGHT_ROOT}")
endif()

function(ultralight_import_library target_name lib_basename)
    add_library(${target_name} SHARED IMPORTED)

    if(WIN32)
        set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION "${ULTRALIGHT_BIN}/${lib_basename}.dll"
            IMPORTED_IMPLIB   "${ULTRALIGHT_BIN}/${lib_basename}.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${ULTRALIGHT_ROOT}/include"
        )
    else()
        set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION "${ULTRALIGHT_BIN}/lib${lib_basename}.so"
            INTERFACE_INCLUDE_DIRECTORIES "${ULTRALIGHT_ROOT}/include"
        )
    endif()
endfunction()

ultralight_import_library(Ultralight::Ultralight     Ultralight)
ultralight_import_library(Ultralight::UltralightCore UltralightCore)
ultralight_import_library(Ultralight::AppCore        AppCore)
ultralight_import_library(Ultralight::WebCore        WebCore)

add_library(Ultralight::SDK INTERFACE IMPORTED)
target_link_libraries(Ultralight::SDK INTERFACE
    Ultralight::Ultralight
    Ultralight::UltralightCore
    Ultralight::AppCore
    Ultralight::WebCore
)

if(NOT WIN32)
    set(CMAKE_BUILD_RPATH "${ULTRALIGHT_BIN}")
    set(CMAKE_INSTALL_RPATH "/usr/lib/ultralightwebcursor")
endif()

find_package(Qt6 6.6 REQUIRED COMPONENTS Core DBus)

add_library(ultralightwebcursor_core OBJECT
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config/CursorJSON.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config/CursorJSON.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config/UserConfig.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config/UserConfig.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Quick/PluginPath/PluginPath.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Quick/PluginPath/PluginPath.hpp"
)

target_include_directories(ultralightwebcursor_core
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/header
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/Quick/PluginPath
)

target_link_libraries(ultralightwebcursor_core
    PUBLIC
    Qt6::Core
    Qt6::DBus
)

# Ensure objects compiled into shared modules are position-independent
set_target_properties(ultralightwebcursor_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set(FORCE_DESKTOP_ENVIRONMENT "" CACHE STRING
    " (KDE/GNOME/HYPRLAND/NIRI/SWAY/WINDOWS):"
)

if(WIN32)
    set(DETECTED_DESKTOP_ENVIRONMENT "WINDOWS")
elseif(FORCE_DESKTOP_ENVIRONMENT)
    string(TOUPPER "${FORCE_DESKTOP_ENVIRONMENT}" DETECTED_DESKTOP_ENVIRONMENT)
    message(STATUS "Desktop environment forced to: ${DETECTED_DESKTOP_ENVIRONMENT}")
else()
    set(_de_raw "$ENV{XDG_CURRENT_DESKTOP}")
    if(NOT _de_raw)
        set(_de_raw "$ENV{XDG_SESSION_DESKTOP}")
    endif()
    if(NOT _de_raw)
        set(_de_raw "$ENV{DESKTOP_SESSION}")
    endif()

    string(TOUPPER "${_de_raw}" DETECTED_DESKTOP_ENVIRONMENT)
endif()

if(WIN32 OR DETECTED_DESKTOP_ENVIRONMENT MATCHES "WINDOWS")
    target_compile_definitions(ultralightwebcursor_core PUBLIC BUILD_TYPE_WINDOWS)
    add_subdirectory(GUI/windows)
    add_subdirectory(src/windows)
else()


    if(DETECTED_DESKTOP_ENVIRONMENT MATCHES "KDE")
        set(KF6_MIN_VERSION "6.0.0")

        find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
        set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

        include(FeatureSummary)
        include(GNUInstallDirs)
        include(KDEInstallDirs)
        include(KDECMakeSettings)
        include(KDECompilerSettings NO_POLICY_SCOPE)

        find_package(Qt6 6.6 REQUIRED COMPONENTS
            Core Gui DBus Widgets Qml Quick QuickControls2
        )

        find_package(KF6 REQUIRED COMPONENTS
            CoreAddons Config ConfigWidgets KCMUtils I18n Auth
        )

        find_package(KWin REQUIRED)
        target_compile_definitions(ultralightwebcursor_core PUBLIC BUILD_TYPE_KWIN)
        add_subdirectory(src/kwin-plasma)
        add_subdirectory(GUI/kwin-plasma)


        install(
            DIRECTORY "${ULTRALIGHT_BIN}/"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/ultralightwebcursor"
            FILES_MATCHING PATTERN "*.so*"
        )

        install(DIRECTORY WebCursor/
            DESTINATION "${CMAKE_INSTALL_DATADIR}/kwin/effects/ultralightwebcursor"
        )
        install(
            DIRECTORY "${ULTRALIGHT_ROOT}/resources"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/kwin/effects/ultralightwebcursor"
        )

        feature_summary(
            WHAT ALL
            INCLUDE_QUIET_PACKAGES
            FATAL_ON_MISSING_REQUIRED_PACKAGES
        )

    #elseif(DETECTED_DESKTOP_ENVIRONMENT MATCHES "HYPRLAND")
     #   add_subdirectory(GUI/hyprland)
    #    add_subdirectory(src/hyprland) not now :D
    elseif(DETECTED_DESKTOP_ENVIRONMENT MATCHES "X11")
      target_compile_definitions(ultralightwebcursor_core PUBLIC BUILD_TYPE_X11)
        add_subdirectory(GUI/x11)
        add_subdirectory(src/x11)
    else()
        target_compile_definitions(ultralightwebcursor_core PUBLIC BUILD_TYPE_X11)
        add_subdirectory(GUI/x11)
        add_subdirectory(src/x11)
    endif()

endif()