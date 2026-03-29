#pragma once

#include <filesystem>

auto initialize_git_repository(const std::filesystem::path& path, bool make_path) -> void;
