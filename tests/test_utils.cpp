#include "utils.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("project_slug lowercases and collapses separators", "[utils]") {
	REQUIRE(utils::project_slug("MyApp") == "myapp");
	REQUIRE(utils::project_slug("My Cool App") == "my_cool_app");
	REQUIRE(utils::project_slug("foo-bar") == "foo_bar");
	REQUIRE(utils::project_slug("  trim  ") == "trim");
}

TEST_CASE("project_slug empty or non-alphanumeric falls back", "[utils]") {
	REQUIRE(utils::project_slug("") == "project");
	REQUIRE(utils::project_slug("___") == "project");
}

TEST_CASE("cpp_namespace_ident matches slug when valid", "[utils]") {
	REQUIRE(utils::cpp_namespace_ident("Hello") == "hello");
}

TEST_CASE("cpp_namespace_ident prefixes digit-leading slug", "[utils]") {
	REQUIRE(utils::cpp_namespace_ident("2fast") == "_n2fast");
}
