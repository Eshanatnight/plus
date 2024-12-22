cmake_minimum_required(VERSION 3.10)
project(plus)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include(FetchContent)

FetchContent_Declare(
  structopt
  GIT_REPOSITORY https://github.com/Eshanatnight/structopt
  GIT_TAG        v0.2.1
)

FetchContent_GetProperties(structopt)
FetchContent_MakeAvailable(structopt)

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

set(PLUS_SRC ./src/main.cpp ./src/git.cpp ./src/file_control.cpp)
find_package(libgit2)

add_executable(${PROJECT_NAME} ${PLUS_SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PUBLIC ${STRUCTOPT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ./src/include)

target_link_libraries(${PROJECT_NAME} libgit2::libgit2)
