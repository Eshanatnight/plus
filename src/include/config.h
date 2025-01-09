#pragma once
#include <filesystem>
#include <string>
#include <toml++/toml.hpp>
#include <utility>

struct Config {
	struct Project {
		std::string name;
		std::string kind;
		std::string buildDir;
		std::string repo;
		std::string license;
		std::vector<std::string> cmakeDefines;
		Project() = default;
		explicit Project(const toml::table& tbl);
		// NOLINTNEXTLINE
		Project(std::string name, std::string_view kind, std::string_view buildDir) :
			name(std::move(name)), kind(kind), buildDir(buildDir) {}
	};

	struct Author {
		std::string name;
		std::string email;

		Author() = default;
		explicit Author(const toml::table& tbl);
	};

	Project proj;
	Author author;

public:

	Config()  = delete;
	~Config() = default;
	Config(const std::string& appName, std::string_view packageType, std::string_view buildDir);
	explicit Config(const std::filesystem::path& path);
	[[nodiscard]] auto toTomlTable() const -> toml::table;
};
