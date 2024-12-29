#include <config.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <toml++/impl/table.hpp>
#include <toml++/toml.hpp>

Config::Project::Project(std::filesystem::path& path) {

	toml::table tbl;
	try {
		tbl = toml::parse_file(path.c_str());
	} catch(const toml::parse_error& err) {
		std::cerr << "Failed to Parse plus.toml:\n" << err << "\n";
		std::exit(1);
	}
	kind	 = tbl["project"]["kind"].value_or("");
	name	 = tbl["project"]["name"].value_or("");
	buildDir = tbl["project"]["buildDir"].value_or("");
	repo	 = tbl["project"]["repo"].value_or("");
}

Config::Author::Author(std::filesystem::path& path) {

	toml::table tbl;
	try {
		tbl = toml::parse_file(path.c_str());
	} catch(const toml::parse_error& err) {
		std::cerr << "Failed to Parse plus.toml:\n" << err << "\n";
		std::exit(1);
	}

	name  = tbl["author"]["name"].value_or("");
	email = tbl["author"]["email"].value_or("");
}

Config::Config(std::filesystem::path configPath) : proj(configPath), author(configPath) {}

Config::Config(std::string& appName, std::string_view packageType, std::string_view buildDir) :
	proj(appName, packageType, buildDir) {}

auto Config::toTomlTable() const -> toml::table {
	auto tbl = toml::table{
		{ "project",
			toml::table{ { "name", proj.name },
			{ "kind", proj.kind },
			{ "buildDir", proj.buildDir },
			{ "repo", proj.repo } }													  },
		{  "author", toml::table{ { "name", author.name }, { "email", author.email } } }
	};

	return tbl;
}
