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
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <system_error>
#include <set>
#include <string>
#include <string_view>
#include <subprocess.hpp>
#include <thread>
#include <toml++/toml.hpp>
#include <unordered_map>
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
			   "fmt\nplus tidy             # clang-tidy (needs compile_commands.json)\nplus clean\n"
			   "```\n\n";
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
		std::optional<std::filesystem::path> toolchain_file = std::nullopt,
		std::optional<Cli::BuildType> build_type		 = std::nullopt,
		std::optional<std::string> cmake_generator		 = std::nullopt) -> bool {
		namespace fs = std::filesystem;
		const auto buildDir = pwd / conf.proj.buildDir;
		fs::create_directories(buildDir);

		// Stale CMakeCache from a non-Conan configure causes CMake to ignore a new
		// CMAKE_TOOLCHAIN_FILE (warning: "Manually-specified variables were not used").
		if(toolchain_file.has_value() && !toolchain_file->empty()) {
			const auto cacheFile = buildDir / "CMakeCache.txt";
			if(fs::is_regular_file(cacheFile)) {
				std::error_code ec;
				fs::remove(cacheFile, ec);
			}
		}

		subprocess::CommandLine cmd{ "cmake", "-B", buildDir.string(), "-S", pwd.string() };
		if(cmake_generator.has_value() && !cmake_generator.value().empty()) {
			cmd.emplace_back("-G");
			cmd.emplace_back(cmake_generator.value());
		}
		if(toolchain_file.has_value() && !toolchain_file->empty()) {
			const auto absTc = fs::absolute(*toolchain_file);
			cmd.emplace_back(std::string("-DCMAKE_TOOLCHAIN_FILE=") + absTc.string());
			const Cli::BuildType bt = build_type.value_or(Cli::BuildType::dbg);
			cmd.emplace_back(std::string("-DCMAKE_BUILD_TYPE=") + std::string(buildTypeConfig(bt)));
		} else if(build_type.has_value()) {
			cmd.emplace_back(std::string("-DCMAKE_BUILD_TYPE=") +
							 std::string(buildTypeConfig(*build_type)));
		}

		for(const auto& elem: conf.proj.cmakeDefines) {
			if(!elem.empty()) {
				cmd.emplace_back(std::string("-D") + elem);
			}
		}

		subprocess::RunOptions opts;
		opts.cwd = pwd.string();
		return static_cast<bool>(subprocess::run(cmd, opts));
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
		s += "shift\n";
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

	auto run_cmake_build(const std::filesystem::path& pwd,
		const Config& conf,
		Cli::BuildType t,
		std::optional<int> jobs = std::nullopt) -> bool {
		const auto buildDir = (pwd / conf.proj.buildDir).string();
		const std::string cfg(buildTypeConfig(t));
		subprocess::CommandLine cmd{ "cmake", "--build", buildDir, "--config", cfg };
		if(jobs.has_value() && *jobs > 0) {
			cmd.emplace_back("-j");
			cmd.emplace_back(std::to_string(*jobs));
		}
		const auto process = subprocess::run(cmd);
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

	auto escape_for_regex(std::string_view s) -> std::string {
		static constexpr std::string_view specials = ".^$*+?()[]{}\\|";
		std::string out;
		out.reserve(s.size() + 8);
		for(const unsigned char uc: s) {
			const char c = static_cast<char>(uc);
			if(specials.find(c) != std::string_view::npos) {
				out.push_back('\\');
			}
			out.push_back(c);
		}
		return out;
	}

	/** `.cpp` files for clang-tidy: skips vendored trees (e.g. `src/vendor`, `third_party`). */
	void collect_project_cpp_sources(const std::filesystem::path& base, std::vector<std::filesystem::path>& out) {
		namespace fs = std::filesystem;
		static const std::set<std::string> exts{ ".cpp", ".cxx", ".cc" };
		static const std::set<std::string> skip_dir_lower{
			"third_party",
			"third-party",
			"vendor",
			"vendors",
			"external",
			"deps",
			"_deps",
			"vendored",
			"upstream",
			".git",
		};

		if(!fs::exists(base)) {
			return;
		}

		std::error_code ec;
		fs::path iter_root = fs::weakly_canonical(base, ec);
		if(ec) {
			iter_root = base;
		}

		for(fs::recursive_directory_iterator it(iter_root, fs::directory_options::skip_permission_denied);
			it != fs::recursive_directory_iterator{};
			++it) {
			if(it->is_directory()) {
				std::string dirname = it->path().filename().string();
				std::transform(dirname.begin(), dirname.end(), dirname.begin(), [](unsigned char c) {
					return static_cast<char>(std::tolower(c));
				});
				if(skip_dir_lower.count(dirname) != 0u) {
					it.disable_recursion_pending();
				}
				continue;
			}
			if(!it->is_regular_file()) {
				continue;
			}
			const auto p	= it->path();
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
		appName = cli.new_.projectName;
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
		if(!run_cmake_build(pwd, conf, t, cli.build.jobs)) {
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
			if(!run_cmake_configure(pwd, conf, std::nullopt,
					std::optional<Cli::BuildType>{ t }, cli.run.generator)) {
				plus::diag::error("CMake configure failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		}

		if(!run_cmake_build(pwd, conf, t, cli.run.jobs)) {
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

	auto run_command_tidy(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		namespace fs = std::filesystem;
		const auto build_dir	= fs::absolute(pwd / conf.proj.buildDir);
		const auto compile_json = build_dir / "compile_commands.json";
		if(!fs::exists(compile_json)) {
			plus::diag::error_stream()
				<< "`compile_commands.json` not found in `" << build_dir.string()
				<< "`. Configure with CMAKE_EXPORT_COMPILE_COMMANDS=ON (Ninja or Unix Makefiles) "
				   "and run `plus setup`.\n";
			return InitializationError::COMMAND_FAILED;
		}

		std::vector<fs::path> files;
		collect_project_cpp_sources(pwd / FilePaths::SRC_PATH, files);
		collect_project_cpp_sources(pwd / "tests", files);
		collect_project_cpp_sources(pwd / "test", files);

		if(files.empty()) {
			plus::diag::error("no .cpp sources under src/, tests/, or test/.\n");
			return InitializationError::INVALID_ARG;
		}

		subprocess::CommandLine cmd;
		cmd.emplace_back("clang-tidy");
		if(cli.tidy.fix.value_or(false)) {
			cmd.emplace_back("--fix");
		}
		cmd.emplace_back("-p");
		cmd.emplace_back(build_dir.string());
		// Only emit diagnostics for project headers under src/include/tests/test (not
		// FetchContent/Conan paths like build/_deps/.../src/...).
		{
			std::error_code ec;
			const fs::path root_abs = fs::weakly_canonical(pwd, ec);
			if(!ec) {
				std::string filter = "^";
				filter += escape_for_regex(root_abs.string());
				filter += R"((?:/|\\)(?:src|include|tests|test)(?:/|\\).*)";
				cmd.emplace_back(std::string("--header-filter=") + filter);
			}
		}
		for(const auto& fpath: files) {
			cmd.emplace_back(fpath.string());
		}

		try {
			subprocess::RunOptions opts;
			opts.cwd = pwd.string();
			if(!subprocess_ok(subprocess::run(cmd, opts))) {
				plus::diag::error("clang-tidy reported diagnostics.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "failed to run clang-tidy: " << ex.what() << '\n';
			return InitializationError::COMMAND_FAILED;
		}

		std::cout << "clang-tidy: checked " << files.size() << " translation unit(s).\n";
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
		std::cout << "conan_requirements: " << conf.conan.requires.size() << "\n";
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
		const auto t		 = cli.test.type.value_or(Cli::BuildType::dbg);

		if(!std::filesystem::exists(cmake_cache_path(pwd, conf))) {
			if(!std::filesystem::exists(pwd / FilePaths::CMAKELISTS)) {
				plus::diag::error("`CMakeLists.txt` not found.\n");
				return InitializationError::CMAKELISTS_NOT_FOUND;
			}
			if(!run_cmake_configure(pwd, conf, std::nullopt,
					std::optional<Cli::BuildType>{ t }, cli.test.generator)) {
				plus::diag::error("CMake configure failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		}

		if(!std::filesystem::exists(build_dir)) {
			plus::diag::error("build directory missing; run `plus build` first.\n");
			return InitializationError::BUILD_DIR_DOES_NOT_EXIST;
		}
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

		if(cli.deps.list.value_or(false)) {
			if(conf.conan.requires.empty()) {
				std::cout << "No Conan requirements.\n";
			} else {
				for(const auto& r: conf.conan.requires) {
					std::cout << "  " << r << '\n';
				}
			}
			return InitializationError::OK;
		}

		if(cli.deps.tree.value_or(false)) {
			write_conanfile_txt(pwd, conf);
			subprocess::RunOptions opts;
			opts.cwd = pwd.string();
			try {
				const auto proc = subprocess::run(
					subprocess::CommandLine{ "conan", "graph", "info", "." }, opts);
				if(!subprocess_ok(proc)) {
					plus::diag::error("conan graph info failed.\n");
					return InitializationError::COMMAND_FAILED;
				}
			} catch(const std::exception& ex) {
				plus::diag::error_stream()
					<< "failed to run conan graph info: " << ex.what() << '\n';
				return InitializationError::COMMAND_FAILED;
			}
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

		conf.conan.requires.push_back(std::move(ref));
		if(!save_plus_toml(pwd, conf) || !write_conanfile_txt(pwd, conf)) {
			return InitializationError::COMMAND_FAILED;
		}
		std::cout << "Added Conan requirement; updated `conanfile.txt`. Run `plus deps` to install.\n";
		return InitializationError::OK;
	}

	auto run_command_remove(const Cli& cli, const std::filesystem::path& pwd) -> InitializationError {
		const auto plus_toml = pwd / FilePaths::PLUSTOML;
		if(!std::filesystem::exists(plus_toml)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		std::string ref = trim_package_ref(cli.remove.package_ref);
		if(ref.empty()) {
			plus::diag::error("package reference is empty.\n");
			return InitializationError::INVALID_ARG;
		}

		Config conf(plus_toml);
		auto& reqs	 = conf.conan.requires;
		const auto it = std::find(reqs.begin(), reqs.end(), ref);
		if(it == reqs.end()) {
			plus::diag::error_stream() << "requirement `" << ref << "` not found in plus.toml.\n";
			return InitializationError::INVALID_ARG;
		}

		reqs.erase(it);
		if(!save_plus_toml(pwd, conf) || !write_conanfile_txt(pwd, conf)) {
			return InitializationError::COMMAND_FAILED;
		}
		std::cout << "Removed `" << ref << "` from plus.toml; updated `conanfile.txt`.\n";
		return InitializationError::OK;
	}

	auto run_command_check(const Cli& /*cli*/, const std::filesystem::path& pwd)
		-> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		const auto build_dir = pwd / conf.proj.buildDir;
		namespace fs		 = std::filesystem;

		// 1) fmt --check
		{
			std::vector<fs::path> files;
			collect_fmt_files(pwd / FilePaths::SRC_PATH, files);
			collect_fmt_files(pwd / FilePaths::INC_PATH, files);
			collect_fmt_files(pwd / "tests", files);
			collect_fmt_files(pwd / "test", files);
			if(!files.empty()) {
				std::cout << "==> fmt --check\n";
				subprocess::CommandLine cmd{ "clang-format", "--dry-run", "--Werror" };
				for(const auto& f: files) {
					cmd.emplace_back(f.string());
				}
				try {
					if(!subprocess_ok(subprocess::run(cmd))) {
						plus::diag::error("formatting check failed.\n");
						return InitializationError::COMMAND_FAILED;
					}
				} catch(const std::exception& ex) {
					plus::diag::error_stream()
						<< "failed to run clang-format: " << ex.what() << '\n';
					return InitializationError::COMMAND_FAILED;
				}
			}
		}

		// 2) tidy (only if compile_commands.json exists)
		{
			const auto compile_json = build_dir / "compile_commands.json";
			if(fs::exists(compile_json)) {
				std::cout << "==> tidy\n";
				std::vector<fs::path> files;
				collect_project_cpp_sources(pwd / FilePaths::SRC_PATH, files);
				collect_project_cpp_sources(pwd / "tests", files);
				collect_project_cpp_sources(pwd / "test", files);
				if(!files.empty()) {
					subprocess::CommandLine cmd;
					cmd.emplace_back("clang-tidy");
					cmd.emplace_back("-p");
					cmd.emplace_back(fs::absolute(build_dir).string());
					for(const auto& fpath: files) {
						cmd.emplace_back(fpath.string());
					}
					subprocess::RunOptions opts;
					opts.cwd = pwd.string();
					try {
						if(!subprocess_ok(subprocess::run(cmd, opts))) {
							plus::diag::error("clang-tidy reported diagnostics.\n");
							return InitializationError::COMMAND_FAILED;
						}
					} catch(const std::exception& ex) {
						plus::diag::error_stream()
							<< "failed to run clang-tidy: " << ex.what() << '\n';
						return InitializationError::COMMAND_FAILED;
					}
				}
			} else {
				std::cout << "==> tidy (skipped — no compile_commands.json)\n";
			}
		}

		// 3) build
		std::cout << "==> build\n";
		if(!run_cmake_build(pwd, conf, Cli::BuildType::dbg)) {
			plus::diag::error("build failed.\n");
			return InitializationError::COMMAND_FAILED;
		}

		// 4) test
		std::cout << "==> test\n";
		try {
			const auto proc = subprocess::run(subprocess::CommandLine{ "ctest",
				"--test-dir",
				build_dir.string(),
				"--output-on-failure",
				"-C",
				"Debug" });
			if(!subprocess_ok(proc)) {
				plus::diag::error("tests failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "failed to run ctest: " << ex.what() << '\n';
			return InitializationError::COMMAND_FAILED;
		}

		std::cout << "check: all gates passed.\n";
		return InitializationError::OK;
	}

	auto run_command_install(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		const auto build_dir = (pwd / conf.proj.buildDir).string();
		const auto t		 = cli.install.type.value_or(Cli::BuildType::dbg);

		if(!run_cmake_build(pwd, conf, t)) {
			plus::diag::error("build failed.\n");
			return InitializationError::COMMAND_FAILED;
		}

		subprocess::CommandLine cmd{ "cmake", "--install", build_dir };
		if(cli.install.prefix.has_value() && !cli.install.prefix->empty()) {
			cmd.emplace_back("--prefix");
			cmd.emplace_back(*cli.install.prefix);
		}
		cmd.emplace_back("--config");
		cmd.emplace_back(std::string(buildTypeConfig(t)));

		try {
			if(!subprocess_ok(subprocess::run(cmd))) {
				plus::diag::error("cmake --install failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "cmake --install failed: " << ex.what() << '\n';
			return InitializationError::COMMAND_FAILED;
		}
		std::cout << "Installed successfully.\n";
		return InitializationError::OK;
	}

	auto run_command_release(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		if(!std::filesystem::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		Config conf(pwd / FilePaths::PLUSTOML);
		if(!run_cmake_build(pwd, conf, Cli::BuildType::rel, cli.release.jobs)) {
			plus::diag::error("release build failed.\n");
			return InitializationError::COMMAND_FAILED;
		}

		if(conf.proj.kind == "bin" && cli.release.strip.value_or(false)) {
			const auto exe = find_built_executable(pwd, conf);
			if(exe.has_value()) {
				try {
					subprocess::run(subprocess::CommandLine{ "strip", exe->string() });
					std::cout << "Stripped `" << exe->filename().string() << "`.\n";
				} catch(const std::exception&) {
					std::cout << "Warning: `strip` failed or not found; binary not stripped.\n";
				}
			}
		}

		if(conf.proj.kind == "bin") {
			const auto exe = find_built_executable(pwd, conf);
			if(exe.has_value()) {
				namespace fs = std::filesystem;
				std::error_code ec;
				const auto sz = fs::file_size(*exe, ec);
				if(!ec) {
					std::cout << "Release binary: " << exe->filename().string() << " ("
							  << (sz / 1024) << " KiB)\n";
				}
			}
		}
		std::cout << "Release build complete.\n";
		return InitializationError::OK;
	}

	auto run_command_bench(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		namespace fs = std::filesystem;
		if(!fs::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		const auto build_dir = pwd / conf.proj.buildDir;
		const auto t		 = cli.bench.type.value_or(Cli::BuildType::rel);

		if(!run_cmake_build(pwd, conf, t)) {
			plus::diag::error("build failed.\n");
			return InitializationError::COMMAND_FAILED;
		}

		const std::string slug = utils::project_slug(conf.proj.name);
		const fs::path candidates[] = {
			build_dir / (slug + "_bench"),
			build_dir / (slug + "_benchmarks"),
			build_dir / "bench",
			build_dir / "benchmarks",
			build_dir / "Release" / (slug + "_bench"),
			build_dir / "Release" / (slug + "_benchmarks"),
		};

		std::optional<fs::path> bench_exe;
		for(const auto& p: candidates) {
			if(fs::exists(p) && fs::is_regular_file(p)) {
				bench_exe = p;
				break;
			}
		}

		if(!bench_exe.has_value()) {
			plus::diag::error("no benchmark executable found. Add a benchmark target "
							  "(e.g. `<project>_bench`) to your CMakeLists.txt.\n");
			return InitializationError::COMMAND_FAILED;
		}

		std::cout << "Running benchmarks: " << bench_exe->filename().string() << '\n';
		try {
			subprocess::RunOptions opts;
			opts.cwd = pwd.string();
			if(!subprocess_ok(subprocess::run(
				   subprocess::CommandLine{ bench_exe->string() }, opts))) {
				plus::diag::error("benchmarks failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "failed to run benchmarks: " << ex.what() << '\n';
			return InitializationError::COMMAND_FAILED;
		}
		return InitializationError::OK;
	}

	auto snapshot_mtimes(const std::filesystem::path& base, const std::set<std::string>& exts)
		-> std::unordered_map<std::string, std::filesystem::file_time_type> {
		namespace fs = std::filesystem;
		std::unordered_map<std::string, fs::file_time_type> result;
		if(!fs::exists(base)) {
			return result;
		}
		for(const auto& entry:
			fs::recursive_directory_iterator(base, fs::directory_options::skip_permission_denied)) {
			if(!entry.is_regular_file()) {
				continue;
			}
			std::string ext = entry.path().extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});
			if(exts.count(ext) != 0u) {
				result[entry.path().string()] = entry.last_write_time();
			}
		}
		return result;
	}

	auto run_command_watch(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		namespace fs = std::filesystem;
		if(!fs::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const Config conf(pwd / FilePaths::PLUSTOML);
		const auto t	   = cli.watch.type.value_or(Cli::BuildType::dbg);
		const int interval = cli.watch.interval.value_or(2);
		static const std::set<std::string> watch_exts{
			".cpp", ".cxx", ".cc", ".h", ".hpp", ".inl", ".cmake" };

		std::cout << "Watching for changes (poll every " << interval << "s)... Ctrl+C to stop.\n";
		auto prev_src = snapshot_mtimes(pwd / FilePaths::SRC_PATH, watch_exts);
		auto prev_inc = snapshot_mtimes(pwd / FilePaths::INC_PATH, watch_exts);
		auto prev_tst = snapshot_mtimes(pwd / "tests", watch_exts);

		for(;;) {
			std::this_thread::sleep_for(std::chrono::seconds(interval));
			auto cur_src = snapshot_mtimes(pwd / FilePaths::SRC_PATH, watch_exts);
			auto cur_inc = snapshot_mtimes(pwd / FilePaths::INC_PATH, watch_exts);
			auto cur_tst = snapshot_mtimes(pwd / "tests", watch_exts);

			bool changed = (cur_src != prev_src || cur_inc != prev_inc || cur_tst != prev_tst);
			if(!changed) {
				continue;
			}

			std::cout << "\n--- Change detected, rebuilding ---\n";
			if(run_cmake_build(pwd, conf, t, cli.watch.jobs)) {
				std::cout << "--- Build succeeded ---\n";
			} else {
				std::cout << "--- Build FAILED ---\n";
			}

			prev_src = std::move(cur_src);
			prev_inc = std::move(cur_inc);
			prev_tst = std::move(cur_tst);
		}
	}

	auto run_command_update(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		namespace fs = std::filesystem;
		if(!fs::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		Config conf(pwd / FilePaths::PLUSTOML);
		if(conf.conan.requires.empty()) {
			std::cout << "No Conan requirements to update.\n";
			return InitializationError::OK;
		}

		write_conanfile_txt(pwd, conf);

		const auto t = cli.update.type.value_or(Cli::BuildType::dbg);
		const std::string bt(buildTypeConfig(t));
		subprocess::RunOptions opts;
		opts.cwd = pwd.string();

		try {
			const auto proc = subprocess::run(
				subprocess::CommandLine{ "conan",
					"install",
					".",
					"-s",
					std::string("build_type=") + bt,
					"--build=missing",
					"--update",
					std::string("--output-folder=") + conf.conan.output_folder },
				opts);
			if(!subprocess_ok(proc)) {
				plus::diag::error("conan install --update failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} catch(const std::exception& ex) {
			plus::diag::error_stream() << "conan update failed: " << ex.what() << '\n';
			return InitializationError::COMMAND_FAILED;
		}

		std::cout << "Dependencies updated.\n";
		return InitializationError::OK;
	}

	auto run_command_version(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		namespace fs = std::filesystem;
		if(!fs::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		Config conf(pwd / FilePaths::PLUSTOML);
		const std::string& part = cli.version.part;

		unsigned major = 0, minor = 0, patch = 0;
		std::istringstream iss(conf.proj.version);
		char dot = 0;
		iss >> major >> dot >> minor >> dot >> patch;

		if(part == "major") {
			++major;
			minor = 0;
			patch = 0;
		} else if(part == "minor") {
			++minor;
			patch = 0;
		} else if(part == "patch") {
			++patch;
		} else {
			plus::diag::error("expected `major`, `minor`, or `patch`.\n");
			return InitializationError::INVALID_ARG;
		}

		conf.proj.version =
			std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
		if(!save_plus_toml(pwd, conf)) {
			return InitializationError::COMMAND_FAILED;
		}
		std::cout << "Version bumped to " << conf.proj.version << "\n";
		return InitializationError::OK;
	}

	auto run_command_config_set(const Cli& cli, const std::filesystem::path& pwd)
		-> InitializationError {
		namespace fs = std::filesystem;
		if(!fs::exists(pwd / FilePaths::PLUSTOML)) {
			plus::diag::error("`plus.toml` not found.\n");
			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const std::string& key = cli.config.key;
		const std::string& val = cli.config.value;

		Config conf(pwd / FilePaths::PLUSTOML);

		if(key == "name") {
			conf.proj.name = val;
		} else if(key == "version") {
			conf.proj.version = val;
		} else if(key == "cpp_std") {
			conf.proj.cpp_std = val;
		} else if(key == "kind") {
			if(val != "bin" && val != "lib") {
				plus::diag::error("kind must be `bin` or `lib`.\n");
				return InitializationError::INVALID_ARG;
			}
			conf.proj.kind = val;
		} else if(key == "buildDir") {
			conf.proj.buildDir = val;
		} else if(key == "repo") {
			conf.proj.repo = val;
		} else if(key == "license") {
			conf.proj.license = val;
		} else if(key == "author.name") {
			conf.author.name = val;
		} else if(key == "author.email") {
			conf.author.email = val;
		} else if(key == "conan.output_folder") {
			conf.conan.output_folder = val;
		} else {
			plus::diag::error_stream() << "unknown config key `" << key << "`.\n";
			return InitializationError::INVALID_ARG;
		}

		if(!save_plus_toml(pwd, conf)) {
			return InitializationError::COMMAND_FAILED;
		}
		std::cout << "Set `" << key << "` = `" << val << "` in plus.toml.\n";
		return InitializationError::OK;
	}

	auto run_command_completions(const Cli& cli) -> InitializationError {
		const std::string& sh = cli.completions.shell;
		if(sh == "bash") {
			std::cout << R"(# plus bash completions
_plus() {
    local cur="${COMP_WORDS[COMP_CWORD]}"
    local cmds="init new build setup run clean fmt tidy show test deps add remove check install release bench watch update version config completions help"
    if [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "${cmds}" -- "${cur}") )
    fi
}
complete -F _plus plus
)";
		} else if(sh == "zsh") {
			std::cout << R"(#compdef plus
_plus() {
    local -a commands=(
        'init:Initialize a project in the current directory'
        'new:Create a new project'
        'build:Build the project'
        'setup:Configure CMake'
        'run:Build and run'
        'clean:Remove build directory'
        'fmt:Format source files'
        'tidy:Run clang-tidy'
        'show:Show manifest info'
        'test:Run tests'
        'deps:Manage Conan dependencies'
        'add:Add a Conan requirement'
        'remove:Remove a Conan requirement'
        'check:Run fmt+tidy+build+test'
        'install:Install built artifacts'
        'release:Release build'
        'bench:Run benchmarks'
        'watch:Watch and rebuild'
        'update:Update dependencies'
        'version:Bump version'
        'config:Set config values'
        'completions:Generate shell completions'
    )
    _describe 'command' commands
}
compdef _plus plus
)";
		} else if(sh == "fish") {
			std::cout << R"(# plus fish completions
set -l cmds init new build setup run clean fmt tidy show test deps add remove check install release bench watch update version config completions help
complete -c plus -f
for cmd in $cmds
    complete -c plus -n "not __fish_seen_subcommand_from $cmds" -a "$cmd"
end
)";
		} else {
			plus::diag::error("supported shells: bash, zsh, fish.\n");
			return InitializationError::INVALID_ARG;
		}
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
			if(!run_cmake_configure(pwd, conf,
					std::optional<std::filesystem::path>{ tc },
					std::optional<Cli::BuildType>{ t }, cli.setup.generator)) {
				plus::diag::error("CMake configure failed.\n");
				return InitializationError::COMMAND_FAILED;
			}
		} else {
			if(!run_cmake_configure(pwd, conf, std::nullopt,
					std::optional<Cli::BuildType>{ t }, cli.setup.generator)) {
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
	if(cli.tidy.has_value()) {
		return run_command_tidy(cli, pwd);
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
	if(cli.remove.has_value()) {
		return run_command_remove(cli, pwd);
	}
	if(cli.check.has_value()) {
		return run_command_check(cli, pwd);
	}
	if(cli.install.has_value()) {
		return run_command_install(cli, pwd);
	}
	if(cli.release.has_value()) {
		return run_command_release(cli, pwd);
	}
	if(cli.bench.has_value()) {
		return run_command_bench(cli, pwd);
	}
	if(cli.watch.has_value()) {
		return run_command_watch(cli, pwd);
	}
	if(cli.update.has_value()) {
		return run_command_update(cli, pwd);
	}
	if(cli.version.has_value()) {
		return run_command_version(cli, pwd);
	}
	if(cli.config.has_value()) {
		return run_command_config_set(cli, pwd);
	}
	if(cli.completions.has_value()) {
		return run_command_completions(cli);
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
