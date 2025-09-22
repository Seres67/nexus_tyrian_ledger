cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)

project(nexus_tyrian_ledger)

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

add_library(nexus_tyrian_ledger SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/textures.cpp src/session.cpp)
target_link_libraries(nexus_tyrian_ledger PRIVATE imgui)
set_property(TARGET nexus_tyrian_ledger PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
