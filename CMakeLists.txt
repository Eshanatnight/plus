cmake_minimum_required(VERSION 3.20)
project(plus)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE "D:/Program Files/vcpkg/scripts/buildsystems")
set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_PREFIX_PATH "D:\\Program Files\\vcpkg\\packages\\fmt_x64-windows\\share\\fmt")
#set(unofficial-git2_DIR "D:\\Program Files\\vcpkg\\packages\\libgit2_x64-windows\\share\\unofficial-git2")
#set(zlib_DIR "D:\\Program Files\\vcpkg\\packages\\zlib_x64-windows\\share\\zlib")

set(SRC_FILES
    src/main.cpp
    src/githandler.cpp
    src/util.cpp
    src/cli.cpp
)

    set(HEADER_FILES
    src/include/githandler.h
    src/include/util.h
    src/include/cli.h
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${HEADER_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ".\\src\\include")

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

find_package(unofficial-git2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::git2::git2)
