cmake_minimum_required(VERSION 3.21)
project(util)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/scripts/buildsystems")
set(CMAKE_BUILD_TYPE Debug)

add_executable(util main.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(util PRIVATE fmt::fmt)