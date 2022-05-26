cmake_minimum_required(VERSION 3.20)
project(plus)

# Until CMake 3.20.3, if you ask for C++20, using set(CMAKE_CXX_STANDARD 20), you'll get -std:c++latest.
# Since CMake 3.20.4, set(CMAKE_CXX_STANDARD 20) gets you -std:c++20,
# so you need set(CMAKE_CXX_STANDARD 23) to get -std:c++latest --
# assuming that your MSVC compiler version is 16.11 Preview 1 or later (see cmake commit 3aaf1d91bf353).
# issue with the standard library of MSVC
# https://stackoverflow.com/questions/64889383/how-to-enable-stdclatest-in-cmake
set(CMAKE_CXX_STANDARD 23)

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
