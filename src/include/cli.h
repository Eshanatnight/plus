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
		std::optional<Type> kind = Type::bin;
	};
	struct New : structopt::sub_command {
		std::string projectName;
		std::optional<Type> kind = Type::bin;
	};

	enum class BuildType {
		dbg,
		rel
	};
	struct Build : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
	};

	// sub commands
	Init init;
	New new_;
	Build build;
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
		const auto type = cli.init.kind.value();

		switch(type) {
		case Cli::Type::bin :
			break;
		case Cli::Type::lib :
			ret = true;
			break;
		}
	} else if(cli.new_.has_value()) {
		const auto type = cli.new_.kind.value();

		switch(type) {
		case Cli::Type::bin :
			break;
		case Cli::Type::lib :
			ret = true;
			break;
		}
	}

	return ret;
}
STRUCTOPT(Cli::Init, kind);
STRUCTOPT(Cli::Build, type);
STRUCTOPT(Cli::New, projectName, kind);
STRUCTOPT(Cli, init, new_, build);
