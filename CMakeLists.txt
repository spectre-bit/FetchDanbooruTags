cmake_minimum_required(VERSION 3.31)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)

project(fetchDanbooruTags CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 23)

find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(fetchDanbooruTags fetchDanbooruTags.cpp)

target_compile_definitions(fetchDanbooruTags PRIVATE CPR_STATIC_LINKING)
target_link_libraries(fetchDanbooruTags PRIVATE cpr::cpr nlohmann_json::nlohmann_json)