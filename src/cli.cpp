#include "include/cli.h"
#include "include/util.h"
#include "include/githandler.h"

#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/color.h>

#include <ranges>


void printHelp(bool exit)
{
    fmt::print(fg(fmt::color::lawn_green), "Usage:\n");
    fmt::print(fg(fmt::color::green), "plus ");
    fmt::print("[options] \n");

    if(exit)
    {
        return;
    }

    fmt::print(fmt::emphasis::bold | fg(fmt::color::yellow_green) | bg(fmt::color::white_smoke), "Options:\n");
    fmt::print("\n-v, --version     \t\tPrints the version of the program\n");
    fmt::print("-h, --help        \t\tPrints this help\n");
    fmt::print("init              \t\tInitializes a new project\n");
    fmt::print("new <folder_name> \t\tCreates a new project\n");
}


void initProject(const std::filesystem::path& currentPath)
{
    // initialize the git repo
    plusutil::initGitRepository(currentPath.string());

    // create the .gitignore and .gitattributes files
    plusutil::initGitFiles(currentPath.string());

    // create the src folder
    plusutil::create_directory(currentPath.string() + "/src");

    // create the main.cpp file
    plusutil::create_file(currentPath.string() + "/src");

    // create the out directory
    plusutil::create_directory(currentPath.string() + "/out");

    plusutil::createCmakeLists(currentPath.string(), currentPath.stem().string());
}


void initProject(const std::filesystem::path& currentPath, const std::string& projectName)
{
    // initialize the git repo
    plusutil::initGitRepository(currentPath.string());

    // create the .gitignore and .gitattributes files
    plusutil::initGitFiles(currentPath.string());

    // create the src folder
    plusutil::create_directory(currentPath.string() + "/src");

    // create the main.cpp file
    plusutil::create_file(currentPath.string() + "/src");

    // create the out directory
    plusutil::create_directory(currentPath.string() + "/out");

    plusutil::createCmakeLists(currentPath.string(), projectName);
}


void run(const std::vector<std::string>& args)
{
    // get the current path so we wont have to querry it every time
    std::filesystem::path currentPath = std::filesystem::current_path();

    // plus -h or plus --help
    if(std::ranges::find(args.begin(), args.end(), "-h") != args.end()||
        std::ranges::find(args.begin(), args.end(), "--help") != args.end())
    {
        printHelp(false);
        return;
    }

    // plus -v or plus --version
    else if(std::ranges::find(args.begin(), args.end(), "-v") != args.end()||
        std::ranges::find(args.begin(), args.end(), "--version") != args.end())
    {
        fmt::print("plus v0.1\n");
        printHelp();
        return;
    }

    // plus init
    else if(std::ranges::find(args.begin(), args.end(), "init") != args.end())
    {
        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Initializing a new project\n");
        initProject(currentPath);
        return;
    }

    // plus new "Hello World"
    else if(std::ranges::find(args.begin(), args.end(), "new") != args.end())
    {
        std::string projectName = plusutil::get_project_name(args).value();
        std::string projectPath = currentPath.string() + "/" + projectName;

        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Creating a new project\n");

        plusutil::create_directory(projectPath);
        initProject(projectPath);
        return;
    }

    else
    {
        printHelp();
        exit(1);
    }
}