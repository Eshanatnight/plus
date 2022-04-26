#include "githandler.h"
#include "util.h"

#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/color.h>

#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>


void printHelp(bool exit = true)
{
    fmt::print(fg(fmt::color::green), "plus ");
    fmt::print("[options] \n");

    if(exit)
    {
        return;
    }

    fmt::print(fmt::emphasis::bold | fg(fmt::color::yellow_green), "Options:\n");
    fmt::print("-v, --version     \t\tPrints the version of the program\n");
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
    plusutil::create_file(currentPath.string() + "/src/main.cpp");

    // create the out directory
    plusutil::create_directory(currentPath.string() + "/out");

    plusutil::createCmakeLists(currentPath.string(), currentPath.stem().string());
    //plusutil::createCmakeLists(std::filesystem::current_path().string(), std::filesystem::current_path().stem().string());
}

[[maybe_unused]] void initProject(const std::filesystem::path& currentPath, const std::string& projectName)
{
    // initialize the git repo
    plusutil::initGitRepository(currentPath.string());

    // create the .gitignore and .gitattributes files
    plusutil::initGitFiles(currentPath.string());

    // create the src folder
    plusutil::create_directory(currentPath.string() + "/src");

    // create the main.cpp file
    plusutil::create_file(currentPath.string() + "/src/main.cpp");

    // create the out directory
    plusutil::create_directory(currentPath.string() + "/out");

    plusutil::createCmakeLists(currentPath.string(), projectName);
    //plusutil::createCmakeLists(std::filesystem::current_path().string(), projectName);
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printHelp();
        return 1;
    }

    std::vector<std::string> args(argv, argv + argc);
    std::filesystem::path currentPath = std::filesystem::current_path();

    // plus -h or plus --help
    if(std::find(args.begin(), args.end(), "-h") != args.end()||
        std::find(args.begin(), args.end(), "--help") != args.end())
    {
        printHelp(false);
        return 0;
    }

    // plus -v or plus --version
    else if(std::find(args.begin(), args.end(), "-v") != args.end()||
        std::find(args.begin(), args.end(), "--version") != args.end())
    {
        fmt::print("plus v0.1\n");
        printHelp();
        return 0;
    }

    // plus init
    else if(std::find(args.begin(), args.end(), "init") != args.end())
    {
        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Initializing a new project\n");
        initProject(currentPath);
        return 0;
    }

    // plus new "Hello World"
    else if(plusutil::get_project_name(args) != args.end())
    {
        std::string projectName = plusutil::get_project_name(args);
        std::string projectPath = currentPath.string() + "/" + projectName;

        fmt::print(fmt::emphasis::bold | fg(fmt::color::azure), "Creating a new project\n");

        plusutil::create_directory(projectPath);
        initProject(projectPath);
        return 0;
    }

    else
    {
        printHelp();
        return 1;
    }



    /*
    init git repo
    std::string git_dir = ".\\test\\";

    plusutil::initGitRepository(git_dir);
    */

    // plusutil::createCmakeLists(".\\test\\", "test");

    // create the src dir
    // plusutil::create_directory(".\\test\\src");

    // create the main.cpp file
    // plusutil::create_file(".\\test\\src\\main.cpp");

    // create output dir
    // plusutil::create_directory(".\\test\\out");

    return 0;
}
