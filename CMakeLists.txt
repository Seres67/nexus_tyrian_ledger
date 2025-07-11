cmake_minimum_required(VERSION 3.28)

project(nexus_tyrian_ledger)
set(CMAKE_CXX_STANDARD 20)

if(UNIX)
    set(CMAKE_SYSTEM_NAME Windows)

    # Set compiler to mingw-w64
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
    set(CMAKE_C_COMPILER /usr/bin/${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER /usr/bin/${TOOLCHAIN_PREFIX}-g++)
    set(CMAKE_RC_COMPILER /usr/bin/${TOOLCHAIN_PREFIX}-windres)

    # target environment on the build host system
    set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

    # modify default behavior of FIND_XXX() commands
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

endif ()

# Set output name via parameter, default to clion_demo
set(ADDON_OUTPUT_NAME "nexus_tyrian_ledger" CACHE STRING "Name of the output DLL/library (without extension)")

# Gather all sources and headers automatically
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp include/*.hpp)

# Only include ImGui headers, do not add ImGui sources
set(MODULES_HEADERS
    "modules/imgui/imgui.h"
    "modules/nexus/Nexus.h"
    "modules/mumble/Mumble.h"
)

set(IMGUI_SOURCES
        "modules/imgui/imgui.cpp"
        "modules/imgui/imgui_demo.cpp"
        "modules/imgui/imgui_draw.cpp"
        "modules/imgui/imgui_tables.cpp"
        "modules/imgui/imgui_widgets.cpp"
        "modules/imgui/misc/cpp/imgui_stdlib.cpp"
)

if(UNIX)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>"
    )
endif()

add_library(${ADDON_OUTPUT_NAME} SHARED
    ${SRC_FILES}
    ${MODULES_HEADERS}
    ${IMGUI_SOURCES}
    ${RC_FILE}
)

# Set output properties for Windows DLL
set_target_properties(${ADDON_OUTPUT_NAME} PROPERTIES
    OUTPUT_NAME "${ADDON_OUTPUT_NAME}"
    PREFIX "lib"
    SUFFIX ".dll"
)

# Build types and output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

#
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -g")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif()

set(CPR_FORCE_USE_SYSTEM_CURL ON)

add_subdirectory(modules/cpr)

target_link_libraries(
    ${ADDON_OUTPUT_NAME}
    ntdll
    cpr
)

target_include_directories(${ADDON_OUTPUT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/mumble
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/nexus
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/cpr/include
)


# Export DLL for Dockerfile (copy to predictable name)
add_custom_command(TARGET ${ADDON_OUTPUT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${ADDON_OUTPUT_NAME}>
        ${CMAKE_BINARY_DIR}/lib${ADDON_OUTPUT_NAME}.dll
)
