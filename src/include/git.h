#pragma once

#include "cli.h"

#include <filesystem>
#include <future>
#include <optional>

auto _initializGitRepo(const std::filesystem::path& path, const bool makePath) -> void;

[[nodiscard]]
auto initializGitRepo(const Cli& cli, std::filesystem::path& pwd)
	-> std::optional<std::future<void>>;
