#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <fmt/color.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int main()
{
    fmt::print(fg(fmt::color::lawn_green), "Initializing a gitignore file\n");
    auto file = fmt::output_file("..\\.gitignore" , fmt::file::WRONLY | fmt::file::CREATE);
    file.print("# CMAKE Stuff\n\n");
    file.print("## CMake Build Directories\n");
    file.print("/cmake-build-debug\n");
    file.print("/cmake-build-release\n");
    file.print("/out\n");
    file.print("/build\n");
    file.print("/bin\n\n\n\n");
    file.flush();

    file.print("## IDE AND TEXT EDITOR Stuff\n");
    file.print("/.vs\n");
    file.print("/.vscode\n");
    file.print("/.idea\n\n\n\n");
    file.flush();

    file.print("## Mics\n");
    file.print("*.bin\n");
    file.print("*.exe\n");
    file.print("*.log\n");
    file.print("*.obj\n");
    file.print("*.o\n");
    file.print("*.a\n");
    file.print("*.lib\n");
    file.print("*.dll\n");
    file.print("*.so\n");
    file.flush();

    file.close();


    return EXIT_SUCCESS;
}
