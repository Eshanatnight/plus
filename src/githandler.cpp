#include "githandler.h"
#include <fmt/color.h>
#include <fmt/ostream.h>
#include <fmt/os.h>
#include <fmt/core.h>
#include <git2.h>
#include <string>
#include <cstdio>
#include <io.h>
#include <Windows.h>

plusutil::PlusGit::PlusGit()
{}

void plusutil::PlusGit::initGitRepository(const std::string& repositoryPath)
{
    auto_git_initializer;
    auto repo = git::Repository(repositoryPath, git::Repository::init);
}


void plusutil::PlusGit::initGitFiles(const std::string& path)
{
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
