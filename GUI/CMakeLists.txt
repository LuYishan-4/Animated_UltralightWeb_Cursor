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
    ${CMAKE_SOURCE_DIR}/src/lib
    ${CMAKE_SOURCE_DIR}/src/lib/header
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
)
install(
    FILES kcm_ultralightwebcursor.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/kservices6
)