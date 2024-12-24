#pragma once

#include "cli.h"

#include <filesystem>

auto _initializGitRepo(const std::filesystem::path& path, const bool makePath) -> void;

auto initializGitRepo(const Cli& cli, std::filesystem::path& pwd) -> bool;
