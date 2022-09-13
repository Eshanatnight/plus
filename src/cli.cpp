#include "cli.h"
#include "util.h"

#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/color.h>

#include <ranges>


void Cli::printHelp()
{
    fmt::print(fg(fmt::color::lawn_green), "Usage:\n");
    fmt::print(fg(fmt::color::green), "plus ");
    fmt::print("[options] \n");
    fmt::print(fmt::emphasis::bold, "\nOptions:\n");
    fmt::print("init              \t\tInitializes a new project\n");
    fmt::print("new <folder_name> \t\tCreates a new project\n");
    fmt::print("-v, --version     \t\tPrints the version of the program\n");
    fmt::print("-h, --help        \t\tPrints this help\n");
}


void Cli::run(const std::vector<std::string>& args)
{
    // get the current path so we wont have to querry it every time
    currentPath = std::filesystem::current_path().string();
    // plus -h or plus --help
    if (std::ranges::find(args, "-h") != args.end() ||
        std::ranges::find(args, "--help") != args.end())
    {
        printHelp();
        return;
    }
    // plus -v or plus --version
    else if (std::ranges::find(args, "-v") != args.end() ||
        std::ranges::find(args, "--version") != args.end())
    {
        fmt::print("plus v2.0\n");
        printHelp();
        return;
    }
    // plus init
    else if (std::ranges::find(args, "init") != args.end())
    {
        projectName = std::filesystem::current_path().stem().string();
        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Initializing a new project\n");
        initProject(currentPath, projectName);
        return;
    }
    // plus new "Hello World"
    else if (std::ranges::find(args, "new") != args.end())
    {
        projectName = plusutil::get_project_name(args).value();
        std::string projectPath = currentPath + "/" + projectName;
        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Creating a new project\n");
        plusutil::create_directory(projectPath);
        initProject(projectPath, projectName);
        return;
    }
    else
    {
        printHelp();
        exit(1);
    }
}


void Cli::initProject(const std::string& repositoryPath, const std::string& projectName)
{
    repo.initGitRepository(repositoryPath);
    repo.initGitFiles(repositoryPath);

    // create the src folder and out directory
    plusutil::create_directory(repositoryPath + "/src");
    plusutil::create_directory(repositoryPath + "/out");
    // create the main.cpp file
    plusutil::create_file(repositoryPath);
    plusutil::createCmakeLists(repositoryPath, projectName);
}