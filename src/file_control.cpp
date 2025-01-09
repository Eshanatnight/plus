#include "file_control.h"

#include "config.h"
#include "control.h"
#include "data.h"
#include "git.h"
#include "log.h"
#include "subprocess/basic_types.hpp"
#include "subprocess/ProcessBuilder.hpp"

#include <algorithm>
#include <cstddef>
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

auto initializeAndRun(const Cli& cli, std::filesystem::path& pwd, std::string& appName)
	-> InitializationError {

	std::string_view packageType = "bin";
	if(cli.init.has_value()) {
		_initializGitRepo(pwd, false);
		appName = pwd.filename();

		if(cli.init.kind.has_value()) {
			packageType = TypeToString(cli.new_.kind.value());
		}

		if(cli.init.kind.has_value()) {
			packageType = TypeToString(cli.new_.kind.value());
		}

		flushNewConfig(cli, pwd, appName, packageType);

	} else if(cli.new_.has_value()) {
		appName = cli.new_.projectName;
		pwd		= pwd / cli.new_.projectName;
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

	// TODO: make it an array
	std::vector<std::future<void>> futures;
	futures.reserve(5);
	futures.emplace_back(makeBuildDir(pwd));
	makeFiles(futures, pwd, appName, isLib(cli));
	resolve(futures);

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
	std::string_view appName,
	bool isLib) -> void {

	futs.push_back(std::async(std::launch::async,
		[&basePath]() { _writeContent(basePath, InstPath::MAINCPP, FileContents::mainCPP); }));
	futs.push_back(std::async(std::launch::async,
		[&basePath]() { _writeContent(basePath, InstPath::GITIGNORE, FileContents::gitIgnore); }));
	futs.push_back(std::async(std::launch::async, [&basePath, appName, isLib]() {
		// TODO(kellsatnite): figure out a better way
		const std::size_t pos = FileContents::cmakeLists.find(Ident::plusMyAPP);
		auto result			  = FileContents::cmakeLists.substr(0, pos);
		result += appName;
		result += FileContents::cmakeLists.substr(pos + std::string(Ident::plusMyAPP).length());
		if(isLib) {
			const std::size_t pos = result.find(Ident::addExecuteable);
			std::string temp	  = result.substr(0, pos);
			temp += Ident::addLibrary;
			temp += result.substr(pos + Ident::addExecuteable.length());

			result = temp;
		}

		_writeContent(basePath, InstPath::CMAKELISTS, result);
	}));
}
