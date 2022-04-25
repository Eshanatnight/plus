#pragma once
#include <string>

namespace plusutil
{

    // this function will initialize the files for git
    // creates the proper files for git
    // .gitignore, .gitattributes
    [[maybe_unused]] void initGitFiles(const std::string& path);

    // this function will initialize the git repository
    // initializes a git repository in the given path
    [[maybe_unused]] void initGitRepository(const std::string& path);

    // libgit2 utility function
    [[maybe_unused]] void check_lg2(int error, const char *message, const char *extra);
}

