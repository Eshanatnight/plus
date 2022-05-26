# Plus

A Small Utility App For C/CPP and CMake users.

## Prerequisites

- [vcpkg](https://github.com/microsoft/vcpkg)
- [cmake](https://cmake.org/)

## Building with cmake and vcpkg

```terminal
    git clone https://github.com/Eshanatnight/plus.git
```

```terminal
    cd plus
```

```terminal
    mkdir build
```

```terminal
    cd build
```

```terminal
    cmake -DCMAKE_TOOLCHAIN_FILE="<path>/<to>/<vcpkg>/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -S.. -B .
```

```terminal
    cmake --build . --config Release
```

----

## Useage and Keyword Lookup

|Keyword|Command|
|---|---|
|`plus init`|Initialize a project repository in the current path|
|`plus new "Project_Name"`|Create a new Project in a new directory in the curent path|
|`plus help`|Show this help message|

## Features

- [x] Initialize a git repository
- [x] add a .gitignore file
- [x] add a .gitattribute file
- [x] add a CMakeLists.txt file
- [x] create an output directory

## Libs

- [libfmt](https://github.com/fmtlib/fmt)

- [libgit2](https://libgit2.org)
