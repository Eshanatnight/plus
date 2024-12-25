#include "cli.h"
#include "control.h"
#include "file_control.h"
#include "git.h"

#include <filesystem>
#include <future>
#include <iostream>
#include <string>

auto main(int argc, char** argv) -> int {
	try {

		// TODO: make it an array
		std::vector<std::future<void>> futures;
		futures.reserve(5);
		auto app	   = structopt::app("plus");
		const auto cli = app.parse<Cli>(argc, argv);

		std::filesystem::path pwd = std::filesystem::current_path();
		std::string appName;
		auto initialized = initialize(cli, pwd, appName);

		if(!initialized) {
			std::cerr << app.help();
			return EXIT_FAILURE;
		}

		futures.emplace_back(makeBuildDir(pwd));
		makeFiles(futures, pwd, appName);
		resolve(futures);

	} catch(const structopt::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
