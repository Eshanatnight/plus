#include "cli.h"
#include "git.h"

#include <filesystem>
#include <iostream>

auto main(int argc, char** argv) -> int {
#ifndef DEBUG
	try {
#endif
		const auto cli = structopt::app("plus").parse<Cli>(argc, argv);

		std::filesystem::path pwd = std::filesystem::current_path();
		if(cli.init.has_value()) {
			initializGitRepo(pwd, false);
		} else if(cli.new_.has_value()) {
			pwd = pwd / cli.new_.projectName.c_str();
			initializGitRepo(pwd, true);
		} else {
			std::cerr << "Panic: This should be unreachable" << std::endl;
			std::quick_exit(1);
		}

#ifndef DEBUG
	} catch(const structopt::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
#endif
	return EXIT_SUCCESS;
}
