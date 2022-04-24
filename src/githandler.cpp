#include "include/githandler.h"

#include "include/lg2prelude.h"

#include <fmt/color.h>
#include <fmt/ostream.h>
#include <fmt/os.h>

[[maybe_unused]] void plusutil::initGitFiles()
{
    fmt::print(fg(fmt::color::lawn_green), "Initializing a gitignore file\n");
    auto gitignore = fmt::output_file(".gitignore", fmt::file::WRONLY | fmt::file::CREATE);

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
}

[[maybe_unused]] void plusutil::initGitRepository(const std::string& repositoryPath)
{
    fmt::print(fg(fmt::color::lawn_green), "Initializing a git repository\n");
    fmt::print(fg(fmt::color::lawn_green), "Repository path: {}\n", repositoryPath);

    lg2_prelude::init_repo(repositoryPath);
}
