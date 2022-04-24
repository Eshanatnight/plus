#pragma once
#include <string>

namespace plusutil
{
    // this function will initialize the files for git
    // .gitignore, .gitattributes
    [[maybe_unused]] void initGitFiles();

    // this function will initialize the git repository
    [[maybe_unused]] void initGitRepository(const std::string& path);
}

