#include "structopt.hpp"

#include <cstdlib>
#include <iostream>

struct Cli {
	struct Init : structopt::sub_command {
		bool i;
	};
	struct New : structopt::sub_command {
		std::string projectName;
	};

	// sub commands
	Init init;
	New new_;
};

STRUCTOPT(Cli::Init, i);
STRUCTOPT(Cli::New, projectName);
STRUCTOPT(Cli, init, new_);

auto main(int argc, char** argv) -> int {
#ifndef DEBUG
	try {
#endif
		const auto cli = structopt::app("plus").parse<Cli>(argc, argv);
		if(cli.new_.has_value()) {
			std::cout << "Hey New has value: " << cli.new_.projectName << std::endl;
		}
#ifndef DEBUG
	} catch(const structopt::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
#endif
	return EXIT_SUCCESS;
}
