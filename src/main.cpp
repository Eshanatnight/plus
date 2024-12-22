#include "cli.h"
#include "git.h"

#include <filesystem>
#include <iostream>

auto main(int argc, char** argv) -> int {
	try {
		auto app	   = structopt::app("plus");
		const auto cli = app.parse<Cli>(argc, argv);

		std::filesystem::path pwd = std::filesystem::current_path();
		if(cli.init.has_value()) {
			initializGitRepo(pwd, false);
		} else if(cli.new_.has_value()) {
			pwd = pwd / cli.new_.projectName.c_str();
			initializGitRepo(pwd, true);
		} else {
			std::cerr << app.help();
			return EXIT_FAILURE;
		}

	} catch(const structopt::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
