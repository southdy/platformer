cmake_minimum_required(VERSION 2.8)
project(gameplay-encoder)
file(GLOB LIB_SRC ./src/*.cpp ./src/*.h ./src/*.c)
add_executable(gameplay-encoder ${LIB_SRC})
target_link_libraries(gameplay-encoder ${GAMEPLAY_LIB_NAME} ${GAMEPLAY_LIBRARIES} ${GAMEOBJECTS_LIB_NAME} deps)
