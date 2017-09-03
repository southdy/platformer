
file(GLOB GAMEPLAY_SRC src/*.h src/*.cpp src/*.inl)

if(TARGET_OS STREQUAL "MACOSX")
    set(GAMEPLAY_SRC ${GAMEPLAY_SRC}
        src/gameplay-main-macosx.mm
        src/PlatformMacOSX.mm)
ENDIF(TARGET_OS STREQUAL "MACOSX")

file(GLOB GAMEPLAY_LUA src/lua/*.h src/lua/*.cpp)

set(GAMEPLAY_RES
    res/logo_black.png
    res/logo_powered_black.png
    res/logo_powered_white.png
    res/logo_white.png
)

set(GAMEPLAY_RES_SHADERS
    res/shaders/colored.frag
    res/shaders/colored.vert
    res/shaders/font.frag
    res/shaders/font.vert
    res/shaders/form.frag
    res/shaders/form.vert
    res/shaders/lighting.frag
    res/shaders/lighting.vert
    res/shaders/skinning.vert
    res/shaders/skinning-none.vert
    res/shaders/sprite.frag
    res/shaders/sprite.vert
    res/shaders/terrain.frag
    res/shaders/terrain.vert
    res/shaders/textured.frag
    res/shaders/textured.vert
)

set(GAMEPLAY_RES_SHADERS
    res/ui/arial.gpb
    res/ui/default.png
    res/ui/default.theme
)

include_directories(
    src
    ../external-deps/include
)

add_library(gameplay STATIC
    ${GAMEPLAY_SRC}
    ${GAMEPLAY_LUA}
)

set_target_properties(gameplay PROPERTIES
    OUTPUT_NAME "gameplay"
    CLEAN_DIRECT_OUTPUT 1
    VERSION 4.0.0
)

source_group(lua FILES ${GAMEPLAY_LUA})
source_group(res FILES ${GAMEPLAY_RES} ${GAMEPLAY_RES} ${GAMEPLAY_RES_SHADERS} ${GAMEPLAY_RES_UI})
source_group(src FILES ${GAMEPLAY_SRC})



