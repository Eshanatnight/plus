#pragma once

#include <cstdlib>
#include <optional>
#include <string>
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
		/** `cmake --build` parallelism (`-j`); omit for CMake default. */
		std::optional<int> jobs;
	};

	struct Setup : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
		/** Run `conan install` and configure CMake with Conan’s toolchain. */
		std::optional<bool> conan = false;
		/** CMake generator (`cmake -G …`); omit for CMake default. */
		std::optional<std::string> generator;
	};

	struct Deps : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
		/** Only regenerate `conanfile.txt`; do not run Conan. */
		std::optional<bool> sync_only = false;
		/** Print installed dependency list. */
		std::optional<bool> list = false;
		/** Print resolved dependency graph. */
		std::optional<bool> tree = false;
	};

	struct Add : structopt::sub_command {
		/** Conan reference, e.g. `fmt/10.2.1` */
		std::string package_ref;
	};

	struct Remove : structopt::sub_command {
		/** Conan reference to remove, e.g. `fmt/10.2.1` */
		std::string package_ref;
	};

	struct Run : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
		std::optional<std::string> generator;
		std::optional<int> jobs;
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
		std::optional<std::string> generator;
	};

	struct Check : structopt::sub_command {
		std::optional<bool> verbose = false;
	};

	struct Install : structopt::sub_command {
		std::optional<std::string> prefix;
		std::optional<BuildType> type = BuildType::dbg;
	};

	struct Release : structopt::sub_command {
		std::optional<bool> strip = false;
		std::optional<int> jobs;
	};

	struct Bench : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
	};

	struct Watch : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
		std::optional<int> jobs;
		/** Poll interval in seconds. */
		std::optional<int> interval;
	};

	struct Update : structopt::sub_command {
		std::optional<BuildType> type = BuildType::dbg;
	};

	struct Version : structopt::sub_command {
		/** Semver part to bump: major, minor, patch */
		std::string part;
	};

	struct ConfigSet : structopt::sub_command {
		std::string key;
		std::string value;
	};

	struct Completions : structopt::sub_command {
		/** Shell name: bash, zsh, fish */
		std::string shell;
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
	Remove remove;
	Check check;
	Install install;
	Release release;
	Bench bench;
	Watch watch;
	Update update;
	Version version;
	ConfigSet config;
	Completions completions;
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
STRUCTOPT(Cli::Build, type, jobs);
STRUCTOPT(Cli::Setup, type, conan, generator);
STRUCTOPT(Cli::Deps, type, sync_only, list, tree);
STRUCTOPT(Cli::Add, package_ref);
STRUCTOPT(Cli::Remove, package_ref);
STRUCTOPT(Cli::New, projectName, kind, no_git);
STRUCTOPT(Cli::Run, type, generator, jobs);
STRUCTOPT(Cli::Clean, quiet);
STRUCTOPT(Cli::Fmt, check);
STRUCTOPT(Cli::Tidy, fix);
STRUCTOPT(Cli::Show, verbose);
STRUCTOPT(Cli::Test, type, generator);
STRUCTOPT(Cli::Check, verbose);
STRUCTOPT(Cli::Install, prefix, type);
STRUCTOPT(Cli::Release, strip, jobs);
STRUCTOPT(Cli::Bench, type);
STRUCTOPT(Cli::Watch, type, jobs, interval);
STRUCTOPT(Cli::Update, type);
STRUCTOPT(Cli::Version, part);
STRUCTOPT(Cli::ConfigSet, key, value);
STRUCTOPT(Cli::Completions, shell);
STRUCTOPT(Cli, init, new_, build, setup, run, clean, fmt, tidy, show, test, deps, add, remove, check, install, release, bench, watch, update, version, config, completions);
