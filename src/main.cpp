#include "cli.h"
#include "file_control.h"

#include <filesystem>
#include <iostream>
#include <string>

auto main(int argc, char** argv) -> int {
	try {

		auto app	   = structopt::app("plus");
		const auto cli = app.parse<Cli>(argc, argv);

		std::filesystem::path pwd = std::filesystem::current_path();
		std::string appName;
		auto initialized = initializeAndRun(cli, pwd, appName);

		if(!initialized) {
			std::cerr << app.help();
			return EXIT_FAILURE;
		}

	} catch(const structopt::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
