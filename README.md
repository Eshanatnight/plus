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
- **clang-tidy** (optional) — for `plus tidy` and the `PLUS_ENABLE_CLANG_TIDY` CMake option
- **Git** — expected on `PATH` when creating projects without `--no-git`

## Building with Conan (recommended)

This repository includes a root **`plus.toml`** (same manifest format **`plus`** uses everywhere). Dependencies are listed under **`[conan]`** and stay in sync with **`conanfile.txt`** via **`plus deps --sync-only`** or **`plus add`**.

### First-time bootstrap

You need a **`plus`** binary once (or plain CMake/Conan). Typical flow:

```bash
./configure.sh dbg   # or: rel — Conan into deps/, then CMake (optional: ./configure.sh dbg -- -G Ninja)
cmake --build build
```

### Day-to-day with the `plus` CLI

After **`build/plus`** exists, from the repository root:

```bash
plus setup --conan dbg    # or: rel — same idea as configure.sh
plus build
plus test
plus run -- <args>        # runs the built `plus` binary
plus tidy                 # needs compile_commands.json from configure
```

Plain CMake (no Conan toolchain) from a machine that already has **libgit2** and **tomlplusplus** on the search path:

```bash
plus setup
plus build
```

The resulting binary is **`build/plus`** (or under your generator’s output directory for multi-config IDEs).

**`plus setup --conan`** removes a stale **`build/CMakeCache.txt`** if present so CMake applies the Conan toolchain (an old cache from a non-Conan configure would otherwise ignore **`CMAKE_TOOLCHAIN_FILE`** and **`find_package`** would fail). If configure still looks wrong, run **`plus clean`** and try again.

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

## clang-tidy (static analysis)

This repository ships a root **`.clang-tidy`** configuration. CMake always sets **`CMAKE_EXPORT_COMPILE_COMMANDS=ON`**, so after configuring you get **`compile_commands.json`** in the build directory (with **Ninja** or **Unix Makefiles**; some multi-config generators omit it—use Ninja if `plus tidy` cannot find the database).

**While compiling plus** (optional):

```bash
cmake -DPLUS_ENABLE_CLANG_TIDY=ON -B build -S .
cmake --build build
```

**From the CLI** (in any plus project with `plus.toml`):

```bash
plus tidy              # clang-tidy on src/, tests/, test/
plus tidy --fix        # apply fix-its where clang-tidy supports them
```

`plus tidy` uses **`-p <absolute path to build dir>`** from **`project.buildDir`** in `plus.toml` (default `build`). It passes **`--header-filter`** so diagnostics are limited to headers under your project’s **`src/`**, **`include/`**, **`tests/`**, and **`test/`** (not **FetchContent** / **`build/_deps`** or Conan trees). Vendored directories named e.g. **`third_party`**, **`vendor`**, **`external`**, **`deps`**, **`_deps`** are skipped when collecting **`.cpp`** files. Ensure you have run **`plus setup`** (or CMake) at least once so **`compile_commands.json`** exists.

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
| `plus setup -g Ninja` / `--generator` | Same with `cmake -G …` (quote names with spaces, e.g. `"Unix Makefiles"`) |
| `plus setup --conan` | `conan install` then CMake with Conan’s toolchain |
| `plus setup --type rel` | Use Release for Conan/CMake where applicable |
| `plus build` / `plus build --type rel` | `cmake --build` |
| `plus build -j 8` / `--jobs` | `cmake --build … -j 8` (parallel compile jobs; CMake 3.12+) |
| `plus run` | Configure if needed, build, run the binary (`bin` projects only) |
| `plus run -g Ninja -j 8` | Optional configure generator (`-G`) and build parallelism |
| `plus clean` | Remove the configured build directory |
| `plus test` | Run **ctest** in the build tree |
| `plus test -g Ninja` | If configure is needed first, pass through `cmake -G …` |
| `plus fmt` | Run **clang-format** on `src/`, `include/`, `tests/`, `test/` |
| `plus fmt --check` | Check formatting only (`--dry-run --Werror`) |
| `plus tidy` | Run **clang-tidy** on project `.cpp` files (skips vendored dirs); diagnostics limited to project headers via **`--header-filter`** |
| `plus tidy --fix` | Same, applying supported fix-its |
| `plus show` | Print manifest summary; `plus show --verbose` lists Conan requires and author |
| `plus add <ref>` | Append a Conan requirement (e.g. `fmt/10.2.1`) to `plus.toml` and refresh `conanfile.txt` |
| `plus deps` | Write `conanfile.txt` and run `conan install` |
| `plus deps --sync-only` | Only regenerate `conanfile.txt` |
| `plus help` | CLI help |

### Manifest and Conan

- **`plus.toml`** holds project metadata and, under **`[conan]`**, **`requires`** and **`output_folder`** (default `deps`).
- New projects include **`conanfile.txt`** (CMakeDeps, CMakeToolchain, `cmake_layout`) and **`configure.sh`** for the same Conan → CMake flow as this repository.
- After `plus deps` or `plus setup --conan`, add **`find_package`** / **`target_link_libraries`** in **`CMakeLists.txt`** for each dependency as usual for **CMakeDeps**.

### Layout (this repo and scaffolded projects)

- `plus.toml` — manifest (this repository ships one at the root)  
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
- [x] `plus setup`, `plus build`, `plus run`, `plus clean`, `plus test`, `plus fmt`, `plus tidy`, `plus show`
- [x] Optional **clang-tidy** during build (`PLUS_ENABLE_CLANG_TIDY`) and **`compile_commands.json`** export

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
