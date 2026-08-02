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


if(WIN32)
    set(ULTRALIGHT_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/sdk/ultralight-free-sdk-1.4.0-win-x64"
    )
else()
    set(ULTRALIGHT_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/sdk/ultralight-free-sdk-1.4.0-linux-x64"
    )
endif()

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


#
# ============================================================
#  平台 / 桌面环境 分支
# ============================================================
#
if(WIN32)

    # ===== Windows 独立版：不依赖 KWin/KAuth =====
    add_subdirectory(GUI/windows)
    add_subdirectory(src/windows)

else()

    #
    # ---- Linux 桌面环境检测 ----
    #
    # 打包/CI 环境通常没有登录 session，XDG_CURRENT_DESKTOP 会是空的，
    # 所以提供强制指定选项，例如：
    #   cmake -B build -DFORCE_DESKTOP_ENVIRONMENT=KDE
    #
    set(FORCE_DESKTOP_ENVIRONMENT "" CACHE STRING
        "强制指定目标桌面环境 (KDE/GNOME/HYPRLAND/NIRI/SWAY)，留空则自动检测当前 session"
    )

    if(FORCE_DESKTOP_ENVIRONMENT)
        string(TOUPPER "${FORCE_DESKTOP_ENVIRONMENT}" DETECTED_DESKTOP_ENVIRONMENT)
        message(STATUS "Desktop environment forced to: ${DETECTED_DESKTOP_ENVIRONMENT}")
    else()
        # 依次尝试几个环境变量，取第一个非空的
        set(_de_raw "$ENV{XDG_CURRENT_DESKTOP}")
        if(NOT _de_raw)
            set(_de_raw "$ENV{XDG_SESSION_DESKTOP}")
        endif()
        if(NOT _de_raw)
            set(_de_raw "$ENV{DESKTOP_SESSION}")
        endif()

        string(TOUPPER "${_de_raw}" DETECTED_DESKTOP_ENVIRONMENT)

        if(NOT DETECTED_DESKTOP_ENVIRONMENT)
            message(WARNING
                "无法从环境变量检测桌面环境（XDG_CURRENT_DESKTOP 等均为空）。\n"
                "如果这是打包/CI 环境，请显式指定：-DFORCE_DESKTOP_ENVIRONMENT=KDE"
            )
        else()
            message(STATUS "Detected desktop environment: ${DETECTED_DESKTOP_ENVIRONMENT}")
        endif()
    endif()

    # XDG_CURRENT_DESKTOP 可能是冒号分隔的多值，例如 "KDE" / "ubuntu:GNOME" / "Hyprland"
    # 用 MATCHES 做子串匹配即可覆盖这些情况

    if(DETECTED_DESKTOP_ENVIRONMENT MATCHES "KDE")

        # ===== KDE / KWin：现有实现 =====

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

    elseif(DETECTED_DESKTOP_ENVIRONMENT MATCHES "HYPRLAND" OR
           DETECTED_DESKTOP_ENVIRONMENT MATCHES "NIRI" OR
           DETECTED_DESKTOP_ENVIRONMENT MATCHES "SWAY")


        message(WARNING
            "${DETECTED_DESKTOP_ENVIRONMENT} 尚未实现（wlroots-based compositor，"
            "需要走 wlr-layer-shell overlay 方案，而非 KWin Effect）。"
        )
        add_subdirectory(GUI/hyprland)
        add_subdirectory(GUI/niri)
        add_subdirectory(src/hyprland)
        add_subdirectory(src/niri)

    elseif(DETECTED_DESKTOP_ENVIRONMENT MATCHES "GNOME")

        # ===== GNOME / Mutter =====
        # Mutter 没有等价的原生 C++ effect 插件接口，通常要写 GNOME Shell Extension (JS)，
        # 架构和 KWin/wlroots 都不一样，先占位
        message(WARNING
            "GNOME 尚未实现（需要 GNOME Shell Extension，架构与 KWin 完全不同）。"
        )
        add_subdirectory(GUI/gnome)
        add_subdirectory(src/gnome)
  
    else()

        message(WARNING
            "未识别或不支持的桌面环境: '${DETECTED_DESKTOP_ENVIRONMENT}'\n"
            "当前仅实现了 KDE(KWin) 分支。\n"
            "可用 -DFORCE_DESKTOP_ENVIRONMENT=KDE 强制指定，或提交 PR 支持你的 DE。"
        )

    endif()

endif()