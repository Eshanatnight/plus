#pragma once
#include <string>

#include "repo.h" 
#include "initializer.h"

namespace plusutil
{
    class PlusGit
    {
        std::unique_ptr<git::Repository> repo;
        std::string path;

    public: 
        PlusGit();

        // this function will initialize the git repository
        // initializes a git repository in the given path
        void initGitRepository(const std::string& path);
        
        // this function will initialize the files for git
        // creates the proper files for git
        // .gitignore, .gitattributes
        void initGitFiles(const std::string& path);
    };






}

