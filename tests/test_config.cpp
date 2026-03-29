#include "config.h"

#include <catch2/catch_test_macros.hpp>
#include <toml++/toml.hpp>

TEST_CASE("Config scaffold constructor sets defaults", "[config]") {
	Config c("HelloWorld", "lib", "out");
	REQUIRE(c.proj.name == "HelloWorld");
	REQUIRE(c.proj.kind == "lib");
	REQUIRE(c.proj.buildDir == "out");
	REQUIRE(c.proj.version == "0.1.0");
	REQUIRE(c.proj.cpp_std == "17");
	REQUIRE(c.proj.cmakeDefines.empty());
}

TEST_CASE("Config::toTomlTable round-trips key fields", "[config]") {
	Config c("demo", "bin", "build");
	c.author.name  = "Ada";
	c.author.email = "ada@example.com";
	c.proj.repo	   = "https://example.com/demo";

	const toml::table tbl = c.toTomlTable();
	REQUIRE(tbl["project"]["name"].value_or(std::string{}) == "demo");
	REQUIRE(tbl["project"]["kind"].value_or(std::string{}) == "bin");
	REQUIRE(tbl["project"]["version"].value_or(std::string{}) == "0.1.0");
	REQUIRE(tbl["project"]["cpp_std"].value_or(std::string{}) == "17");
	REQUIRE(tbl["project"]["buildDir"].value_or(std::string{}) == "build");
	REQUIRE(tbl["project"]["repo"].value_or(std::string{}) == "https://example.com/demo");
	REQUIRE(tbl["author"]["name"].value_or(std::string{}) == "Ada");
	REQUIRE(tbl["author"]["email"].value_or(std::string{}) == "ada@example.com");
}

TEST_CASE("Project parses full plus.toml project section", "[config]") {
	const char* doc = R"(
[project]
name = "X"
kind = "bin"
version = "2.0.0"
cpp_std = "20"
buildDir = "b"
repo = "r"
cmakeDefines = ["FOO=1", "BAR=2"]

[author]
name = "n"
email = "e"
)";

	const toml::table tbl = toml::parse(doc);
	Config::Project p(tbl);
	REQUIRE(p.name == "X");
	REQUIRE(p.kind == "bin");
	REQUIRE(p.version == "2.0.0");
	REQUIRE(p.cpp_std == "20");
	REQUIRE(p.buildDir == "b");
	REQUIRE(p.repo == "r");
	REQUIRE(p.cmakeDefines.size() == 2);
	REQUIRE(p.cmakeDefines[0] == "FOO=1");
	REQUIRE(p.cmakeDefines[1] == "BAR=2");

	Config::Author a(tbl);
	REQUIRE(a.name == "n");
	REQUIRE(a.email == "e");
}

TEST_CASE("Project tolerates missing cmakeDefines", "[config]") {
	const toml::table tbl = toml::parse(R"(
[project]
name = "a"
kind = "bin"
version = "1.0.0"
cpp_std = "17"
buildDir = "build"
repo = ""
)");
	Config::Project p(tbl);
	REQUIRE(p.cmakeDefines.empty());
}

TEST_CASE("Project ignores non-array cmakeDefines", "[config]") {
	const toml::table tbl = toml::parse(R"(
[project]
name = "a"
kind = "bin"
version = "1.0.0"
cpp_std = "17"
buildDir = "build"
repo = ""
cmakeDefines = "not-an-array"
)");
	Config::Project p(tbl);
	REQUIRE(p.cmakeDefines.empty());
}

TEST_CASE("Conan parses requires and output_folder", "[config]") {
	const toml::table tbl = toml::parse(R"(
[conan]
output_folder = "vendor"
requires = [ "fmt/10.2.0", "zlib/1.3.1" ]
)");
	Config::Conan c(tbl);
	REQUIRE(c.output_folder == "vendor");
	REQUIRE(c.requires.size() == 2);
	REQUIRE(c.requires[0] == "fmt/10.2.0");
	REQUIRE(c.requires[1] == "zlib/1.3.1");
}

TEST_CASE("toTomlTable includes conan section", "[config]") {
	Config c("demo", "bin", "build");
	c.conan.
		requires
		.push_back("catch2/3.5.0");
	c.conan.output_folder = "deps";
	const toml::table tbl = c.toTomlTable();
	REQUIRE(tbl["conan"]["output_folder"].value_or(std::string{}) == "deps");
	REQUIRE(tbl["conan"]["requires"].is_array());
}
