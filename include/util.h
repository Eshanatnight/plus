#pragma once
#include <string>
#include <optional>
#include <vector>

namespace plusutil
{
    // create camkelists.txt
    void createCmakeLists(const std::string& path, const std::string& project_name, bool isLib);

    // create directory
    void create_directory(const std::string& path);

    // create file
    void create_file(const std::string& path);

    // returns an optional project name
    std::optional<std::string> get_project_name(const std::vector<std::string>& args);
}

