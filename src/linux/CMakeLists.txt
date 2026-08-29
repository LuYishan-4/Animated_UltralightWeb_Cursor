project(ultralightwebcursor-installer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core)

add_executable(ultralightwebcursor-install main.cpp)

target_link_libraries(ultralightwebcursor-install PRIVATE Qt6::Core)

install(TARGETS ultralightwebcursor-install RUNTIME DESTINATION bin)
