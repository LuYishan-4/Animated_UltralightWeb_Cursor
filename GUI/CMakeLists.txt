cmake_minimum_required(VERSION 3.16)

set(QT_MIN_VERSION "6.5.0")
set(KF6_MIN_VERSION "6.0.0")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(kcm_ultralightwebcursor)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)



include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GNUInstallDirs)

set(KDE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBEXECDIR})
kcmutils_add_qml_kcm(kcm_ultralightwebcursor)

target_sources(kcm_ultralightwebcursor PRIVATE
    main_settings.cpp
    main_settings.hpp
    SettingsBackend.cpp
    SettingsBackend.hpp
          $<TARGET_OBJECTS:ultralightwebcursor_core>
)




target_include_directories(kcm_ultralightwebcursor PRIVATE
     ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/header
    ${CMAKE_SOURCE_DIR}/src/lib
    /usr/include/kwin
    /usr/include/kwin/core
)



target_link_libraries(

    kcm_ultralightwebcursor

    PRIVATE


    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus


    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::KCMUtils
    KF6::I18n
    KF6::AuthCore
)
install(
    FILES kcm_ultralightwebcursor.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/kservices6
)
#
# KAuth helper
#

add_executable(
    ultralightwebcursor_helper
    khelper.cpp
)


target_link_libraries(
    ultralightwebcursor_helper

    PRIVATE

    KF6::AuthCore
    KF6::CoreAddons
    Qt6::Core

    ultralightwebcursor_core
)



install(
    TARGETS
        ultralightwebcursor_helper

    DESTINATION
        ${KDE_INSTALL_LIBEXECDIR}
)



#
# Policy
#

install(
    FILES
        org.ultralightwebcursor.policy

    DESTINATION
        ${KDE_INSTALL_DATADIR}/polkit-1/actions
)