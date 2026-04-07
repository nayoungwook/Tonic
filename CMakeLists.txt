cmake_minimum_required(VERSION 3.16)
project(Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
        ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG})
endforeach()

file(GLOB_RECURSE ENGINE_SOURCES
    core/*.cpp
    error/*.cpp
    renderer/*.cpp
    util/*.cpp
)

file(GLOB TEST_SOURCES
    test/*.cpp
)

file(GLOB_RECURSE GAME_SOURCES
)

add_executable(engine_test
    ${ENGINE_SOURCES}
    ${TEST_SOURCES}
    ${GAME_SOURCES}
)

target_include_directories(engine_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(OpenGL REQUIRED)

target_link_libraries(engine_test
    PRIVATE
        OpenGL::GL
        ${LIB_DIR}/glew32.lib
        ${LIB_DIR}/SDL2.lib
        ${LIB_DIR}/SDL2main.lib
)

target_compile_definitions(engine_test
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG_MODE>
        $<$<CONFIG:Release>:NDEBUG>
)

add_custom_command(TARGET engine_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_DIR}/SDL2.dll
        $<TARGET_FILE_DIR:engine_test>
)

add_custom_command(TARGET engine_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_DIR}/glew32.dll
        $<TARGET_FILE_DIR:engine_test>
)

add_custom_command(TARGET engine_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:engine_test>/shaders
)