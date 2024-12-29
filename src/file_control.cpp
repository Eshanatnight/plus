#include "file_control.h"

#include "config.h"
#include "control.h"
#include "data.h"
#include "git.h"
#include "subprocess/basic_types.hpp"
#include "subprocess/ProcessBuilder.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <subprocess.hpp>
#include <toml++/toml.hpp>
#include <vector>

auto initializeAndRun(const Cli& cli, std::filesystem::path& pwd, std::string& appName) -> bool {

	std::string_view packageType = "bin";
	if(cli.init.has_value()) {
		_initializGitRepo(pwd, false);
		appName = pwd.filename();

		if(cli.init.kind.has_value()) {
			packageType = TypeToString(cli.new_.kind.value());
		}

	} else if(cli.new_.has_value()) {
		appName = cli.new_.projectName;
		pwd		= pwd / cli.new_.projectName;
		_initializGitRepo(pwd, true);
		if(cli.new_.kind.has_value()) {
			packageType = TypeToString(cli.new_.kind.value());
		}
	} else if(cli.build.has_value()) {

		// check if plus.toml exists or not
		const auto exists = std::filesystem::exists(pwd / FilePaths::PLUSTOML);

		if(!exists) {
			std::cerr << "plus.toml does not exists.\nAre you in the correct directory?\n";

			return false;
		}

		auto conf = Config(pwd / FilePaths::PLUSTOML);

		auto process = subprocess::run({ "cmake", "--build", conf.proj.buildDir });
		return true;

	} else {
		return false;
	}

	Config conf = Config(appName, packageType, std::string(FilePaths::BUILD_PATH));

	const auto tbl = conf.toTomlTable();

	std::ofstream tomlFile(pwd / "plus.toml");

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
