#include "file_control.h"

#include "data.h"
#include "git.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <toml++/impl/array.hpp>
#include <toml++/impl/json_formatter.hpp>
#include <toml++/impl/toml_formatter.hpp>
#include <vector>

auto initialize(const Cli& cli, std::filesystem::path& pwd, std::string& appName) -> bool {

	std::string_view packageType = "bin";
	bool ret					 = false;
	if(cli.init.has_value()) {
		_initializGitRepo(pwd, false);
		appName = pwd.filename();
		ret		= true;
	} else if(cli.new_.has_value()) {
		appName = cli.new_.projectName.c_str();
		pwd		= pwd / cli.new_.projectName.c_str();
		_initializGitRepo(pwd, true);
		if(cli.new_.packageType.has_value()) {
			packageType = TypeToString(cli.new_.packageType.value());
		}

		ret = true;
	}

	if(ret) {
		// save the config toml
		auto tbl = toml::table{
			{ "project",
				toml::table{ { "name", appName },
					{ "kind", packageType },
					{ "buildDir", FilePaths::BUILD_PATH },
					{ "author", toml::table{ { "name", "" }, { "repo", "" } } } } }
		};

		std::ofstream tomlFile(pwd / "plus.toml");

		tomlFile << toml::toml_formatter(tbl);
	}

	return ret;
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
		// TODO: figure out a better way
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
