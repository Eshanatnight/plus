#pragma once

#include <string>
#include <string_view>
#include <toml++/toml.hpp>

namespace Ident {
	static constexpr std::string_view plusMyAPP		 = "PLUS_MY_APP";
	static constexpr std::string_view plusCppStd	 = "PLUS_CPP_STD";
	static constexpr std::string_view plusSlug		 = "PLUS_SLUG";
	static constexpr std::string_view plusNs		 = "PLUS_NS";
	static constexpr std::string_view plusDisplayName = "PLUS_DISPLAY_NAME";
	static constexpr std::string_view addExecuteable = "add_executable";
	static constexpr std::string_view addLibrary	 = "add_library";
}  // namespace Ident

namespace FileContents {

	static constexpr std::string_view mainCPP =
		"#include <iostream>\n"
		"\n"
		"int main() {\n"
		"    std::cout << \"Hello, world!\\n\";\n"
		"    return 0;\n"
		"}\n";

	static constexpr std::string_view gitIgnore = "# IDE and Code Editor Stuff\n"
												  "/.vs\n"
												  "/.vscode\n"
												  "/.idea\n"
												  "\n"
												  "# Build Dirs\n"
												  "[Bb][Ii][Nn]\n"
												  "[Bb][Uu][Ii][Ll][Dd]\n"
												  "[Oo][Uu][Tt]\n"
												  "deps/\n"
												  "\n"
												  "[Oo][Bb][Jj]\n"
												  "\n"
												  "# Executables\n"
												  "*.exe\n"
												  "*.obj\n"
												  "\n";

	static constexpr std::string_view gitAttributes = "* text=auto\n";

	/** Placeholders: PLUS_MY_APP, PLUS_CPP_STD */
	static std::string cmakeBin = "cmake_minimum_required(VERSION 3.14)\n"
								  "\n"
								  "project(PLUS_MY_APP CXX)\n"
								  "\n"
								  "set(CMAKE_CXX_STANDARD PLUS_CPP_STD)\n"
								  "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
								  "set(CMAKE_CXX_EXTENSIONS OFF)\n"
								  "\n"
								  "add_executable(${PROJECT_NAME} src/main.cpp)\n"
								  "\n";

	/** Placeholders: PLUS_MY_APP, PLUS_CPP_STD, PLUS_SLUG */
	static std::string cmakeLib = "cmake_minimum_required(VERSION 3.14)\n"
								  "\n"
								  "project(PLUS_MY_APP CXX)\n"
								  "\n"
								  "set(CMAKE_CXX_STANDARD PLUS_CPP_STD)\n"
								  "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
								  "set(CMAKE_CXX_EXTENSIONS OFF)\n"
								  "\n"
								  "add_library(${PROJECT_NAME} STATIC\n"
								  "    src/PLUS_SLUG.cpp\n"
								  ")\n"
								  "\n"
								  "target_include_directories(${PROJECT_NAME} PUBLIC\n"
								  "    \"${CMAKE_CURRENT_SOURCE_DIR}/include\"\n"
								  ")\n"
								  "\n";

}  // namespace FileContents

namespace FilePaths {
	using namespace std::string_view_literals;

	static constexpr auto SRC_PATH	 = "src"sv;
	static constexpr auto INC_PATH	 = "include"sv;
	static constexpr auto BUILD_PATH = "build"sv;
	static constexpr auto MAIN_CPP	 = "main.cpp"sv;
	static constexpr auto GITIGNORE	 = ".gitignore"sv;
	static constexpr auto GITATTRIBUTES = ".gitattributes"sv;
	static constexpr auto CMAKELISTS = "CMakeLists.txt"sv;
	static constexpr auto README_MD	 = "README.md"sv;
	static constexpr auto LICENSE		 = "LICENSE"sv;
	static constexpr auto CONFIGURE_SH = "configure.sh"sv;

	static constexpr auto PLUSTOML = "plus.toml"sv;
}  // namespace FilePaths
