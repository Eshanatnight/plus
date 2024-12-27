#pragma once

#include <cstdlib>
#include <optional>
#include <string_view>
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

static constexpr auto TypeToString(Cli::Type type) -> std::string_view {
	switch(type) {
	case Cli::Type::bin :
		return "bin";
	case Cli::Type::lib :
		return "lib";
	}
}

static inline bool isLib(const Cli& cli) {
	bool ret = false;

	if(cli.init.has_value()) {
		const auto type = cli.init.packageType.value();

		switch(type) {
		case Cli::Type::bin :
			break;
		case Cli::Type::lib :
			ret = true;
			break;
		}
	} else if(cli.new_.has_value()) {
		const auto type = cli.new_.packageType.value();

		switch(type) {
		case Cli::Type::bin :
			break;
		case Cli::Type::lib :
			ret = true;
			break;
		}
	} else {
		std::cerr << "Unknown error while parsing packageType\n";
		std::exit(1);
	}

	return ret;
}
STRUCTOPT(Cli::Init, packageType);
STRUCTOPT(Cli::New, projectName, packageType);
STRUCTOPT(Cli, init, new_);
