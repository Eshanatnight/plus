#pragma once

#include "cli.h"

#include <filesystem>
#include <future>
#include <string_view>

enum class InstPath {
	MAINCPP,
	GITIGNORE,
	CMAKELISTS
};

auto initializeAndRun(const Cli& cli, std::filesystem::path& pwd, std::string& appName) -> bool;
auto InstPathToFilePath(InstPath file, const std::filesystem::path& basePath)
	-> std::filesystem::path;

// NOLINTNEXTLINE
auto _writeContent(
	const std::filesystem::path& basePath, InstPath instPath, std::string_view content) -> bool;

auto makeBuildDir(const std::filesystem::path& basePath) -> std::future<void>;
auto makeFiles(std::vector<std::future<void>>& futs,
	const std::filesystem::path& basePath,
	std::string_view appName,
	bool isLib) -> void;
