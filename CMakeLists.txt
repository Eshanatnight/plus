cmake_minimum_required(VERSION 3.20)
project(plus)

# Until CMake 3.20.3, if you ask for C++20, using set(CMAKE_CXX_STANDARD 20), you'll get -std:c++latest.
# Since CMake 3.20.4, set(CMAKE_CXX_STANDARD 20) gets you -std:c++20,
# so you need set(CMAKE_CXX_STANDARD 23) to get -std:c++latest --
# assuming that your MSVC compiler version is 16.11 Preview 1 or later (see cmake commit 3aaf1d91bf353).
# issue with the standard library of MSVC
# https://stackoverflow.com/questions/64889383/how-to-enable-stdclatest-in-cmake
set(CMAKE_CXX_STANDARD 23)


set(SRC_FILE
    src/cli.cpp
    src/githandler.cpp
    src/main.cpp
    src/util.cpp
    src/cppgit2/annotated_commit.cpp
    src/cppgit2/blame.cpp
    src/cppgit2/blob.cpp
    src/cppgit2/buffer.cpp
    src/cppgit2/commit.cpp
    src/cppgit2/config.cpp
    src/cppgit2/diff.cpp
    src/cppgit2/id_to_str.cpp
    src/cppgit2/index.cpp
    src/cppgit2/initializer.cpp
    src/cppgit2/object.cpp
    src/cppgit2/odb_object.cpp
    src/cppgit2/odb.cpp
    src/cppgit2/reference.cpp
    src/cppgit2/remote.cpp
    src/cppgit2/repo.cpp
    src/cppgit2/revspec.cpp
    src/cppgit2/revwalker.cpp
    src/cppgit2/signature.cpp
    src/cppgit2/status.cpp
    src/cppgit2/submodule.cpp
    src/cppgit2/tag.cpp
    src/cppgit2/tree.cpp
)

set(HEADER_FILE
    "include/cli.h"
    "include/githandler.h"
    "include/util.h"
    "include/cppgit2/annotated_commit.h"
    "include/cppgit2/blame.h"
    "include/cppgit2/blob.h"
    "include/cppgit2/buffer.h"
    "include/cppgit2/commit.h"
    "include/cppgit2/config.h"
    "include/cppgit2/diff.h"
    "include/cppgit2/id_to_str.h"
    "include/cppgit2/index.h"
    "include/cppgit2/initializer.h"
    "include/cppgit2/object.h"
    "include/cppgit2/odb_object.h"
    "include/cppgit2/odb.h"
    "include/cppgit2/reference.h"
    "include/cppgit2/remote.h"
    "include/cppgit2/repo.h"
    "include/cppgit2/revspec.h"
    "include/cppgit2/revwalker.h"
    "include/cppgit2/signature.h"
    "include/cppgit2/status.h"
    "include/cppgit2/submodule.h"
    "include/cppgit2/tag.h"
    "include/cppgit2/tree.h"
)


add_executable(${PROJECT_NAME}
    ${SRC_FILE}
    ${HEADER_FILE}
)

target_include_directories(${PROJECT_NAME} PUBLIC ".\\include")
target_include_directories(${PROJECT_NAME} PUBLIC ".\\include\\cppgit2")

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

find_package(unofficial-git2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::git2::git2)
