cmake_minimum_required(VERSION 3.29)
project(War)

set(CMAKE_CXX_STANDARD 20)

set(RAYLIB_PATH "C:/raylib/raylib")

add_subdirectory(${RAYLIB_PATH} ${CMAKE_BINARY_DIR}/raylib_build)

include(FetchContent)
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        v1.3.17
)
FetchContent_MakeAvailable(enet)


add_executable(War main.cpp
        Game.cpp Game.h
        Player.cpp Player.h
        Weapon.cpp Weapon.h
        Aim.cpp Aim.h
        Bullet.cpp Bullet.h
        Particle.cpp Particle.h)

target_sources(War PRIVATE
        NetworkServer.cpp NetworkClient.h
        NetworkClient.cpp NetworkServer.h
)

target_link_libraries(War PRIVATE raylib)
target_link_libraries(War PRIVATE enet)

if (WIN32)
    target_link_libraries(War PRIVATE winmm)
endif()

if (DEFINED enet_SOURCE_DIR AND EXISTS "${enet_SOURCE_DIR}/include")
    target_include_directories(War PRIVATE ${enet_SOURCE_DIR}/include)
elseif(DEFINED enet_BINARY_DIR AND EXISTS "${enet_BINARY_DIR}/include")
    target_include_directories(War PRIVATE ${enet_BINARY_DIR}/include)
endif()