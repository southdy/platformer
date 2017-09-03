cmake_minimum_required(VERSION 2.8)
set(LIB_NAME gameobjects)
project(${LIB_NAME})
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
include_directories(
    ${GAMEPLAY_PATH}/gameplay/src
    ${GAMEPLAY_PATH}/external-deps/include
    ${GAMEPLAY_PATH}/external-deps/include/AL
    ${GAMEPLAY_PATH}/external-deps/include/BulletCollision
    ${GAMEPLAY_PATH}/external-deps/include/BulletDynamics
    ${GAMEPLAY_PATH}/external-deps/include/AL
    ${GAMEPLAY_PATH}/external-deps/include/freetype
    ${GAMEPLAY_PATH}/external-deps/include/GL
    ${GAMEPLAY_PATH}/external-deps/include/LinearMath
    ${GAMEPLAY_PATH}/external-deps/include/lua
    ${GAMEPLAY_PATH}/external-deps/include/ogg
    ${GAMEPLAY_PATH}/external-deps/include/vectormath
    ${GAMEPLAY_PATH}/external-deps/include/vorbis
)
file(GLOB LIB_SRC ./src/*.cpp ./src/*.h ./src/*.inl)
add_library(${LIB_NAME} ${LIB_SRC})
