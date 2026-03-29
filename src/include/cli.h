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
		std::optional<Type> kind   = Type::bin;
		std::optional<bool> no_git = false;
	};
	struct New : structopt::sub_command {
		std::string projectName;
		std::optional<Type> kind   = Type::bin;
		std::optional<bool> no_git = false;
	};

	enum class BuildType {
		dbg,
		rel
	};
	struct Build : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
	};

	struct Setup : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
		/** Run `conan install` and configure CMake with Conan’s toolchain. */
		std::optional<bool> conan = false;
	};

	struct Deps : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
		/** Only regenerate `conanfile.txt`; do not run Conan. */
		std::optional<bool> sync_only = false;
	};

	struct Add : structopt::sub_command {
		/** Conan reference, e.g. `fmt/10.2.1` */
		std::string package_ref;
	};

	struct Run : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
	};

	struct Clean : structopt::sub_command {
		std::optional<bool> quiet = false;
	};

	struct Fmt : structopt::sub_command {
		/** If true, only verify formatting (clang-format --dry-run). */
		std::optional<bool> check = false;
	};

	struct Tidy : structopt::sub_command {
		/** Apply clang-tidy fix-its where supported (mutates sources). */
		std::optional<bool> fix = false;
	};

	struct Show : structopt::sub_command {
		std::optional<bool> verbose = false;
	};

	struct Test : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
	};

	// sub commands
	Init init;
	New new_;
	Build build;
	Setup setup;
	Run run;
	Clean clean;
	Fmt fmt;
	Tidy tidy;
	Show show;
	Test test;
	Deps deps;
	Add add;
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

static inline bool skipGit(const Cli& cli) {
	if(cli.init.has_value() && cli.init.no_git.value_or(false)) {
		return true;
	}
	if(cli.new_.has_value() && cli.new_.no_git.value_or(false)) {
		return true;
	}
	return false;
}

static constexpr auto buildTypeConfig(Cli::BuildType t) -> std::string_view {
	return t == Cli::BuildType::rel ? "Release" : "Debug";
}

STRUCTOPT(Cli::Init, kind, no_git);
STRUCTOPT(Cli::Build, type);
STRUCTOPT(Cli::Setup, type, conan);
STRUCTOPT(Cli::Deps, type, sync_only);
STRUCTOPT(Cli::Add, package_ref);
STRUCTOPT(Cli::New, projectName, kind, no_git);
STRUCTOPT(Cli::Run, type);
STRUCTOPT(Cli::Clean, quiet);
STRUCTOPT(Cli::Fmt, check);
STRUCTOPT(Cli::Tidy, fix);
STRUCTOPT(Cli::Show, verbose);
STRUCTOPT(Cli::Test, type);
STRUCTOPT(Cli, init, new_, build, setup, run, clean, fmt, tidy, show, test, deps, add);
