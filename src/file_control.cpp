// Command routing, project scaffolding, CMake/Conan integration.
#include "file_control.h"

#include "config.h"
#include "control.h"
#include "data.h"
#include "git.h"
#include "log.h"
#include "plus/diagnostics.hpp"
#include "subprocess/basic_types.hpp"
#include "subprocess/ProcessBuilder.hpp"
#include "utils.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <subprocess.hpp>
#include <toml++/toml.hpp>
#include <vector>

namespace {

	void replace_all(std::string& s, std::string_view from, const std::string& to) {
		std::size_t pos = 0;
		while((pos = s.find(from, pos)) != std::string::npos) {
			s.replace(pos, from.length(), to);
			pos += to.length();
		}
	}

	auto current_year_string() -> std::string {
		const std::time_t t = std::time(nullptr);
		const std::tm* tm	= std::localtime(&t);
		if(tm == nullptr) {
			return "2026";
		}
		return std::to_string(tm->tm_year + 1900);
	}

	auto build_readme(const Config& conf) -> std::string {
		std::string out = "# ";
		out += conf.proj.name;
		out += "\n\n";
		out += "C++ project scaffolded with [plus](https://github.com/Eshanatnight/plus) "
			   "(Cargo-style layout: `plus.toml`, `src/`, CMake).\n\n";
		out += "## Build\n\n";
		out += "```bash\nplus setup              # CMake only\nplus setup --conan      # conan "
			   "install + CMake with toolchain\nplus deps               # conan install from "
			   "plus.toml\nplus deps --sync-only   # only write conanfile.txt\nplus add fmt/10.2.1 "
			   "    # append Conan require to plus.toml\nplus build\nplus run\nplus test\nplus "
			   "fmt\nplus clean\n```\n\n";
		out += "## Conan dependencies\n\n";
		out += "List packages under `[conan].requires` in `plus.toml` (e.g. `fmt/10.2.1`), run "
			   "`plus deps` or `./configure.sh dbg`, then use `find_package` / "
			   "`target_link_libraries` in `CMakeLists.txt` as usual with **CMakeDeps**.\n\n";
		out += "## Layout\n\n";
		out += "- `plus.toml` — project manifest\n";
		out += "- `src/` — source files\n";
		if(conf.proj.kind == "lib") {
			out += "- `include/` — public headers\n";
		}
		out += "- `build/` — CMake build tree (gitignored)\n";
		return out;
	}

	auto build_license_mit(const Config& conf) -> std::string {
		std::string holder = conf.author.name;
		if(holder.empty()) {
			holder = conf.proj.name;
		}
		std::string out = "MIT License\n\n";
		out += "Copyright (c) ";
		out += current_year_string();
		out += ' ';
		out += holder;
		out += R"(

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
)";
		return out;
	}

	auto build_lib_header(const std::string& ns) -> std::string {
		std::string out = "#pragma once\n\nnamespace ";
		out += ns;
		out += " {\n\nvoid greet();\n\n}  // namespace ";
		out += ns;
		out += "\n";
		return out;
	}

	auto build_lib_cpp(const Config& conf, const std::string& slug, const std::string& ns)
		-> std::string {
		std::string out = "#include \"";
		out += slug;
		out += '/';
		out += slug;
		out += ".h\"\n\n#include <iostream>\n\nnamespace ";
		out += ns;
		out += " {\n\nvoid greet() {\n    std::cout << \"Hello from ";
		out += conf.proj.name;
		out += "\\n\";\n}\n\n}  // namespace ";
		out += ns;
		out += "\n";
		return out;
	}

	auto materialize_cmake_bin(const Config& conf, const std::string& cmake_project)
		-> std::string {
		std::string cm = FileContents::cmakeBin;
		replace_all(cm, Ident::plusMyAPP, cmake_project);
		replace_all(cm, Ident::plusCppStd, conf.proj.cpp_std);
		return cm;
	}

	auto materialize_cmake_lib(
		const Config& conf, const std::string& cmake_project, const std::string& slug)
		-> std::string {
		std::string cm = FileContents::cmakeLib;
		replace_all(cm, Ident::plusMyAPP, cmake_project);
		replace_all(cm, Ident::plusCppStd, conf.proj.cpp_std);
		replace_all(cm, Ident::plusSlug, slug);
		return cm;
	}

	auto run_cmake_configure(const std::filesystem::path& pwd,
		const Config& conf,
		const std::optional<std::filesystem::path>& toolchain_file = std::nullopt) -> bool {
		namespace fs		= std::filesystem;
		const auto buildDir = pwd / conf.proj.buildDir;
		fs::create_directories(buildDir);
		std::string defs;

		std::for_each(conf.proj.cmakeDefines.begin(),
			conf.proj.cmakeDefines.end(),
			[&defs](const auto& elem) {
				defs += "-D";
				defs += elem;
				defs += ' ';
			});

		subprocess::CommandLine cmd{ "cmake", "-B", buildDir.string(), "-S", pwd.string() };
		if(toolchain_file.has_value() && !toolchain_file->empty()) {
			const auto absTc = fs::absolute(*toolchain_file);
			cmd.emplace_back(std::string("-DCMAKE_TOOLCHAIN_FILE=") + absTc.string());
		}
		if(!defs.empty()) {
			cmd.emplace_back(defs);
		}

		return static_cast<bool>(subprocess::run(cmd));
	}

	auto write_conanfile_txt(const std::filesystem::path& project_root, const Config& conf)
		-> bool {
		std::ofstream f(project_root / "conanfile.txt");
		if(!f.is_open()) {
			plus::diag::error("could not write conanfile.txt.\n");
			return false;
		}
		f << "[requires]\n";
		for(const auto& r: conf.conan.requires) {
			f << r << '\n';
		}
		f << "\n[generators]\nCMakeDeps\nCMakeToolchain\n\n[layout]\ncmake_layout\n";
		return true;
	}

	auto conan_toolchain_path(
		const std::filesystem::path& project_root, const Config& conf, Cli::BuildType t)
		-> std::filesystem::path {
		const std::string cfg(buildTypeConfig(t));
		return project_root / conf.conan.output_folder / "build" / cfg / "generators" /
			   "conan_toolchain.cmake";
	}

	auto run_conan_install(
		const std::filesystem::path& project_root, const Config& conf, Cli::BuildType t) -> bool {
		write_conanfile_txt(project_root, conf);
		subprocess::RunOptions opts;
		opts.cwd = project_root.string();
		const std::string bt(buildTypeConfig(t));
		const auto cmd = subprocess::CommandLine{ "conan",
			"install",
			".",
			"-s",
			std::string("build_type=") + bt,
			"--build=missing",
			std::string("--output-folder=") + conf.conan.output_folder };
		try {
			return static_cast<bool>(subprocess::run(cmd, opts));
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "conan install failed: " << ex.what() << '\n';
			return false;
		}
	}

	auto save_plus_toml(const std::filesystem::path& project_root, const Config& conf) -> bool {
		std::ofstream f(project_root / FilePaths::PLUSTOML);
		if(!f.is_open()) {
			plus::diag::error("could not write plus.toml.\n");
			return false;
		}
		f << toml::toml_formatter(conf.toTomlTable());
		return true;
	}

	auto trim_package_ref(std::string s) -> std::string {
		while(!s.empty() && std::isspace(static_cast<unsigned char>(s.front())) != 0) {
			s.erase(s.begin());
		}
		while(!s.empty() && std::isspace(static_cast<unsigned char>(s.back())) != 0) {
			s.pop_back();
		}
		return s;
	}

	auto build_configure_sh(const Config& conf) -> std::string {
		const std::string& dep = conf.conan.output_folder;
		std::string s		   = "#!/usr/bin/env bash\nset -euo pipefail\n";
		s += "if [[ \"${1:-}\" != \"dbg\" && \"${1:-}\" != \"rel\" ]]; then\n";
		s += "  echo \"Usage: ./configure.sh <dbg|rel>\" >&2\n  exit 1\nfi\n";
		s += "case \"$1\" in dbg) BT=Debug ;; rel) BT=Release ;; esac\n";
		s += "mkdir -p build\n";
		s += "echo \"Running Conan...\"\n";
		s +=
			"conan install . -s \"build_type=${BT}\" --build=missing --output-folder=" + dep + "\n";
		s += "echo \"Configuring CMake...\"\n";
		s += "TOOLCHAIN=\"./" + dep + "/build/${BT}/generators/conan_toolchain.cmake\"\n";
		s += "cmake -B ./build -S . \"-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN}\" "
			 "\"-DCMAKE_BUILD_TYPE=${BT}\" "
			 "\"$@\"\n";
		return s;
	}

	auto run_cmake_build(const std::filesystem::path& pwd, const Config& conf, Cli::BuildType t)
		-> bool {
		const auto buildDir = (pwd / conf.proj.buildDir).string();
		const std::string cfg(buildTypeConfig(t));
		const auto process = subprocess::run(
			subprocess::CommandLine{ "cmake", "--build", buildDir, "--config", cfg });
		return static_cast<bool>(process);
	}

	auto cmake_cache_path(const std::filesystem::path& pwd, const Config& conf)
		-> std::filesystem::path {
		return pwd / conf.proj.buildDir / "CMakeCache.txt";
	}

	auto find_built_executable(const std::filesystem::path& pwd, const Config& conf)
		-> std::optional<std::filesystem::path> {
		namespace fs			  = std::filesystem;
		const std::string slug	  = utils::project_slug(conf.proj.name);
		const fs::path root		  = pwd / conf.proj.buildDir;
		const std::string slugExe = slug + ".exe";

		const fs::path candidates[] = {
			root / slug,
			root / slugExe,
			root / "Debug" / slug,
			root / "Debug" / slugExe,
			root / "Release" / slug,
			root / "Release" / slugExe,
			root / "RelWithDebInfo" / slugExe,
			root / "MinSizeRel" / slugExe,
		};

		for(const auto& p: candidates) {
			if(fs::exists(p) && fs::is_regular_file(p)) {
				return p;
			}
		}

		if(fs::exists(root) && fs::is_directory(root)) {
			for(const auto& entry: fs::directory_iterator(root)) {
				if(!entry.is_directory()) {
					continue;
				}
				for(const auto& name: { slug, slugExe }) {
					const auto p = entry.path() / name;
					if(fs::exists(p) && fs::is_regular_file(p)) {
						return p;
					}
				}
			}
		}

		return std::nullopt;
	}

	void collect_fmt_files(
		const std::filesystem::path& base, std::vector<std::filesystem::path>& out) {
		namespace fs = std::filesystem;
		static const std::set<std::string> exts{ ".cpp", ".cxx", ".cc", ".h", ".hpp", ".inl" };

		if(!fs::exists(base)) {
			return;
		}

		for(const auto& entry:
			fs::recursive_directory_iterator(base, fs::directory_options::skip_permission_denied)) {
			if(!entry.is_regular_file()) {
				continue;
			}
			const auto p	= entry.path();
			std::string ext = p.extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});
			if(exts.count(ext) == 0u) {
				continue;
			}
			out.push_back(p);
		}
	}

	[[nodiscard]] auto subprocess_ok(const subprocess::CompletedProcess& proc) -> bool {
		return static_cast<bool>(proc);
	}

	auto run_command_init(const Cli& cli, std::filesystem::path& pwd, std::string& appName)
		-> InitializationError {
		const auto destToml = pwd / FilePaths::PLUSTOML;
		if(std::filesystem::exists(destToml)) {
			plus::diag::error("`plus.toml` already exists in this directory.\n");
			return InitializationError::INVALID_ARG;
		}

		if(!skipGit(cli)) {
			initialize_git_repository(pwd, false);
		}
		appName = pwd.filename().string();
		if(appName.empty() || appName == "." || appName == "..") {
			appName = pwd.lexically_normal().filename().string();
		}

		std::string_view packageType = "bin";
		if(cli.init.kind.has_value()) {
			packageType = TypeToString(cli.init.kind.value());
		}

		flushNewConfig(cli, pwd, appName, packageType);
		return InitializationError::OK;
	}

	auto run_command_new(const Cli& cli, std::filesystem::path& pwd, std::string& appName)
		-> InitializationError {
		appName				  = cli.new_.projectName;
		const auto projectDir = pwd / cli.new_.projectName;
		if(std::filesystem::exists(projectDir)) {
			plus::diag::error_stream()
				<< "destination `" << projectDir.string() << "` already exists.\n";
			return InitializationError::INVALID_ARG;
		}

		pwd = projectDir;
		if(!skipGit(cli)) {
			initialize_git_repository(pwd, true);
		}

		std::string_view packageType = "bin";
		if(cli.new_.kind.has_value()) {
			packageType = TypeToString(cli.new_.kind.value());
		}

		flushNewConfig(cli, pwd, appName, packageType);
		return InitializationError::OK;
	}

	auto run_command_build(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found (not a plus project directory?).\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		const auto t = cli.build.type.value_or(Cli::BuildType::dbg);
		if(!run_cmake_build(pwd, conf, t)) {
			plus::diag::error("build failed.\n");
			return InitializationError::COMMAND_FAILED;
		}
		return InitializationError::OK;
	}

	auto run_command_run(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		Config conf(pwd / FilePaths::PLUSTOML);
		if(conf.proj.kind == "lib") {
			plus::diag::error("`plus run` applies only to binary (`bin`) packages.\n");
			return InitializationError::INVALID_ARG;
		}

		if(!std::filesystem::exists(pwd / FilePaths::CMAKELISTS)) {
			plus::diag::error("`CMakeLists.txt` not found.\n");
			return InitializationError::CMAKELISTS_NOT_FOUND;
		}

		const auto t = cli.run.type.value_or(Cli::BuildType::dbg);

		if(!std::filesystem::exists(cmake_cache_path(pwd, conf))) {
			if(!run_cmake_configure(pwd, conf)) {
				plus::diag::error("CMake configure failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		}

		if(!run_cmake_build(pwd, conf, t)) {
			plus::diag::error("build failed.\n");
			return InitializationError::COMMAND_FAILED;
		}

		const auto exe = find_built_executable(pwd, conf);
		if(!exe.has_value()) {
			plus::diag::error_stream()
				<< "could not find built executable under `" << conf.proj.buildDir << "`.\n";
			return InitializationError::COMMAND_FAILED;
		}

		subprocess::RunOptions run_opts;
		run_opts.cwd		= pwd.string();
		const auto run_proc = subprocess::run(subprocess::CommandLine{ exe->string() }, run_opts);
		if(!subprocess_ok(run_proc)) {
			return InitializationError::COMMAND_FAILED;
		}
		return InitializationError::OK;
	}

	auto run_command_clean(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		const auto build_dir = pwd / conf.proj.buildDir;
		if(!std::filesystem::exists(build_dir)) {
			if(!cli.clean.quiet.value_or(false)) {
				std::cout << "Nothing to clean (build directory missing).\n";
			}
			return InitializationError::OK;
		}

		const auto n = std::filesystem::remove_all(build_dir);
		if(!cli.clean.quiet.value_or(false)) {
			std::cout << "Removed `" << build_dir.string() << "` (" << n << " entries).\n";
		}
		return InitializationError::OK;
	}

	auto run_command_fmt(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		std::vector<std::filesystem::path> files;
		collect_fmt_files(pwd / FilePaths::SRC_PATH, files);
		collect_fmt_files(pwd / FilePaths::INC_PATH, files);
		collect_fmt_files(pwd / "tests", files);
		collect_fmt_files(pwd / "test", files);

		if(files.empty()) {
			plus::diag::error("no source files under src/, include/, tests/, or test/.\n");
			return InitializationError::INVALID_ARG;
		}

		subprocess::CommandLine cmd;
		cmd.emplace_back("clang-format");
		if(cli.fmt.check.value_or(false)) {
			cmd.emplace_back("--dry-run");
			cmd.emplace_back("--Werror");
		} else {
			cmd.emplace_back("-i");
		}
		for(const auto& fpath: files) {
			cmd.emplace_back(fpath.string());
		}

		try {
			if(!subprocess_ok(subprocess::run(cmd))) {
				plus::diag::error("clang-format reported formatting issues.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "failed to run clang-format: " << ex.what() << '\n';
			return InitializationError::COMMAND_FAILED;
		}

		if(!cli.fmt.check.value_or(false)) {
			std::cout << "Formatted " << files.size() << " file(s).\n";
		}
		return InitializationError::OK;
	}

	auto run_command_show(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		std::cout << "package: " << conf.proj.name << "\n";
		std::cout << "version: " << conf.proj.version << "\n";
		std::cout << "kind: " << conf.proj.kind << "\n";
		std::cout << "cpp_std: " << conf.proj.cpp_std << "\n";
		std::cout << "build_dir: " << conf.proj.buildDir << "\n";
		std::cout << "conan_output_folder: " << conf.conan.output_folder << "\n";
		std::cout << "conan_requirements: "
				  << conf.conan.
						 requires
			.size() << "\n";
		if(cli.show.verbose.value_or(false)) {
			for(const auto& r: conf.conan.requires) {
				std::cout << "  - " << r << '\n';
			}
			if(!conf.author.name.empty() || !conf.author.email.empty()) {
				std::cout << "author: " << conf.author.name << " <" << conf.author.email << ">\n";
			}
			if(!conf.proj.repo.empty()) {
				std::cout << "repo: " << conf.proj.repo << "\n";
			}
		}
		return InitializationError::OK;
	}

	auto run_command_test(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		Config conf(pwd / FilePaths::PLUSTOML);
		const auto build_dir = pwd / conf.proj.buildDir;

		if(!std::filesystem::exists(cmake_cache_path(pwd, conf))) {
			if(!std::filesystem::exists(pwd / FilePaths::CMAKELISTS)) {
				plus::diag::error("`CMakeLists.txt` not found.\n");
				return InitializationError::CMAKELISTS_NOT_FOUND;
			}
			if(!run_cmake_configure(pwd, conf)) {
				plus::diag::error("CMake configure failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		}

		if(!std::filesystem::exists(build_dir)) {
			plus::diag::error("build directory missing; run `plus build` first.\n");
			return InitializationError::BUILD_DIR_DOES_NOT_EXIST;
		}

		const auto t = cli.test.type.value_or(Cli::BuildType::dbg);
		const std::string cfg(buildTypeConfig(t));

		try {
			const auto proc = subprocess::run(subprocess::CommandLine{
				"ctest", "--test-dir", build_dir.string(), "--output-on-failure", "-C", cfg });
			if(!subprocess_ok(proc)) {
				return InitializationError::COMMAND_FAILED;
			}
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "failed to run ctest: " << ex.what() << '\n';
			return InitializationError::COMMAND_FAILED;
		}

		return InitializationError::OK;
	}

	auto run_command_deps(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		const auto plus_toml = pwd / FilePaths::PLUSTOML;
		if(!std::filesystem::exists(plus_toml)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		Config conf(plus_toml);

		if(cli.deps.sync_only.value_or(false)) {
			if(!write_conanfile_txt(pwd, conf)) {
				return InitializationError::COMMAND_FAILED;
			}
			std::cout << "Wrote `conanfile.txt` from `plus.toml`.\n";
			return InitializationError::OK;
		}

		const auto t = cli.deps.type.value_or(Cli::BuildType::dbg);
		if(!run_conan_install(pwd, conf, t)) {
			plus::diag::error("`conan install` failed.\n");
			return InitializationError::COMMAND_FAILED;
		}
		std::cout << "Conan install finished (`--output-folder=" << conf.conan.output_folder
				  << "`).\n";
		return InitializationError::OK;
	}

	auto run_command_add(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		const auto plus_toml = pwd / FilePaths::PLUSTOML;
		if(!std::filesystem::exists(plus_toml)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		std::string ref = trim_package_ref(cli.add.package_ref);
		if(ref.empty()) {
			plus::diag::error("package reference is empty (expected e.g. `fmt/10.2.1`).\n");
			return InitializationError::INVALID_ARG;
		}

		Config conf(plus_toml);
		for(const auto& existing: conf.conan.requires) {
			if(existing == ref) {
				std::cout << "Requirement `" << ref << "` is already listed in plus.toml.\n";
				write_conanfile_txt(pwd, conf);
				return InitializationError::OK;
			}
		}

		conf.conan.
			requires
			.push_back(std::move(ref));
		if(!save_plus_toml(pwd, conf) || !write_conanfile_txt(pwd, conf)) {
			return InitializationError::COMMAND_FAILED;
		}
		std::cout
			<< "Added Conan requirement; updated `conanfile.txt`. Run `plus deps` to install.\n";
		return InitializationError::OK;
	}

	auto run_command_setup(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		const auto plus_toml = pwd / FilePaths::PLUSTOML;
		if(!std::filesystem::exists(plus_toml)) {
			plus::diag::error("`plus.toml` not found (not a plus project directory?).\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		if(!std::filesystem::exists(pwd / FilePaths::CMAKELISTS)) {
			plus::diag::error("`CMakeLists.txt` not found.\n");
			return InitializationError::CMAKELISTS_NOT_FOUND;
		}

		Config conf(plus_toml);
		const auto build_dir = pwd / conf.proj.buildDir;
		std::filesystem::create_directories(build_dir);

		const auto t = cli.setup.type.value_or(Cli::BuildType::dbg);

		if(cli.setup.conan.value_or(false)) {
			if(!run_conan_install(pwd, conf, t)) {
				plus::diag::error("`conan install` failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
			const auto tc = conan_toolchain_path(pwd, conf, t);
			if(!std::filesystem::exists(tc)) {
				plus::diag::error_stream() << "Conan toolchain not found at `" << tc.string()
										   << "` (check `[conan].output_folder` in plus.toml).\n";
				return InitializationError::COMMAND_FAILED;
			}
			if(!run_cmake_configure(pwd, conf, tc)) {
				plus::diag::error("CMake configure failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} else {
			if(!run_cmake_configure(pwd, conf)) {
				plus::diag::error("CMake configure failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		}
		return InitializationError::OK;
	}

} // namespace

auto initialize_and_run(const Cli& cli, std::filesystem::path& pwd, std::string& appName)
	-> InitializationError {

	if(cli.init.has_value()) {
		return run_command_init(cli, pwd, appName);
	}
	if(cli.new_.has_value()) {
		return run_command_new(cli, pwd, appName);
	}
	if(cli.build.has_value()) {
		return run_command_build(cli, pwd);
	}
	if(cli.run.has_value()) {
		return run_command_run(cli, pwd);
	}
	if(cli.clean.has_value()) {
		return run_command_clean(cli, pwd);
	}
	if(cli.fmt.has_value()) {
		return run_command_fmt(cli, pwd);
	}
	if(cli.show.has_value()) {
		return run_command_show(cli, pwd);
	}
	if(cli.test.has_value()) {
		return run_command_test(cli, pwd);
	}
	if(cli.deps.has_value()) {
		return run_command_deps(cli, pwd);
	}
	if(cli.add.has_value()) {
		return run_command_add(cli, pwd);
	}
	if(cli.setup.has_value()) {
		return run_command_setup(cli, pwd);
	}
	return InitializationError::UNKNOWN;
}

auto flushNewConfig(const Cli& cli,
	const std::filesystem::path& pwd,
	const std::string& appName,
	std::string_view packageType) -> bool {

	Config conf = Config(appName, packageType, std::string(FilePaths::BUILD_PATH));

	const auto tbl = conf.toTomlTable();

	std::ofstream tomlFile(pwd / FilePaths::PLUSTOML);

	tomlFile << toml::toml_formatter(tbl);

	std::vector<std::future<void>> futures;
	futures.reserve(8);
	futures.emplace_back(makeBuildDir(pwd));
	makeFiles(futures, pwd, conf, isLib(cli));
	resolve(futures);

	std::cout << "Created C++ package `" << appName << "` (" << conf.proj.kind << ") at `"
			  << std::filesystem::absolute(pwd).string() << "`\n";

	return true;
}

auto InstPathToFilePath(InstPath file, const std::filesystem::path& basePath)
	-> std::filesystem::path {

	switch(file) {
	case InstPath::MAINCPP :
		return basePath / FilePaths::SRC_PATH / FilePaths::MAIN_CPP;
	case InstPath::GITIGNORE :
		return basePath / FilePaths::GITIGNORE;
	case InstPath::CMAKELISTS :
		return basePath / FilePaths::CMAKELISTS;
	case InstPath::GITATTRIBUTES :
		return basePath / FilePaths::GITATTRIBUTES;
	case InstPath::README :
		return basePath / FilePaths::README_MD;
	case InstPath::LICENSE :
		return basePath / FilePaths::LICENSE;
	case InstPath::CONFIGURE_SH :
		return basePath / FilePaths::CONFIGURE_SH;
	}
}

auto write_scaffold_file(
	const std::filesystem::path& basePath, InstPath instPath, std::string_view content) -> bool {
	namespace fs	= std::filesystem;
	const auto path = InstPathToFilePath(instPath, basePath);
	fs::create_directories(path.parent_path());
	std::ofstream ofs(path);

	if(!ofs.is_open()) {
		plus::diag::error_stream() << "could not open file for writing: " << path << '\n';
		return false;
	}

	ofs << content;
	ofs.close();

	return true;
}

auto makeBuildDir(const std::filesystem::path& basePath) -> std::future<void> {
	return std::async(std::launch::async, [&basePath]() {
		namespace fs = std::filesystem;
		// create the build dir
		fs::create_directories(basePath / FilePaths::BUILD_PATH);
	});
}

auto makeFiles(std::vector<std::future<void>>& futs,
	const std::filesystem::path& basePath,
	const Config& conf,
	bool isLib) -> void {

	const std::string slug			  = utils::project_slug(conf.proj.name);
	const std::string ns			  = utils::cpp_namespace_ident(conf.proj.name);
	const std::string cmake_project	  = slug;
	const std::string readme_content  = build_readme(conf);
	const std::string license_content = build_license_mit(conf);

	if(!isLib) {
		futs.push_back(std::async(std::launch::async, [&basePath]() {
			write_scaffold_file(basePath, InstPath::MAINCPP, FileContents::mainCPP);
		}));
	} else {
		const std::string header_content = build_lib_header(ns);
		const std::string cpp_content	 = build_lib_cpp(conf, slug, ns);

		futs.push_back(std::async(std::launch::async, [basePath, slug, header_content]() {
			namespace fs	= std::filesystem;
			const auto path = basePath / FilePaths::INC_PATH / slug / (slug + std::string(".h"));
			fs::create_directories(path.parent_path());
			std::ofstream ofs(path);
			if(!ofs.is_open()) {
				plus::diag::error_stream() << "could not open file for writing: " << path << '\n';
				return;
			}
			ofs << header_content;
		}));

		futs.push_back(std::async(std::launch::async, [basePath, slug, cpp_content]() {
			namespace fs	= std::filesystem;
			const auto path = basePath / FilePaths::SRC_PATH / (slug + std::string(".cpp"));
			fs::create_directories(path.parent_path());
			std::ofstream ofs(path);
			if(!ofs.is_open()) {
				plus::diag::error_stream() << "could not open file for writing: " << path << '\n';
				return;
			}
			ofs << cpp_content;
		}));
	}

	futs.push_back(std::async(std::launch::async, [&basePath]() {
		write_scaffold_file(basePath, InstPath::GITIGNORE, FileContents::gitIgnore);
	}));
	futs.push_back(std::async(std::launch::async, [&basePath]() {
		write_scaffold_file(basePath, InstPath::GITATTRIBUTES, FileContents::gitAttributes);
	}));
	futs.push_back(std::async(std::launch::async, [&basePath, readme_content]() {
		write_scaffold_file(basePath, InstPath::README, readme_content);
	}));
	futs.push_back(std::async(std::launch::async, [&basePath, license_content]() {
		write_scaffold_file(basePath, InstPath::LICENSE, license_content);
	}));

	futs.push_back(std::async(std::launch::async, [basePath, conf, isLib, cmake_project, slug]() {
		std::string result = isLib ? materialize_cmake_lib(conf, cmake_project, slug)
								   : materialize_cmake_bin(conf, cmake_project);
		write_scaffold_file(basePath, InstPath::CMAKELISTS, result);
	}));

	futs.push_back(std::async(std::launch::async, [basePath, conf]() {
		if(!write_conanfile_txt(basePath, conf)) {
			return;
		}
		const auto sh = build_configure_sh(conf);
		write_scaffold_file(basePath, InstPath::CONFIGURE_SH, sh);
	}));
}
