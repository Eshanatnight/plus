#pragma once

#include <filesystem>

auto _initializGitRepo(const std::filesystem::path& path, const bool makePath) -> void;
