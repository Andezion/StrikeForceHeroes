cmake_minimum_required(VERSION 3.29)
project(War)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        v1.3.17
)
FetchContent_MakeAvailable(enet)


add_executable(War main.cpp
        game/Game.cpp game/Game.h
        game/Player.cpp game/Player.h
        shoot/Weapon.cpp shoot/Weapon.h
        shoot/Aim.cpp shoot/Aim.h
        shoot/Bullet.cpp shoot/Bullet.h
        shoot/Particle.cpp shoot/Particle.h)

target_sources(War PRIVATE
        network/NetworkServer.cpp network/NetworkServer.h
        network/NetworkClient.cpp network/NetworkClient.h
)

target_include_directories(War PRIVATE
        ${CMAKE_SOURCE_DIR}/game
        ${CMAKE_SOURCE_DIR}/shoot
        ${CMAKE_SOURCE_DIR}/network
)

target_link_libraries(War PRIVATE raylib)
target_link_libraries(War PRIVATE enet)

if (WIN32)
    target_link_libraries(War PRIVATE winmm)
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(War PRIVATE m dl pthread)
endif()

if (DEFINED enet_SOURCE_DIR AND EXISTS "${enet_SOURCE_DIR}/include")
    target_include_directories(War PRIVATE ${enet_SOURCE_DIR}/include)
elseif(DEFINED enet_BINARY_DIR AND EXISTS "${enet_BINARY_DIR}/include")
    target_include_directories(War PRIVATE ${enet_BINARY_DIR}/include)
endif()