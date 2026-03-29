cmake_minimum_required(VERSION 3.10)
project(plus)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# For clang-tidy, IDE integration, and `plus tidy -p <build>`
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PLUS_ENABLE_CLANG_TIDY "Run clang-tidy when compiling plus (and plus_tests)" OFF)

include(FetchContent)

FetchContent_Declare(
  structopt
  GIT_REPOSITORY https://github.com/Eshanatnight/structopt
  GIT_TAG        v0.2.1
)

FetchContent_GetProperties(structopt)
FetchContent_MakeAvailable(structopt)

FetchContent_Declare(
  subprocess
  GIT_REPOSITORY https://github.com/Eshanatnight/subprocess
  GIT_TAG        master
)

FetchContent_GetProperties(subprocess)
FetchContent_MakeAvailable(subprocess)

include(CTest)

if(BUILD_TESTING)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
  )
  FetchContent_MakeAvailable(Catch2)
endif()

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
else()
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Wno-long-long -pedantic")
endif()


set(STRUCTOPT_SRC ${structopt_SOURCE_DIR}/src)

set(PLUS_SRC ./src/main.cpp ./src/git.cpp ./src/file_control.cpp ./src/config.cpp ./src/utils.cpp)
find_package(libgit2)
find_package(tomlplusplus)

add_executable(${PROJECT_NAME} ${PLUS_SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC ${STRUCTOPT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ./src/include)

target_link_libraries(${PROJECT_NAME} libgit2::libgit2 tomlplusplus::tomlplusplus subprocess)

set(PLUS_CLANG_TIDY_CMD "")
if(PLUS_ENABLE_CLANG_TIDY)
  find_program(
    CLANG_TIDY_EXECUTABLE
    NAMES clang-tidy clang-tidy-20 clang-tidy-19 clang-tidy-18 clang-tidy-17 clang-tidy-16
    DOC "Path to clang-tidy executable")
  if(NOT CLANG_TIDY_EXECUTABLE)
    message(FATAL_ERROR "PLUS_ENABLE_CLANG_TIDY is ON but clang-tidy was not found on PATH.")
  endif()
  # Semicolon-separated list for CMake's CXX_CLANG_TIDY property
  set(PLUS_CLANG_TIDY_CMD "${CLANG_TIDY_EXECUTABLE};-extra-arg=-Wno-unknown-warning-option")
  set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${PLUS_CLANG_TIDY_CMD}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # add_definitions("-DDEBUG")
  target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif()

if(BUILD_TESTING)
  add_executable(plus_tests
    tests/test_utils.cpp
    tests/test_config.cpp
    src/utils.cpp
    src/config.cpp
  )
  set_property(TARGET plus_tests PROPERTY CXX_STANDARD 17)
  target_include_directories(plus_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/include)
  target_link_libraries(plus_tests PRIVATE Catch2::Catch2WithMain tomlplusplus::tomlplusplus)
  if(PLUS_CLANG_TIDY_CMD)
    set_target_properties(plus_tests PROPERTIES CXX_CLANG_TIDY "${PLUS_CLANG_TIDY_CMD}")
  endif()
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(Catch)
  catch_discover_tests(plus_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS plus
  )
endif()