cmake_minimum_required(VERSION 3.29)
project(War)

set(CMAKE_CXX_STANDARD 20)

set(RAYLIB_PATH "C:/raylib/raylib")

add_subdirectory(${RAYLIB_PATH} ${CMAKE_BINARY_DIR}/raylib_build)

add_executable(War main.cpp)

target_link_libraries(War raylib)

if (WIN32)
    target_link_libraries(War winmm)
endif()