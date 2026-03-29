#include "utils.h"

#include "log.h"

#include <cctype>
#include <sstream>

auto utils::toml_to_string(const toml::table& tbl) -> std::string {

	std::stringstream tomlStream;
	tomlStream << toml::toml_formatter(tbl);

	LOG_DEBUG(tomlStream.str());

	return tomlStream.str();
}

auto utils::project_slug(std::string_view name) -> std::string {
	std::string out;
	out.reserve(name.size());
	bool lastUnderscore = false;
	for(unsigned char uc: name) {
		if(std::isalnum(uc) != 0) {
			out.push_back(static_cast<char>(std::tolower(uc)));
			lastUnderscore = false;
		} else {
			if(!out.empty() && !lastUnderscore) {
				out.push_back('_');
				lastUnderscore = true;
			}
		}
	}
	while(!out.empty() && out.back() == '_') {
		out.pop_back();
	}
	if(out.empty()) {
		return "project";
	}
	return out;
}

auto utils::cpp_namespace_ident(std::string_view name) -> std::string {
	auto slug = project_slug(name);
	if(slug.empty()) {
		return "project";
	}
	if(std::isdigit(static_cast<unsigned char>(slug.front())) != 0) {
		slug.insert(slug.begin(), 'n');
		slug.insert(slug.begin(), '_');
	}
	return slug;
}
