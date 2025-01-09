#pragma once

#include <toml++/toml.h>
namespace utils {
	auto toml_to_string(const toml::table& tbl) -> std::string;
};
