cmake_minimum_required(VERSION 3.20)
project(plus)

set(CMAKE_CXX_STANDARD 20)

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
