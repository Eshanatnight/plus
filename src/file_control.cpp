#include "file_control.h"

#include "data.h"

#include <filesystem>
#include <fstream>
#include <iostream>

auto InstPathToFilePath(InstPath file) -> std::filesystem::path {
	auto pwd = std::filesystem::current_path();

	switch(file) {
	case InstPath::MAINCPP :
		return pwd / FilePaths::SRC_PATH / FilePaths::MAIN_CPP;
	case InstPath::GITIGNORE :
		return pwd / FilePaths::GITIGNORE;
	case InstPath::CMAKELISTS :
		return pwd / FilePaths::CMAKELISTS;
	}
}

auto writeContent(InstPath instPath, std::string_view content) -> bool {
	namespace fs	= std::filesystem;
	const auto path = InstPathToFilePath(instPath);
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
