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

enum class InitializationError {
	INVALID_ARG,
	PLUS_TOML_NOT_FOUND,
	CMAKELISTS_NOT_FOUND,
	BUILD_DIR_DOES_NOT_EXIST,
	UNKNOWN,
	OK,
};

auto initializeAndRun(const Cli& cli, std::filesystem::path& pwd, std::string& appName)
	-> InitializationError;
auto InstPathToFilePath(InstPath file, const std::filesystem::path& basePath)
	-> std::filesystem::path;

auto flushNewConfig(const Cli& cli,
	const std::filesystem::path& pwd,
	const std::string& appName,
	std::string_view packageType) -> bool;

// NOLINTNEXTLINE
auto _writeContent(
	const std::filesystem::path& basePath, InstPath instPath, std::string_view content) -> bool;

auto makeBuildDir(const std::filesystem::path& basePath) -> std::future<void>;
auto makeFiles(std::vector<std::future<void>>& futs,
	const std::filesystem::path& basePath,
	std::string_view appName,
	bool isLib) -> void;
