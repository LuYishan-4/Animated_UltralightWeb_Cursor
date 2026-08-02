cmake_minimum_required(VERSION 3.16)
project(gui_gnome)
message(STATUS "Building GNOME settings GUI")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_add_executable(settings_gnome
	main.cpp
)

qt_add_qml_module(settings_gnome
	URI settings.gnome
	VERSION 1.0
	QML_FILES main.qml
)

target_sources(settings_gnome PRIVATE
	SettingsBackend.cpp
)

target_include_directories(settings_gnome PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(settings_gnome PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)

install(TARGETS settings_gnome RUNTIME DESTINATION bin)
