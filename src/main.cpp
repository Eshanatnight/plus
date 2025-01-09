#include "cli.h"
#include "file_control.h"

#include <cstdlib>
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

		switch(initialized) {
		case InitializationError::OK :
			return EXIT_SUCCESS;

		case InitializationError::BUILD_DIR_DOES_NOT_EXIST :
		case InitializationError::CMAKELISTS_NOT_FOUND :
		case InitializationError::PLUS_TOML_NOT_FOUND :
			return EXIT_FAILURE;

		case InitializationError::INVALID_ARG :
		case InitializationError::UNKNOWN :
			std::cerr << app.help();
			return EXIT_FAILURE;
		}

	} catch(const structopt::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
