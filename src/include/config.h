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

		Project() = delete;
		explicit Project(std::filesystem::path& path);
		// NOLINTNEXTLINE
		Project(std::string name, std::string_view kind, std::string_view buildDir) :
			name(std::move(name)), kind(kind), buildDir(buildDir) {}
	};

	struct Author {
		std::string name;
		std::string email;

		Author() = default;
		explicit Author(std::filesystem::path& path);
	};

	Project proj;
	Author author;

public:

	Config()  = delete;
	~Config() = default;
	Config(std::string& appName, std::string_view packageType, std::string_view buildDir);
	explicit Config(std::filesystem::path configPath);
	[[nodiscard]] auto toTomlTable() const -> toml::table;
};
