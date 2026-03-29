# Plus

**Plus** is a small CLI for C++ projects: scaffold layouts and a `plus.toml` manifest (similar in spirit to Rust’s Cargo), drive **CMake**, and optionally manage dependencies with **Conan 2**.

## Prerequisites

- **CMake** 3.10+
- **C++17** toolchain
- To build **plus** itself, either:
  - **[Conan 2](https://docs.conan.io/)** (recommended for this repo — see *Building with Conan*), or
  - **[vcpkg](https://github.com/microsoft/vcpkg)** with `libgit2` and `tomlplusplus`
- **Conan 2** (optional) — only needed if you use `plus deps`, `plus setup --conan`, or generated `configure.sh` in your own projects
- **clang-format** (optional) — for `plus fmt`
- **Git** — expected on `PATH` when creating projects without `--no-git`

## Building with Conan (recommended)

From the repository root:

```bash
./configure.sh dbg   # or: rel
```

This runs `conan install` into `deps/` and configures CMake with the generated toolchain (see `configure.sh`).

Then:

```bash
cmake --build build
```

The resulting binary is `build/plus` (or under your generator’s output directory).

## Building with CMake and vcpkg

If you prefer vcpkg for **plus**’s own dependencies:

```bash
git clone https://github.com/Eshanatnight/plus.git
cd plus
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE="<path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake" \
      -DCMAKE_BUILD_TYPE=Release \
      -G "<generator>" -S .. -B .
cmake --build . --config Release   # MSVC: add --config Release
```

## Testing

Unit tests use [Catch2](https://github.com/catchorg/Catch2) (fetched by CMake when `BUILD_TESTING` is on, which is the default):

```bash
cd build
ctest -L plus --output-on-failure
```

Use **`ctest -L plus`** so only **plus** tests run; a plain `ctest` may also run tests from dependencies (e.g. **subprocess**). To skip tests and the Catch2 fetch:

```bash
cmake -DBUILD_TESTING=OFF ...
```

---

## Usage

| Command | Description |
|--------|-------------|
| `plus new <name>` | Create a new project in `./<name>/` |
| `plus new <name> --lib` | Same, as a static library layout (`include/`, `src/`) |
| `plus new <name> --no-git` | Skip initializing a Git repository |
| `plus init` | Scaffold in the current directory |
| `plus init --lib` / `--no-git` | Library template and/or no Git |
| `plus setup` | Configure CMake (`cmake -B <buildDir> -S .`) |
| `plus setup --conan` | `conan install` then CMake with Conan’s toolchain |
| `plus setup --type rel` | Use Release for Conan/CMake where applicable |
| `plus build` / `plus build --type rel` | `cmake --build` |
| `plus run` | Configure if needed, build, run the binary (`bin` projects only) |
| `plus clean` | Remove the configured build directory |
| `plus test` | Run **ctest** in the build tree |
| `plus fmt` | Run **clang-format** on `src/`, `include/`, `tests/`, `test/` |
| `plus fmt --check` | Check formatting only (`--dry-run --Werror`) |
| `plus show` | Print manifest summary; `plus show --verbose` lists Conan requires and author |
| `plus add <ref>` | Append a Conan requirement (e.g. `fmt/10.2.1`) to `plus.toml` and refresh `conanfile.txt` |
| `plus deps` | Write `conanfile.txt` and run `conan install` |
| `plus deps --sync-only` | Only regenerate `conanfile.txt` |
| `plus help` | CLI help |

### Manifest and Conan

- **`plus.toml`** holds project metadata and, under **`[conan]`**, **`requires`** and **`output_folder`** (default `deps`).
- New projects include **`conanfile.txt`** (CMakeDeps, CMakeToolchain, `cmake_layout`) and **`configure.sh`** for the same Conan → CMake flow as this repository.
- After `plus deps` or `plus setup --conan`, add **`find_package`** / **`target_link_libraries`** in **`CMakeLists.txt`** for each dependency as usual for **CMakeDeps**.

### Layout (scaffolded project)

- `plus.toml` — manifest  
- `src/` — sources (`main.cpp` or library `.cpp`)  
- `include/<slug>/` — public headers (library projects)  
- `build/` — CMake build tree (ignored by default)  
- `conanfile.txt`, `configure.sh` — Conan integration  
- `README.md`, `LICENSE`, `.gitignore`, `.gitattributes`

---

## Features

- [x] `plus.toml` project manifest (name, version, `cpp_std`, `kind`, `buildDir`, `cmakeDefines`, author, Conan block)
- [x] `plus new` / `plus init` (binary or library), optional `--no-git`
- [x] Git init via **libgit2** (unless `--no-git`)
- [x] CMake scaffold (C++ standard from manifest)
- [x] Conan: `plus add`, `plus deps`, `plus setup --conan`, generated `conanfile.txt` and `configure.sh`
- [x] `plus setup`, `plus build`, `plus run`, `plus clean`, `plus test`, `plus fmt`, `plus show`

## Libraries (plus binary)

| Library | Role |
|--------|------|
| [libgit2](https://libgit2.org) | Git repository initialization |
| [toml++](https://github.com/marzer/tomlplusplus) | Parse and emit `plus.toml` |
| [structopt](https://github.com/Eshanatnight/structopt) | CLI parsing (FetchContent) |
| [subprocess](https://github.com/Eshanatnight/subprocess) | Running cmake, conan, ctest, etc. (FetchContent) |
| [Catch2](https://github.com/catchorg/Catch2) | Unit tests only (FetchContent, when `BUILD_TESTING` is on) |

## See also

- [CMake `CMAKE_CXX_STANDARD` / MSVC “latest”](https://stackoverflow.com/questions/64889383/how-to-enable-stdclatest-in-cmake) — if you need a newer language mode than the scaffold’s `cpp_std` field.
