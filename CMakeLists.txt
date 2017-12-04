cmake_minimum_required(VERSION 3.0)
project(autobacklight)

add_executable(autobacklight WIN32 main.cpp Monitor.cpp Camera.cpp escapi.cpp)

target_link_libraries(autobacklight Dxva2)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT autobacklight)


