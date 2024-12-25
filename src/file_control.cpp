#include "file_control.h"

#include "data.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <string_view>
#include <vector>

auto InstPathToFilePath(InstPath file, std::filesystem::path& basePath) -> std::filesystem::path {

	switch(file) {
	case InstPath::MAINCPP :
		return basePath / FilePaths::SRC_PATH / FilePaths::MAIN_CPP;
	case InstPath::GITIGNORE :
		return basePath / FilePaths::GITIGNORE;
	case InstPath::CMAKELISTS :
		return basePath / FilePaths::CMAKELISTS;
	}
}

auto _writeContent(std::filesystem::path& basePath, InstPath instPath, std::string_view content)
	-> bool {
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

auto makeFiles(
	std::vector<std::future<void>>& futs, std::filesystem::path& basePath, std::string_view appName)
	-> void {

	futs.push_back(std::async(std::launch::async,
		[&basePath]() { _writeContent(basePath, InstPath::MAINCPP, FileContents::mainCPP); }));
	futs.push_back(std::async(std::launch::async,
		[&basePath]() { _writeContent(basePath, InstPath::GITIGNORE, FileContents::gitIgnore); }));
	futs.push_back(std::async(std::launch::async, [&basePath, appName]() {
		// TODO: figure out a better way
		const std::size_t pos = FileContents::cmakeLists.find(Ident::plusMyAPP);
		auto result			  = FileContents::cmakeLists.substr(0, pos);
		result += appName;
		result += FileContents::cmakeLists.substr(pos + std::string(Ident::plusMyAPP).length());

		_writeContent(basePath, InstPath::CMAKELISTS, result);
	}));
}
