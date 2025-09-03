cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
if (UNIX)
    set(CMAKE_SYSTEM_NAME Windows)
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

    # cross compilers to use for C, C++ and Fortran
    set(PRIVATE_MINGW_PATH /usr/bin)
    set(CMAKE_C_COMPILER ${PRIVATE_MINGW_PATH}/${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER ${PRIVATE_MINGW_PATH}/${TOOLCHAIN_PREFIX}-g++)
    set(CMAKE_Fortran_COMPILER ${PRIVATE_MINGW_PATH}/${TOOLCHAIN_PREFIX}-gfortran)
    set(CMAKE_RC_COMPILER /usr/bin/${TOOLCHAIN_PREFIX}-windres)

    # target environment on the build host system
    set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

    # modify default behavior of FIND_XXX() commands
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif ()

project(nexus_tyrian_ledger)

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

# include cpr as a cmake project
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs")
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG da40186618909b1a7363d4e4495aa899c6e0eb75) # Replace with your desired git commit from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

add_library(nexus_tyrian_ledger SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/textures.cpp src/session.cpp)
target_link_libraries(nexus_tyrian_ledger PRIVATE imgui cpr::cpr)
#set_property(TARGET nexus_tyrian_ledger PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# copy dll to game directory
add_custom_command(TARGET nexus_tyrian_ledger POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:nexus_tyrian_ledger>
        "C:\\Games\\Guild Wars 2\\addons\\nexus_tyrian_ledger.dll.update"
        COMMENT "Copying nexus_tyrian_ledger.dll.update to game directory"
)
