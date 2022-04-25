#pragma once
#include <string>
#include <vector>

namespace plusutil
{
    // create camkelists.txt
    void createCmakeLists(const std::string& path, const std::string& project_name);

    // create directory
    void create_directory(const std::string& path);

    // create file
    void create_file(const std::string& path);

    // returns an iterator to the name of the project if the project name is not found
    // returns an iterator to the end of the vector
    auto get_project_name(std::vector<std::string>& args);

}
