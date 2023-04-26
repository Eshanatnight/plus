#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

#include "githandler.h"

class Cli
{
public:
    static void printHelp();

    void run(const std::vector<std::string>& args);

private:
    void initProject(const std::string& repositoryPath, const std::string& projectName, bool isLib);

private:
    plusutil::PlusGit repo;
    std::string currentPath;
    std::string projectName;
};