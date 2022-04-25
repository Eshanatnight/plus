#include "util.h"
#include <filesystem>
#include <fmt/ostream.h>
#include <fmt/os.h>
#include <fmt/color.h>
#include <fmt/format.h>

void plusutil::createCmakeLists(const std::string &path, const std::string& project_name)
{
    fmt::print(fg(fmt::color::lawn_green), "Creating CMakeLists.txt\n");

    auto cmakelists = fmt::output_file(path + "/CMakeLists.txt");
    cmakelists.print("cmake_minimum_required(VERSION 3.10)\n");
    cmakelists.print("project({})\n", project_name);
    cmakelists.print("\n");
    cmakelists.print("add_executable({} {})\n", project_name, "main.cpp");
    cmakelists.flush();
    cmakelists.close();
}

void plusutil::create_directory(const std::string &path)
{
    fmt::print(fg(fmt::color::lawn_green), "Creating directory: {}\n", path);
    std::filesystem::create_directory(path);
}

void plusutil::create_file(const std::string &path)
{
    char brace_open = '{';
    char brace_close = '}';

    fmt::print(fg(fmt::color::lawn_green), "Creating file...\n");

    auto main_cpp = fmt::output_file("./src/main.cpp");
    main_cpp.print("#include <iostream>\n");
    main_cpp.print("\n");
    main_cpp.print("int main()\n");
    main_cpp.print("{}\n", brace_open);
    main_cpp.print("    std::cout << \"Hello World!\" << std::endl;\n");
    main_cpp.print("\n    return 0;\n");
    main_cpp.print("{}\n", brace_close);
    main_cpp.flush();
    main_cpp.close();
}

auto plusutil::get_project_name(std::vector<std::string>& args)
{
    auto it = std::find(args.begin(), args.end(), "new");
    if (it != args.end() && it + 1 != args.end())
    {
        return ++it;
    }

    return args.end();
}