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
    fmt::print("[commands] \n");
    fmt::print(fmt::emphasis::bold, "\nCommands:\n");
    fmt::print("init <subcommand>              \t\tInitializes a new project\n");
    fmt::print("new <folder_name> <subcommand> \t\tCreates a new project\n");
    fmt::print("-v, --version                  \t\tPrints the version of the program\n");
    fmt::print("-h, --help                     \t\tPrints this help\n");
    fmt::print("\n\n");
    fmt::print(fmt::emphasis::bold, "Subcommands:\n");
    fmt::print("--lib                          \t\tInitializes a new library project\n");
    fmt::print("--bin                          \t\tInitializes a new binary project (Default)\n");
}


void Cli::run(const std::vector<std::string>& args)
{
    // get the current path so we wont have to querry it every time
    currentPath = std::filesystem::current_path().string();
    // plus -h or plus --help
    if (std::ranges::find(args, "-h") != args.end() ||
        std::ranges::find(args, "--help") != args.end())
    {
        if(args.size() == 1) {
            fmt::print("Invalid Command\n");
            return;
        }
        printHelp();
        return;
    }

    // plus -v or plus --version
    else if (std::ranges::find(args, "-v") != args.end() ||
        std::ranges::find(args, "--version") != args.end())
    {
        if(args.size() == 1) {
            fmt::print("Invalid Command\n");
            return;
        }
        fmt::print("plus v3.1\n");
        printHelp();
        return;
    }
    // plus init
    else if (std::ranges::find(args, "init") != args.end())
    {
        bool isLib = false;
        if(std::ranges::find(args, "--lib") != args.end()) {
            isLib = true;
        }
        this->projectName = std::filesystem::current_path().stem().string();
        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Initializing project\n");
        this->initProject(currentPath, projectName, isLib);
        return;
    }
    // plus new "Hello World"
    else if (std::ranges::find(args, "new") != args.end())
    {
        bool isLib = false;
        if(std::ranges::find(args, "--lib") != args.end()) {
            isLib = true;
        }
        this->projectName = plusutil::get_project_name(args).value();
        std::string projectPath = currentPath + "/" + projectName;
        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Creating new project\n");
        plusutil::create_directory(projectPath);
        this->initProject(projectPath, projectName, isLib);
        return;
    }
    else
    {
        printHelp();
        exit(1);
    }
}


void Cli::initProject(const std::string& repositoryPath, const std::string& projectName, bool isLib)
{
    this->repo.initGitRepository(repositoryPath);
    this->repo.initGitFiles(repositoryPath);

    // create the src folder and out directory
    plusutil::create_directory(repositoryPath + "/src");
    plusutil::create_directory(repositoryPath + "/build");

    plusutil::create_file(repositoryPath);
    plusutil::createCmakeLists(repositoryPath, projectName, isLib);
}