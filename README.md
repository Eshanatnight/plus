# Plus

A Small Utility App For C/CPP and CMake users.

## Prerequisites

- [vcpkg](https://github.com/microsoft/vcpkg)
- [cmake](https://cmake.org/)
- [premake](https://premake.github.io/)

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
    cmake -DCMAKE_TOOLCHAIN_FILE="<path>/<to>/<vcpkg>/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -G<generator> -S.. -B .
```

### If MSVC

```terminal
    cmake --build . --config Release
```

## Building with Premake

Download the premake5 executable. Then use

```bash
    premake5 <generator>
```

### Using Other Compilers to Build

As of the current MSVC `26.05.22`,
Currently for full support for `std::ranges` we need to use `/std:c++latest` which is directly not accessible from cmake.

For MSVC we have to set the standard to 23. (This is reflected in the CMakeLists.txt file)
Other compilers might need to modify the CMakeLists.txt file to set the standard to the latest or even the latest c++20.

More Information [here](https://stackoverflow.com/questions/64889383/how-to-enable-stdclatest-in-cmake)

----

## Testing

Unit tests use [Catch2](https://github.com/catchorg/Catch2) (fetched by CMake when `BUILD_TESTING` is on, which is the default). After configuring and building:

```bash
cd build
ctest -L plus --output-on-failure
```

Running plain `ctest` may also execute tests from dependencies (for example the `subprocess` library); the `plus` label restricts runs to this project’s cases. To skip tests and the Catch2 download, configure with `-DBUILD_TESTING=OFF`.

----

## Useage and Keyword Lookup

|Keyword|Command|
|---|---|
|`plus init`|Initialize a project repository in the current path|
|`plus new "Project_Name"`|Create a new Project in a new directory in the curent path|
|`plus add fmt/10.2.1`|Append a Conan requirement to `plus.toml` and refresh `conanfile.txt`|
|`plus deps`|Run `conan install` from `plus.toml` (writes `conanfile.txt` first)|
|`plus deps --sync-only`|Only regenerate `conanfile.txt` (no network)|
|`plus setup --conan`|Conan install + CMake configure using the Conan toolchain|
|`plus help`|Show this help message|

New projects include `[conan]` in `plus.toml` (empty `requires` by default), `conanfile.txt`, and `configure.sh` (same flow as this repo: Conan 2 + CMakeDeps + `cmake_layout`). Set `[conan].output_folder` if you want something other than `deps`.

## Features

- [x] Initialize a git repository
- [x] add a .gitignore file
- [x] add a .gitattribute file
- [x] add a CMakeLists.txt file
- [x] create an output directory

## Libs

- [libfmt](https://github.com/fmtlib/fmt)

- [libgit2](https://libgit2.org)

## Todo

- [ ] Add [Ranges-v3](https://github.com/ericniebler/range-v3) for Clang/GNU Compilers.
