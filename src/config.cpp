#include "log.h"

#include <algorithm>
#include <config.h>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <toml++/impl/array.hpp>
#include <vector>

Config::Project::Project(const toml::table& tbl) {
	kind	 = tbl["project"]["kind"].value_or("");
	name	 = tbl["project"]["name"].value_or("");
	buildDir = tbl["project"]["buildDir"].value_or("");
	repo	 = tbl["project"]["repo"].value_or("");

	cmakeDefines.reserve(5);
	const auto& tableCmakeDefines = tbl["project"]["cmakeDefines"];

	if(tableCmakeDefines.is_array()) {
		tableCmakeDefines.as_array()->for_each([this](auto&& elem) {
			if constexpr(toml::is_string<decltype(elem)>) {
				cmakeDefines.emplace_back(elem.value_or(""));
			} else {
				// Deal with the error case of not having the key
				LOG_DEBUG_MSG("type of `elem`: ");
				LOG_DEBUG_MSG(elem.type());
			}
		});
	} else {
		LOG_DEBUG_MSG("cmakeDefines TABLE IS UNDEFINED OR IS NOT AN ARRAY.");
		LOG_DEBUG_MSG(tableCmakeDefines.type());

		std::cerr << "[Error]: Failed to parse Project or Author data\n";
		std::quick_exit(1);
	}
}

Config::Author::Author(const toml::table& tbl) {
	// sets a default value if the key is not found
	// is that correct?
	name  = tbl["author"]["name"].value_or("");
	email = tbl["author"]["email"].value_or("");
}

Config::Config(const std::filesystem::path& configPath) {

	/**
	 * should assert the file has the correct structure
	 *	i dont want to deal with an exception here
		[author]
		email = ''
		name = ''

		[project]
		buildDir = 'build'
		kind = 'bin'
		name = 'sss'
		repo = ''
		cmakeDefines=[]
	 */

	toml::table tbl;

	try {
		tbl = toml::parse_file(configPath.c_str());
	} catch(const toml::parse_error& err) {
		std::cerr << "Failed to parse plus.toml\n";
		std::cerr << "[Error]: " << err << '\n';
		std::quick_exit(1);
	}

	proj   = Project(tbl);
	author = Author(tbl);
}

Config::Config(
	const std::string& appName, std::string_view packageType, std::string_view buildDir) :
	proj(appName, packageType, buildDir) {}

auto Config::toTomlTable() const -> toml::table {
	auto cmakeDefs = toml::array{};
	std::for_each(proj.cmakeDefines.begin(),
		proj.cmakeDefines.end(),
		[&cmakeDefs](const auto& elem) { cmakeDefs.push_back(elem); });

	auto tbl = toml::table{
		{ "project",
			toml::table{ { "name", proj.name },
			{ "kind", proj.kind },
			{ "buildDir", proj.buildDir },
			{ "repo", proj.repo },
			{ "cmakeDefines", cmakeDefs } }												},
		{  "author", toml::table{ { "name", author.name }, { "email", author.email } } }
	};

	return tbl;
}
