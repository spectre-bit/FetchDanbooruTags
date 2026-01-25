cmake_minimum_required(VERSION 3.31)
project(FetchDanbooruTags CXX)

find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 23)

add_executable(fetchDanbooruTags fetchDanbooruTags.cpp)

target_link_libraries(fetchDanbooruTags PRIVATE cpr::cpr)
target_link_libraries(fetchDanbooruTags PRIVATE nlohmann_json::nlohmann_json)

