#include "cli.h"
#include "git.h"

#include <filesystem>
#include <future>
#include <iostream>

auto main(int argc, char** argv) -> int {
	try {

		std::vector<std::future<void>> futures;
		futures.reserve(5);
		auto app	   = structopt::app("plus");
		const auto cli = app.parse<Cli>(argc, argv);

		std::filesystem::path pwd = std::filesystem::current_path();

		auto initialized = initializGitRepo(cli, pwd);

		if(!initialized.has_value()) {
			std::cerr << app.help();
			return EXIT_FAILURE;
		}
		futures.emplace_back(std::move(initialized.value()));

		for(auto& fut: futures) fut.get();

	} catch(const structopt::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
