#! /usr/bin/env bash

if [[ $1 == "dbg" ]]; then
	buildType=Debug
elif [[ $1 == "rel" ]]; then
	buildType=Release
else
	printf "Useage: ./configure.sh <dbg | rel>"
	exit 1
fi

if [[ ! -d ./build ]]; then
	printf "Build Folder not found. \nCreating....\n"
	mkdir build
fi

printf "Configuring conan\n"
# TODO: deal with release compiling
conan install . -sbuild_type="${buildType}" --build=missing --output-folder=deps

printf "Configuring CMake\n"

cmake -B ./build -S . -GNinja -DCMAKE_TOOLCHAIN_FILE=./deps/build/Debug/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE="${buildType}"
