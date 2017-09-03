cmake_minimum_required(VERSION 2.8)
project(platformer)

macro (link_os_lib libName)
    find_library(FOUND_LIB_${libName} NAMES ${libName})
    list(APPEND GAMEPLAY_LIBRARIES ${FOUND_LIB_${libName}})
    message(STATUS "Library Found: ${libName} Path: ${FOUND_LIB_${libName}}")
endmacro (link_os_lib)

macro(add_source_group SOURCE_PATH SOURCES)
    if(WIN32)
        string(REPLACE "/" "\\\\" SOURCE_PATH_FINAL ${SOURCE_PATH})
    else()
        set(SOURCE_PATH_FINAL ${SOURCE_PATH})
    endif()
    source_group(${SOURCE_PATH_FINAL} FILES ${SOURCES})
endmacro(add_source_group)

set(GAME_NAME platformer)
set(PLATFORMER_EXT_LIBS_PATH "external")
set(GAMEPLAY_EXT_LIBS_PATH "${PLATFORMER_EXT_LIBS_PATH}/GamePlay-deps")
set(GAMEPLAY_LIB_NAME "gameplay")
set(GAMEOBJECTS_LIB_NAME "gameobjects")
set(GAMEPLAY_LIB_DIR_NAME "GamePlay")
set(GAMEOBJECTS_LIB_DIR_NAME "gameobjects-gameplay3d")
set(GAMEPLAY_PATH "${PLATFORMER_EXT_LIBS_PATH}/${GAMEPLAY_LIB_DIR_NAME}")
set(GAMEPLAY_SRC_PATH "${GAMEPLAY_PATH}/gameplay/src")
set(GAME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(DEBUG_ONLY_DEFINES_GP "-DGP_USE_PROFILER")
set(RELEASE_ONLY_DEFINES_GP "-DGP_NO_LUA_BINDINGS -DGP_USE_PROFILER")
set(GAME_CONFIG ./build/game.config ./build/user.config ./build/default.config .gitignore README.md)
set(ANDROID_CONFIG ./android/AndroidManifest.xml ./android/build.xml ./android/res/values/strings.xml ./android/jni/Android.mk ./android/jni/Application.mk)
file(GLOB RAW_LEVELS ./raw/levels/*)
file(GLOB RAW_VECTOR ./raw/svgs/*.export)
file(GLOB RAW_SPRITESHEETS ./raw/spritesheets/*)
file(GLOB RAW_AUTOMATION ./raw/automation/*)
file(GLOB RES_AUDIO ./res/audio/*.audio)
file(GLOB RES_LUA ./res/lua/*.lua)
file(GLOB RES_LUA_TOOLS ./res/lua/tools/*)
file(GLOB RES_GAME_OBJECTS ./res/gameobjects/*)
file(GLOB RES_LEVELS ./res/levels/*)
file(GLOB RES_PARALLAX ./res/parallax/*)
file(GLOB RES_PHYSICS ./res/physics/*)
file(GLOB RES_SHADERS ./res/shaders/*)
file(GLOB RES_SPRITESHEETS ./res/spritesheets/*)
file(GLOB RES_UI ./res/ui/*)
file(GLOB GAME_SRC ./src/*.cpp ./src/*.h ./src/*.inl)

add_definitions(-DGP_SCENE_VISIT_EXTENSIONS)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(TARGET_OS "LINUX")
    add_definitions(-D__linux__)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(TARGET_OS "MACOSX")
    add_definitions(-D__APPLE__)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(TARGET_OS "WINDOWS")
    add_definitions(-DWIN32 -D_WINDOWS -DUNICODE)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(MSVC_COMMON "-DMSVC /MP /Gm-")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=2 /MDd /Zi ${MSVC_COMMON}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_ITERATOR_DEBUG_LEVEL=0 ${MSVC_COMMON}")
endif()

if(WIN32)
    execute_process(COMMAND cmd /C "${CMAKE_SOURCE_DIR}/raw/automation/mklink.bat")
    add_source_group("android" "${ANDROID_CONFIG}")
    add_source_group("src" "${GAME_SRC}")
    add_source_group("res" "${GAME_CONFIG}")
    add_source_group("raw/levels" "${RAW_LEVELS}")
    add_source_group("raw/vector" "${RAW_VECTOR}")
    add_source_group("raw/automation" "${RAW_AUTOMATION}")
    add_source_group("raw/spritesheets" "${RAW_SPRITESHEETS}")
    add_source_group("res/audio" "${RES_AUDIO}")
    add_source_group("res/lua" "${RES_LUA}")
    add_source_group("res/lua/tools" "${RES_LUA_TOOLS}")
    add_source_group("res/gameobjects" "${RES_GAME_OBJECTS}")
    add_source_group("res/levels" "${RES_LEVELS}")
    add_source_group("res/parallax" "${RES_PARALLAX}")
    add_source_group("res/physics" "${RES_PHYSICS}")
    add_source_group("res/shaders" "${RES_SPRITESHEETS}")
    add_source_group("res/spritesheets" "${RES_SHADERS}")
    add_source_group("res/ui" "${RES_UI}")
else()
    execute_process(COMMAND "${CMAKE_SOURCE_DIR}/raw/automation/mklink.sh")
    add_source_group("android" "${ANDROID_CONFIG}")
    add_source_group("src" "${GAME_SRC}")
    add_source_group("res" "${GAME_CONFIG}")
    add_source_group("raw/levels" "${RAW_LEVELS}")
    add_source_group("raw/vector" "${RAW_VECTOR}")
    add_source_group("raw/automation" "${RAW_AUTOMATION}")
    add_source_group("raw/spritesheets" "${RAW_SPRITESHEETS}")
    add_source_group("res/audio" "${RES_AUDIO}")
    add_source_group("res/lua" "${RES_LUA}")
    add_source_group("res/lua/tools" "${RES_LUA_TOOLS}")
    add_source_group("res/gameobjects" "${RES_GAME_OBJECTS}")
    add_source_group("res/levels" "${RES_LEVELS}")
    add_source_group("res/parallax" "${RES_PARALLAX}")
    add_source_group("res/physics" "${RES_PHYSICS}")
    add_source_group("res/shaders" "${RES_SPRITESHEETS}")
    add_source_group("res/spritesheets" "${RES_SHADERS}")
    add_source_group("res/ui" "${RES_UI}")
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GAMEPLAY_SRC_PATH}
    ${CMAKE_SOURCE_DIR}/src
    ${GAMEPLAY_EXT_LIBS_PATH}/lua-5.2.3
    ${GAMEPLAY_EXT_LIBS_PATH}/bullet-2.82-r2704/src
    ${GAMEPLAY_EXT_LIBS_PATH}/openal-1.16.0/include
    ${GAMEPLAY_EXT_LIBS_PATH}/vorbis-1.3.4/include
    ${GAMEPLAY_EXT_LIBS_PATH}/ogg-1.3.2/include
    ${GAMEPLAY_EXT_LIBS_PATH}/png-1.6.15
    ${GAMEPLAY_EXT_LIBS_PATH}/glew-1.10.0/include
    ${GAMEPLAY_EXT_LIBS_PATH}/freetype-2.4.5/include
    ${GAMEPLAY_EXT_LIBS_PATH}/freetype-2.4.5/include/freetype
    ${GAMEPLAY_EXT_LIBS_PATH}/tinyxml2-2.1.0
    ${GAMEPLAY_EXT_LIBS_PATH}/base64-1.0.0
    ${GAMEPLAY_EXT_LIBS_PATH}/zlib-1.2.8
    ${PLATFORMER_EXT_LIBS_PATH}/${GAMEOBJECTS_LIB_DIR_NAME}/src
)

if(BUILD_CONTENT_TOOLS)
    add_subdirectory(external/json-to-gameplay3d)
    add_subdirectory(${PLATFORMER_EXT_LIBS_PATH}/${GAMEPLAY_LIB_DIR_NAME}/tools/encoder)
endif()

add_subdirectory(raw/automation)
add_subdirectory(${PLATFORMER_EXT_LIBS_PATH}/GamePlay-deps)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_ONLY_DEFINES_GP}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${RELEASE_ONLY_DEFINES_GP}")
add_subdirectory(${PLATFORMER_EXT_LIBS_PATH}/${GAMEPLAY_LIB_DIR_NAME}/gameplay)
add_subdirectory(${PLATFORMER_EXT_LIBS_PATH}/${GAMEOBJECTS_LIB_DIR_NAME})

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FINAL")
add_executable(${GAME_NAME}
    WIN32
    ${GAME_SRC}
    ${ANDROID_CONFIG}
    ${GAME_CONFIG}
    ${RES_GAME_OBJECTS}
    ${RES_SPRITESHEETS}
    ${RAW_SPRITESHEETS}
    ${RAW_LEVELS}
    ${RAW_VECTOR}
    ${RES_AUDIO}
    ${RES_LEVELS}
    ${RES_PARALLAX}
    ${RES_PHYSICS}
    ${RES_LUA}
    ${RES_LUA_TOOLS}
    ${RAW_AUTOMATION}
    ${RES_UI}
    ${RES_SHADERS}
)

if(NOT TARGET_OS STREQUAL "WINDOWS")
    link_os_lib("dl")
    link_os_lib("m")
    link_os_lib("pthread")
    if(TARGET_OS STREQUAL "LINUX")
        link_os_lib("GL")
        link_os_lib("X11")
        link_os_lib("rt")
        link_os_lib("glib-2.0")
        link_os_lib("gobject-2.0")
        link_os_lib("freetype")
    else()
        link_os_lib("AppKit")
        link_os_lib("CoreVideo")
        link_os_lib("GameKit")
        link_os_lib("Foundation")
        link_os_lib("IOKit")
        link_os_lib("OpenAL")
        link_os_lib("OpenGL")
    endif(TARGET_OS STREQUAL "LINUX")
else()
    list(APPEND GAMEPLAY_LIBRARIES "OpenGL32")
    list(APPEND GAMEPLAY_LIBRARIES "GLU32")
    list(APPEND GAMEPLAY_LIBRARIES "XInput")
endif (NOT TARGET_OS STREQUAL "WINDOWS")
target_link_libraries(${GAME_NAME} ${GAMEPLAY_LIB_NAME} deps ${GAMEPLAY_LIBRARIES} ${GAMEOBJECTS_LIB_NAME})
add_dependencies(${GAME_NAME} ${GAMEPLAY_LIB_NAME} ${GAMEOBJECTS_LIB_NAME} deps gameplay-deps)
set_target_properties(${GAME_NAME} PROPERTIES
    OUTPUT_NAME "${GAME_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIR}"
)
