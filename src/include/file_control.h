#pragma once

#include <filesystem>
#include <string_view>

enum class InstPath {
	MAINCPP,
	GITIGNORE,
	CMAKELISTS
};

auto InstPathToFilePath(InstPath file) -> std::filesystem::path;

auto writeContent(InstPath instPath, std::string_view content) -> bool;
