#pragma once

#include <filesystem>

auto initializGitRepo(const std::filesystem::path& path, const bool makePath) -> void;
