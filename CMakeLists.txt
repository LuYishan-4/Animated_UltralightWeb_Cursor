cmake_minimum_required(VERSION 3.21)

project(UltralightWebCursor VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(GNUInstallDirs)

if(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "Supported platforms are Linux (X11/XWayland) and Windows")
endif()

function(uwc_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /EHsc)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# ---- Ultralight SDK -------------------------------------------------------
if(WIN32)
    set(ULTRALIGHT_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/sdk/ultralight-free-sdk-1.4.0-win-x64")
else()
    set(ULTRALIGHT_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/sdk/ultralight-free-sdk-1.4.0-linux-x64")
endif()
set(ULTRALIGHT_BIN "${ULTRALIGHT_ROOT}/bin")

if(NOT EXISTS "${ULTRALIGHT_ROOT}/include/AppCore/App.h")
    message(FATAL_ERROR
        "Ultralight SDK not found at ${ULTRALIGHT_ROOT}. Run ./build.sh first.")
endif()
if(NOT EXISTS "${ULTRALIGHT_ROOT}/resources/cacert.pem" OR
   NOT EXISTS "${ULTRALIGHT_ROOT}/resources/icudt67l.dat" OR
   NOT EXISTS "${ULTRALIGHT_ROOT}/license/EULA.txt")
    message(FATAL_ERROR "Ultralight SDK resources or license files are incomplete")
endif()

foreach(_uwc_library AppCore Ultralight UltralightCore WebCore)
    if(WIN32)
        if(NOT EXISTS "${ULTRALIGHT_BIN}/${_uwc_library}.dll" OR
           NOT EXISTS "${ULTRALIGHT_ROOT}/lib/${_uwc_library}.lib")
            message(FATAL_ERROR "Missing Ultralight library: ${_uwc_library}")
        endif()
    elseif(NOT EXISTS "${ULTRALIGHT_BIN}/lib${_uwc_library}.so")
        message(FATAL_ERROR "Missing Ultralight library: ${_uwc_library}")
    endif()
endforeach()

function(ultralight_import_library target_name library_name)
    add_library(${target_name} SHARED IMPORTED GLOBAL)
    if(WIN32)
        set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION "${ULTRALIGHT_BIN}/${library_name}.dll"
            IMPORTED_IMPLIB "${ULTRALIGHT_ROOT}/lib/${library_name}.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${ULTRALIGHT_ROOT}/include"
        )
    else()
        set_target_properties(${target_name} PROPERTIES
            IMPORTED_LOCATION "${ULTRALIGHT_BIN}/lib${library_name}.so"
            INTERFACE_INCLUDE_DIRECTORIES "${ULTRALIGHT_ROOT}/include"
        )
    endif()
endfunction()

ultralight_import_library(Ultralight::Ultralight Ultralight)
ultralight_import_library(Ultralight::UltralightCore UltralightCore)
ultralight_import_library(Ultralight::AppCore AppCore)
ultralight_import_library(Ultralight::WebCore WebCore)

add_library(Ultralight::SDK INTERFACE IMPORTED GLOBAL)
target_link_libraries(Ultralight::SDK INTERFACE
    Ultralight::Ultralight
    Ultralight::UltralightCore
    Ultralight::AppCore
    Ultralight::WebCore
)

# ---- Shared application core ---------------------------------------------
find_package(Qt6 6.6 REQUIRED COMPONENTS Core)

set(UWC_INSTALL_DATA_DIR
    "${CMAKE_INSTALL_FULL_DATADIR}/ultralightwebcursor")
set(UWC_DATA_REVISION "${PROJECT_VERSION}")
find_package(Git QUIET)
if(Git_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short=12 HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _uwc_git_revision
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(_uwc_git_revision)
        string(APPEND UWC_DATA_REVISION "+g${_uwc_git_revision}")
    endif()
endif()
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config/BuildConfig.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/BuildConfig.hpp"
    @ONLY
)
file(WRITE "${CMAKE_BINARY_DIR}/uwc-version.txt" "${PROJECT_VERSION}\n")

if(WIN32)
    set(UWC_WINDOWS_ICON
        "${CMAKE_CURRENT_SOURCE_DIR}/GUI/icons/UltralightWebCursor.ico")
    set(UWC_WINDOWS_RESOURCE
        "${CMAKE_CURRENT_BINARY_DIR}/generated/UltralightWebCursor.rc")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/GUI/windows-version.rc.in"
        "${UWC_WINDOWS_RESOURCE}"
        @ONLY
    )
endif()

add_library(ultralightwebcursor-core STATIC
    src/config/AppPaths.cpp
    src/config/AppPaths.hpp
    src/config/ThemeMetadata.cpp
    src/config/ThemeMetadata.hpp
    src/config/UserConfig.cpp
    src/config/UserConfig.hpp
    src/engine/Autostart.cpp
    src/engine/Autostart.hpp
)

target_include_directories(ultralightwebcursor-core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/engine"
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)
target_link_libraries(ultralightwebcursor-core PUBLIC Qt6::Core)
uwc_enable_warnings(ultralightwebcursor-core)

add_subdirectory(GUI)
add_subdirectory(src/engine)

# ---- Installation ---------------------------------------------------------
if(WIN32)
    install(DIRECTORY "${ULTRALIGHT_BIN}/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "*.dll")
    install(DIRECTORY "${ULTRALIGHT_ROOT}/resources"
        DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY WebCursor/
        DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES LICENSE
        DESTINATION licenses/UltralightWebCursor)
    install(DIRECTORY "${ULTRALIGHT_ROOT}/license/"
        DESTINATION licenses/Ultralight)
    install(FILES "${ULTRALIGHT_ROOT}/license/NOTICES.md"
        DESTINATION licenses/Ultralight
        RENAME NOTICES.txt)
else()
    install(DIRECTORY "${ULTRALIGHT_BIN}/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/ultralightwebcursor"
        FILES_MATCHING PATTERN "*.so*")
    install(DIRECTORY WebCursor/
        DESTINATION "${CMAKE_INSTALL_DATADIR}/ultralightwebcursor")
    install(DIRECTORY "${ULTRALIGHT_ROOT}/resources"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/ultralightwebcursor")

    install(FILES
        GUI/io.github.luyishan4.ultralightwebcursor.desktop
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    install(FILES
        GUI/io.github.luyishan4.ultralightwebcursor.metainfo.xml
        DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")
    install(FILES
        GUI/icons/io.github.luyishan4.ultralightwebcursor.svg
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")

    set(UWC_LICENSE_INSTALL_DIR
        "${CMAKE_INSTALL_DATADIR}/licenses/ultralightwebcursor"
        CACHE PATH "Relative directory used to install application licenses")
    install(FILES LICENSE
        DESTINATION "${UWC_LICENSE_INSTALL_DIR}")
    install(DIRECTORY "${ULTRALIGHT_ROOT}/license/"
        DESTINATION "${UWC_LICENSE_INSTALL_DIR}/Ultralight")
    install(FILES "${ULTRALIGHT_ROOT}/license/NOTICES.md"
        DESTINATION "${UWC_LICENSE_INSTALL_DIR}/Ultralight"
        RENAME NOTICES.txt)
endif()
