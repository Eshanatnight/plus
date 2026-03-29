#pragma once

#include <string>
#include <string_view>
#include <toml++/toml.h>

namespace utils {
	auto toml_to_string(const toml::table& tbl) -> std::string;
	/** Lowercase path-safe slug (e.g. include dir / source basename). */
	auto project_slug(std::string_view name) -> std::string;
	/** Valid C++ namespace identifier derived from the project name. */
	auto cpp_namespace_ident(std::string_view name) -> std::string;
} // namespace utils
