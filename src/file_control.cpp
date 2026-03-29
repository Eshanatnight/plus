#include "file_control.h"

#include "config.h"
#include "control.h"
#include "data.h"
#include "git.h"
#include "log.h"
#include "utils.h"
#include "subprocess/basic_types.hpp"
#include "subprocess/ProcessBuilder.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <numeric>
#include <ostream>
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
	out += "```bash\nplus setup\nplus build\n```\n\n";
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

auto build_lib_cpp(const Config& conf, const std::string& slug, const std::string& ns) -> std::string {
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

auto materialize_cmake_bin(const Config& conf, const std::string& cmake_project) -> std::string {
	std::string cm = FileContents::cmakeBin;
	replace_all(cm, Ident::plusMyAPP, cmake_project);
	replace_all(cm, Ident::plusCppStd, conf.proj.cpp_std);
	return cm;
}

auto materialize_cmake_lib(const Config& conf, const std::string& cmake_project, const std::string& slug)
	-> std::string {
	std::string cm = FileContents::cmakeLib;
	replace_all(cm, Ident::plusMyAPP, cmake_project);
	replace_all(cm, Ident::plusCppStd, conf.proj.cpp_std);
	replace_all(cm, Ident::plusSlug, slug);
	return cm;
}

}  // namespace

auto initializeAndRun(const Cli& cli, std::filesystem::path& pwd, std::string& appName)
	-> InitializationError {

	std::string_view packageType = "bin";
	if(cli.init.has_value()) {
		const auto destToml = pwd / FilePaths::PLUSTOML;
		if(std::filesystem::exists(destToml)) {
			std::cerr << "Error: `plus.toml` already exists in this directory.\n";
			return InitializationError::INVALID_ARG;
		}

		_initializGitRepo(pwd, false);
		appName = pwd.filename().string();
		if(appName.empty() || appName == "." || appName == "..") {
			appName = pwd.lexically_normal().filename().string();
		}

		if(cli.init.kind.has_value()) {
			packageType = TypeToString(cli.init.kind.value());
		}

		flushNewConfig(cli, pwd, appName, packageType);

	} else if(cli.new_.has_value()) {
		appName = cli.new_.projectName;
		const auto projectDir = pwd / cli.new_.projectName;
		if(std::filesystem::exists(projectDir)) {
			std::cerr << "Error: destination `" << projectDir.string()
					  << "` already exists.\n";
			return InitializationError::INVALID_ARG;
		}

		pwd = projectDir;
		_initializGitRepo(pwd, true);
		if(cli.new_.kind.has_value()) {
			packageType = TypeToString(cli.new_.kind.value());
		}

		flushNewConfig(cli, pwd, appName, packageType);

	} else if(cli.build.has_value()) {

		// check if plus.toml exists or not
		const auto exists = std::filesystem::exists(pwd / FilePaths::PLUSTOML);

		if(!exists) {
			std::cerr << "Error: `plus.toml` does not exists.\nAre you in the correct directory?\n";

			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		auto conf = Config(pwd / FilePaths::PLUSTOML);

		auto process = subprocess::run({ "cmake", "--build", conf.proj.buildDir });
		(void)process;
		return InitializationError::OK;

	} else if(cli.setup.has_value()) {
		// do stuff
		// read the config file and generate a config

		// check for plus.toml
		const auto plusTomlPath = pwd / FilePaths::PLUSTOML;
		auto exists				= std::filesystem::exists(plusTomlPath);

		if(!exists) {
			std::cerr
				<< "Error: `plus.toml` does not exists.\nAre you in the correct directory?\n ";

			return InitializationError::PLUS_TOML_NOT_FOUND;
		}

		const auto cmakeListPath = pwd / FilePaths::CMAKELISTS;
		exists					 = std::filesystem::exists(cmakeListPath);

		if(!exists) {

			std::cerr
				<< "Error: `CMakeLists.txt` does not exists.\nAre you in the correct directory?\n ";

			return InitializationError::CMAKELISTS_NOT_FOUND;
		}

		Config conf(plusTomlPath);
		const auto buildDir = pwd / conf.proj.buildDir;
		exists				= std::filesystem::exists(buildDir);

		if(!exists) {

			std::cerr << "Error: build directory does not exists.\n";

			return InitializationError::BUILD_DIR_DOES_NOT_EXIST;
		}

		std::string defs;

		std::for_each(conf.proj.cmakeDefines.begin(),
			conf.proj.cmakeDefines.end(),
			[&defs](const auto& elem) {
				defs += "-D";
				defs += elem;
				defs += ' ';
			});

		LOG_DEBUG_MSG(defs);

		// TODO: make the cmake src dir configurable
		subprocess::CompletedProcess process;
		if(!defs.empty()) {

			process = subprocess::run(
				{ "cmake", "-B", buildDir.c_str(), "-S", pwd.c_str(), defs.c_str() });
		} else {

			process = subprocess::run({ "cmake", "-B", buildDir.c_str(), "-S", pwd.c_str() });
		}

		(void)process;
		//
		return InitializationError::OK;

	} else {
		return InitializationError::UNKNOWN;
	}

	return InitializationError::OK;
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
	}
}

auto _writeContent(
	const std::filesystem::path& basePath, InstPath instPath, std::string_view content) -> bool {
	namespace fs	= std::filesystem;
	const auto path = InstPathToFilePath(instPath, basePath);
	fs::create_directories(path.parent_path());
	std::ofstream ofs(path);

	if(!ofs.is_open()) {
		std::cerr << "Failed to open file: " << path << '\n';
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

	const std::string slug			 = utils::project_slug(conf.proj.name);
	const std::string ns			 = utils::cpp_namespace_ident(conf.proj.name);
	const std::string cmake_project	 = slug;
	const std::string readme_content = build_readme(conf);
	const std::string license_content = build_license_mit(conf);

	if(!isLib) {
		futs.push_back(std::async(std::launch::async,
			[&basePath]() { _writeContent(basePath, InstPath::MAINCPP, FileContents::mainCPP); }));
	} else {
		const std::string header_content = build_lib_header(ns);
		const std::string cpp_content	  = build_lib_cpp(conf, slug, ns);

		futs.push_back(std::async(std::launch::async, [basePath, slug, header_content]() {
			namespace fs = std::filesystem;
			const auto path =
				basePath / FilePaths::INC_PATH / slug / (slug + std::string(".h"));
			fs::create_directories(path.parent_path());
			std::ofstream ofs(path);
			if(!ofs.is_open()) {
				std::cerr << "Failed to open file: " << path << '\n';
				return;
			}
			ofs << header_content;
		}));

		futs.push_back(std::async(std::launch::async, [basePath, slug, cpp_content]() {
			namespace fs = std::filesystem;
			const auto path = basePath / FilePaths::SRC_PATH / (slug + std::string(".cpp"));
			fs::create_directories(path.parent_path());
			std::ofstream ofs(path);
			if(!ofs.is_open()) {
				std::cerr << "Failed to open file: " << path << '\n';
				return;
			}
			ofs << cpp_content;
		}));
	}

	futs.push_back(std::async(std::launch::async,
		[&basePath]() { _writeContent(basePath, InstPath::GITIGNORE, FileContents::gitIgnore); }));
	futs.push_back(std::async(std::launch::async, [&basePath]() {
		_writeContent(basePath, InstPath::GITATTRIBUTES, FileContents::gitAttributes);
	}));
	futs.push_back(std::async(std::launch::async, [&basePath, readme_content]() {
		_writeContent(basePath, InstPath::README, readme_content);
	}));
	futs.push_back(std::async(std::launch::async, [&basePath, license_content]() {
		_writeContent(basePath, InstPath::LICENSE, license_content);
	}));

	futs.push_back(std::async(std::launch::async, [basePath, conf, isLib, cmake_project, slug]() {
		std::string result =
			isLib ? materialize_cmake_lib(conf, cmake_project, slug) : materialize_cmake_bin(conf, cmake_project);
		_writeContent(basePath, InstPath::CMAKELISTS, result);
	}));
}
