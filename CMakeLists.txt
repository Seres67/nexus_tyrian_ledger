cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(BUILD_SHARED_LIBS OFF)
set(OPENSSL_USE_STATIC_LIBS ON)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

project(nexus_tyrian_ledger)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -g")

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

add_library(Threads::Threads INTERFACE IMPORTED GLOBAL)

find_package(OpenSSL REQUIRED)

add_library(nexus_tyrian_ledger SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/textures.cpp src/session.cpp)
target_link_libraries(nexus_tyrian_ledger PRIVATE imgui ws2_32 crypt32 OpenSSL::SSL OpenSSL::Crypto)
set_property(TARGET nexus_tyrian_ledger PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
