#include "utils.h"

#include "log.h"

#include <sstream>

auto utils::toml_to_string(const toml::table& tbl) -> std::string {

	std::stringstream tomlStream;
	tomlStream << toml::toml_formatter(tbl);

	LOG_DEBUG(tomlStream.str());

	return tomlStream.str();
}
