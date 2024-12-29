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
CONAN_PROFILE=$(conan profile path default)

if [[ ! -f "${CONAN_PROFILE}" ]]; then
	printf "Conan profile not found. Creating...\n"
	conan profile detect --force
fi

conan install . -sbuild_type="${buildType}" --build=missing --output-folder=deps

printf "Configuring CMake\n"

CMAKE_MAKE_PROGRAM=$(which ninja)
CMAKE_C_COMPILER=$(which gcc)
CMAKE_CXX_COMPILER=$(which g++)

cmake -B ./build -S . -GNinja -DCMAKE_TOOLCHAIN_FILE=./deps/build/Debug/generators/conan_toolchain.cmake \
	-DCMAKE_BUILD_TYPE="${buildType}" \
	-DCMAKE_MAKE_PROGRAM="${CMAKE_MAKE_PROGRAM}" \
	-DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}" \
	-DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
