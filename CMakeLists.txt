cmake_minimum_required(VERSION 3.16)
project(Engine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENGINE_FAST_DEBUG
    "Keep debug symbols while optimizing the render hot path" ON)

# Single-config generators otherwise build with no optimization when
# CMAKE_BUILD_TYPE is omitted. The sample submits tens of thousands of
# sprites, so an unconfigured build can be several times slower.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# ==========================================================
# Executable Name
# ==========================================================
set(GAME_NAME Game)

# ==========================================================
# Output Directories
# ==========================================================
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG
    ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE
    ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
    ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
    ${CMAKE_CURRENT_SOURCE_DIR})

# ==========================================================
# Source Files
# ==========================================================
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    core/*.cpp
    error/*.cpp
    renderer/*.cpp
    util/*.cpp
    ui/*.cpp
    model/*.cpp
)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    test/*.cpp
)

# ==========================================================
# Executable
# ==========================================================
add_executable(${GAME_NAME}
    ${ENGINE_SOURCES}
    ${TEST_SOURCES}
)

# ==========================================================
# Visual Studio
# ==========================================================
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT ${GAME_NAME}
)

set_target_properties(${GAME_NAME} PROPERTIES
    VS_DEBUGGER_COMMAND "$<TARGET_FILE:${GAME_NAME}>"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# ==========================================================
# Include Directories
# ==========================================================
target_include_directories(${GAME_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/error
        ${CMAKE_CURRENT_SOURCE_DIR}/renderer
        ${CMAKE_CURRENT_SOURCE_DIR}/util
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/model
)

# ==========================================================
# Libraries
# ==========================================================
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(OpenGL REQUIRED)

target_link_libraries(${GAME_NAME}
    PRIVATE
        OpenGL::GL
        ${LIB_DIR}/glew32.lib
        ${LIB_DIR}/SDL3.lib
        ${LIB_DIR}/SDL3_ttf.lib
        ${LIB_DIR}/SDL3_mixer.lib
)

# ==========================================================
# Compile Definitions
# ==========================================================
target_compile_definitions(${GAME_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG_MODE>
        $<$<CONFIG:Release>:NDEBUG>
        SDL_MAIN_HANDLED
        ENGINE_RESOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/resources"
        ENGINE_SHADER_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

# ==========================================================
# Compiler Options
# ==========================================================
if(MSVC)

    if(ENGINE_FAST_DEBUG)
        # CMake's MSVC Debug default contains /RTC1, which is incompatible
        # with useful optimization and disproportionately slows the command
        # buffer, vector moves, and stable sort in a sprite stress test.
        string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG
            "${CMAKE_CXX_FLAGS_DEBUG}")
    endif()

    target_compile_options(${GAME_NAME} PRIVATE
        /EHsc
        /utf-8
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${ENGINE_FAST_DEBUG}>>:/O1>
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${ENGINE_FAST_DEBUG}>>:/Ob1>
        $<$<CONFIG:Release>:/O2 /Oi /Ot /GL>
    )

    target_link_options(${GAME_NAME} PRIVATE
        $<$<CONFIG:Release>:/LTCG /INCREMENTAL:NO>
    )

else()

    target_compile_options(${GAME_NAME} PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<BOOL:${ENGINE_FAST_DEBUG}>>:-Og>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )

endif()

# ==========================================================
# Copy DLLs
# ==========================================================
add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_DIR}/SDL3.dll
        $<TARGET_FILE_DIR:${GAME_NAME}>
)

add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_DIR}/SDL3_ttf.dll
        $<TARGET_FILE_DIR:${GAME_NAME}>
)

add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_DIR}/SDL3_mixer.dll
        $<TARGET_FILE_DIR:${GAME_NAME}>
)

add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_DIR}/glew32.dll
        $<TARGET_FILE_DIR:${GAME_NAME}>
)
