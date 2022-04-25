cmake_minimum_required(VERSION 3.20)
project(util)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/scripts/buildsystems")
set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_PREFIX_PATH "D:\\Program Files\\vcpkg\\packages\\fmt_x64-windows\\share\\fmt")

set(SRC_FILES src/main.cpp src/githandler.cpp)
set(HEADER_FILES src/include/githandler.h)

add_executable(util
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_include_directories(util PUBLIC ".\\src\\include")

find_package(fmt CONFIG REQUIRED)
target_link_libraries(util PRIVATE fmt::fmt)

find_package(unofficial-git2 CONFIG REQUIRED)
target_link_libraries(util PRIVATE unofficial::git2::git2)