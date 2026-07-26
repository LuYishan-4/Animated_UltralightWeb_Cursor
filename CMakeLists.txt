cmake_minimum_required(VERSION 3.16)

project(cursor-ani LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-fexceptions)
set(KF6_MIN_VERSION "6.0.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(GNUInstallDirs)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 6.6 REQUIRED COMPONENTS
    Core
    Gui
    DBus
    Widgets
    Qml
    Quick
    QuickControls2
)


find_package(KF6 REQUIRED COMPONENTS
    CoreAddons
    Config
    ConfigWidgets
    KCMUtils
    I18n
    Auth
)


find_package(KWin REQUIRED)






#
# Ultralight SDK
#

set(
    ULTRALIGHT_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/ultralight-free-sdk-1.4.0-linux-x64"
)

set(
    ULTRALIGHT_BIN
    "${ULTRALIGHT_ROOT}/bin"
)


if(NOT EXISTS "${ULTRALIGHT_ROOT}/include/AppCore/App.h")
    message(FATAL_ERROR
        "Ultralight SDK not found: ${ULTRALIGHT_ROOT}"
    )
endif()



add_library(
    Ultralight::Ultralight
    SHARED
    IMPORTED
)

set_target_properties(
    Ultralight::Ultralight
    PROPERTIES

    IMPORTED_LOCATION
        "${ULTRALIGHT_BIN}/libUltralight.so"

    INTERFACE_INCLUDE_DIRECTORIES
        "${ULTRALIGHT_ROOT}/include"
)



add_library(
    Ultralight::UltralightCore
    SHARED
    IMPORTED
)

set_target_properties(
    Ultralight::UltralightCore
    PROPERTIES

    IMPORTED_LOCATION
        "${ULTRALIGHT_BIN}/libUltralightCore.so"

    INTERFACE_INCLUDE_DIRECTORIES
        "${ULTRALIGHT_ROOT}/include"
)



add_library(
    Ultralight::AppCore
    SHARED
    IMPORTED
)

set_target_properties(
    Ultralight::AppCore
    PROPERTIES

    IMPORTED_LOCATION
        "${ULTRALIGHT_BIN}/libAppCore.so"

    INTERFACE_INCLUDE_DIRECTORIES
        "${ULTRALIGHT_ROOT}/include"
)



add_library(
    Ultralight::WebCore
    SHARED
    IMPORTED
)

set_target_properties(
    Ultralight::WebCore
    PROPERTIES

    IMPORTED_LOCATION
        "${ULTRALIGHT_BIN}/libWebCore.so"

    INTERFACE_INCLUDE_DIRECTORIES
        "${ULTRALIGHT_ROOT}/include"
)



add_library(
    Ultralight::SDK
    INTERFACE
    IMPORTED
)


target_link_libraries(
    Ultralight::SDK
    INTERFACE

    Ultralight::Ultralight
    Ultralight::UltralightCore
    Ultralight::AppCore
    Ultralight::WebCore
)




set(
    CMAKE_BUILD_RPATH
    "${ULTRALIGHT_BIN}"
)


set(
    CMAKE_INSTALL_RPATH
     "/usr/lib/ultralightwebcursor"
)
add_library(ultralightwebcursor_core OBJECT 
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
    /usr/include/kwin
    /usr/include/kwin/core
)


target_link_libraries(ultralightwebcursor_core
    PUBLIC
    Qt6::Core
    Qt6::DBus
)
add_subdirectory(src)
add_subdirectory(GUI)







install(
    DIRECTORY
        "${ULTRALIGHT_BIN}/"

    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/ultralightwebcursor"

    FILES_MATCHING
        PATTERN "*.so*"
)


install(DIRECTORY WebCursor/
        DESTINATION  "${CMAKE_INSTALL_DATADIR}/kwin/effects/ultralightwebcursor/resources"
)


feature_summary(
    WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)