#pragma once

#include <cstdlib>
#include <optional>
#include <structopt.hpp>

struct Cli {
	enum class Type {
		bin,
		lib
	};
	struct Init : structopt::sub_command {
		std::optional<Type> packageType = Type::bin;
	};
	struct New : structopt::sub_command {
		std::string projectName;
		std::optional<Type> packageType = Type::bin;
	};

	// sub commands
	Init init;
	New new_;
};

STRUCTOPT(Cli::Init, packageType);
STRUCTOPT(Cli::New, projectName, packageType);
STRUCTOPT(Cli, init, new_);
