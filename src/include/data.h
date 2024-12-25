#pragma once

#include <string>

namespace Ident {
	static constexpr std::string_view plusMyAPP = "PLUS_MY_APP";
}

namespace FileContents {

	static constexpr std::string_view mainCPP = "#include <iostream>\n"
												"\n"
												"int main() {\n"
												"\n"
												"    std::cout << \"Hello World\\n\";\n"
												"\n"
												"    return 0;\n"
												"}\n"
												"\n";

	static constexpr std::string_view gitIgnore = "# IDE and Code Editor Stuff\n"
												  "/.vs\n"
												  "/.vscode\n"
												  "/.idea\n"
												  "\n"
												  "# Build Dirs\n"
												  "[Bb][Ii][Nn]\n"
												  "[Bb][Uu][Ii][Ll][Dd]\n"
												  "[Oo][Uu][Tt]\n"
												  "\n"
												  "[Oo][Bb][Jj]\n"
												  "\n"
												  "# Executables\n"
												  "*.exe\n"
												  "*.obj\n"
												  "\n";

	static std::string cmakeLists = "cmake_minimum_required(VERSION 3.10)\n"
									"\n"
									"project(PLUS_MY_APP)\n"
									"\n"
									"add_executable(${PROJECT_NAME} ./src/main.cpp)\n"
									"\n";
}

namespace FilePaths {
	using namespace std::string_view_literals;

	static constexpr auto SRC_PATH	 = "src"sv;
	static constexpr auto BUILD_PATH = "build"sv;
	static constexpr auto MAIN_CPP	 = "main.cpp"sv;
	static constexpr auto GITIGNORE	 = ".gitignore"sv;
	static constexpr auto CMAKELISTS = "CMakeLists.txt"sv;
}
