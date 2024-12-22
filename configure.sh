#! /usr/bin/env bash

if [[ ! -d ./build ]]; then
	printf "Build Folder not found. \nCreating...."
	mkdir build
fi

printf "Configuring conan"
# TODO: deal with release compiling
conan install . -sbuild_type=Debug --build=missing --output-folder=deps

printf "Configuring CMake"

cmake -B ./build -S . -GNinja -DCMAKE_TOOLCHAIN_FILE=./deps/build/Debug/generators/conan_toolchain.cmake
