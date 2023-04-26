#include "util.h"
#include <ranges>
#include <filesystem>
#include <fmt/ostream.h>
#include <fmt/os.h>
#include <fmt/color.h>
#include <fmt/format.h>

void plusutil::createCmakeLists(const std::string &path, const std::string& project_name, bool isLib)
{
    auto cmakelists = fmt::output_file(path + "/CMakeLists.txt");
    cmakelists.print("cmake_minimum_required(VERSION 3.10)\n");
    cmakelists.print("project({})\n", project_name);
    cmakelists.print("\n");
    if(isLib){
        cmakelists.print("add_library({} {})\n", project_name, "src/main.cpp");
    } else{
        cmakelists.print("add_executable({} {})\n", project_name, "src/main.cpp");
    }
    cmakelists.flush();
    cmakelists.close();
}

void plusutil::create_directory(const std::string &path)
{
    std::filesystem::create_directory(path);
}

void plusutil::create_file(const std::string &path)
{
    char brace_open = '{';
    char brace_close = '}';

    auto main_cpp = fmt::output_file(path + "/src/main.cpp");
    main_cpp.print("#include <iostream>\n\n");
    main_cpp.print("int main()\n");
    main_cpp.print("{}\n", brace_open);
    main_cpp.print("    std::cout << \"Hello World!\" << std::endl;\n");
    main_cpp.print("\n    return 0;\n");
    main_cpp.print("{}\n", brace_close);
    main_cpp.flush();
    main_cpp.close();
}

std::optional<std::string> plusutil::get_project_name(const std::vector<std::string>& args)
{
    auto _it = std::ranges::find(args, "new");
    // auto it = std::find(args.begin(), args.end(), "new");
    if (_it != args.end() && _it + 1 != args.end())
    {
        return *(++_it);
    }

    return {};
}