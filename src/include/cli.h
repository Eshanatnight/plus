#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

// prints the help message for the app
void printHelp(bool exit = true);

// initializes the project
// called when the `init` command is called
void initProject(const std::filesystem::path& currentPath);

// this is depricated and just kept for backwards compatibility
[[maybe_unused]]void initProject(const std::filesystem::path& currentPath, const std::string& projectName);

void run(const std::vector<std::string>& args);