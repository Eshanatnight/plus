cmake_minimum_required(VERSION 3.20)
project(util)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/scripts/buildsystems")
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_PREFIX_PATH "D:\\Program Files\\vcpkg\\packages\\fmt_x64-windows\\share\\fmt")

add_executable(util src/main.cpp src/githandler.cpp src/util.cpp)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(util PRIVATE fmt::fmt)