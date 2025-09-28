cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(nexus_tyrian_ledger)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -g")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -g -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -g -O2")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif()

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

add_library(nexus_tyrian_ledger SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/textures.cpp src/session.cpp)
target_link_libraries(nexus_tyrian_ledger PRIVATE imgui wininet)
set_property(TARGET nexus_tyrian_ledger PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
