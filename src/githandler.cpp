#include "include/githandler.h"

#include <fmt/color.h>
#include <fmt/ostream.h>
#include <fmt/os.h>
#include <git2.h>
#include <string>
#include <cstdio>
#include <io.h>
#include <Windows.h>


[[maybe_unused]] void plusutil::initGitRepository(const std::string& repositoryPath)
{
    fmt::print(fg(fmt::color::lawn_green), "Initializing a git repository\n");
    fmt::print(fg(fmt::color::lawn_green), "Repository path: {}\n", repositoryPath);

    git_repository* repo;

    git_libgit2_init();

    plusutil::check_lg2(
            git_repository_init(&repo, repositoryPath.c_str(), 0),
            "Unable to init repository",
            NULL);

    git_repository_free(repo);
}


[[maybe_unused]] void plusutil::initGitFiles(const std::string& path)
{
    fmt::print(fg(fmt::color::lawn_green), "Initializing a gitignore file\n");
    auto gitignore = fmt::output_file(path + "/.gitignore", fmt::file::WRONLY | fmt::file::CREATE);

    gitignore.print("# CMAKE Stuff\n\n");
    gitignore.print("## CMake Build Directories\n");
    gitignore.print("/cmake-build-debug\n");
    gitignore.print("/cmake-build-release\n");
    gitignore.print("/out\n");
    gitignore.print("/build\n");
    gitignore.print("/bin\n\n\n\n");
    gitignore.flush();

    gitignore.print("## IDE AND TEXT EDITOR Stuff\n");
    gitignore.print("/.vs\n");
    gitignore.print("/.vscode\n");
    gitignore.print("/.idea\n\n\n\n");
    gitignore.flush();

    gitignore.print("## Mics\n");
    gitignore.print("*.bin\n");
    gitignore.print("*.exe\n");
    gitignore.print("*.log\n");
    gitignore.print("*.obj\n");
    gitignore.print("*.o\n");
    gitignore.print("*.a\n");
    gitignore.print("*.lib\n");
    gitignore.print("*.dll\n");
    gitignore.print("*.so\n");
    gitignore.flush();

    gitignore.close();

    auto gitattributes = fmt::output_file(path + "/.gitattributes", fmt::file::WRONLY | fmt::file::CREATE);
    gitattributes.print("* text=auto\n");
    gitattributes.flush();
    gitattributes.close();
}


void plusutil::check_lg2(int error, const char *message, const char *extra)
{
    const git_error *lg2err;
    const char *lg2msg = "", *lg2spacer = "";

    if (!error)
        return;

    if ((lg2err = git_error_last()) != NULL && lg2err->message != NULL) {
        lg2msg = lg2err->message;
        lg2spacer = " - ";
    }

    if (extra)
    {
        fmt::print(stderr, "`{}` `{}` [{}]{}{}\n", message, extra, error, lg2spacer, lg2msg);
    }

    else
    {
        fmt::print(stderr, "{}[{}]{}{}\n",message, error, lg2spacer, lg2msg);
    }

    exit(1);
}