#!/usr/bin/env bash
set -euo pipefail
if [[ "${1:-}" != "dbg" && "${1:-}" != "rel" ]]; then
	echo "Usage: ./configure.sh <dbg|rel> [-- extra cmake args...]" >&2
	exit 1
fi
case "$1" in dbg) BT=Debug ;; rel) BT=Release ;; esac
shift
mkdir -p build
echo "Running Conan..."
conan install . -s "build_type=${BT}" --build=missing --output-folder=deps
echo "Configuring CMake..."
TOOLCHAIN="./deps/build/${BT}/generators/conan_toolchain.cmake"
cmake -B ./build -S . "-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN}" "-DCMAKE_BUILD_TYPE=${BT}" "$@"
