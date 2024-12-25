#pragma once

#include <filesystem>
#include <future>
#include <string_view>

enum class InstPath {
	MAINCPP,
	GITIGNORE,
	CMAKELISTS
};

auto InstPathToFilePath(InstPath file, std::filesystem::path& basePath) -> std::filesystem::path;

auto _writeContent(std::filesystem::path& basePath, InstPath instPath, std::string_view content)
	-> bool;

auto makeBuildDir(const std::filesystem::path& basePath) -> std::future<void>;
auto makeFiles(
	std::vector<std::future<void>>& futs, std::filesystem::path& basePath, std::string_view appName)
	-> void;
