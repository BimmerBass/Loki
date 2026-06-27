// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "pch.hpp"
#include "Loki/util/stringops.hpp"

namespace util_tests
{
	using namespace loki::util;

	TEST_CASE("split handles empty and separator-free strings", "[util][stringops]")
	{
		REQUIRE(split("", '\t') == std::vector<std::string>{});
		REQUIRE(split("hello, world!", '|') == std::vector<std::string>{"hello, world!"});
	}

	TEST_CASE("split can keep or remove empty fields", "[util][stringops]")
	{
		const auto expected_with_empty = std::vector<std::string>{"he", "", "o, wor", "d!"};
		const auto expected_without_empty = std::vector<std::string>{"he", "o, wor", "d!"};

		REQUIRE(split("hello, world!", 'l', true) == expected_with_empty);
		REQUIRE(split("hello, world!", 'l', false) == expected_without_empty);
	}

	TEST_CASE("join handles empty and populated vectors", "[util][stringops]")
	{
		REQUIRE(join(std::vector<std::string>{}, ',') == "");
		REQUIRE(join(std::vector<std::string>{"hello"}, ',') == "hello");
		REQUIRE(join(std::vector<std::string>{"hello", "world"}, ',') == "hello,world");
	}

	TEST_CASE("lowercase preserves whitespace and normalizes letters", "[util][stringops]")
	{
		REQUIRE(lowercase("") == "");
		REQUIRE(lowercase(" \t") == " \t");
		REQUIRE(lowercase("abc123") == "abc123");
		REQUIRE(lowercase("AbC123") == "abc123");
	}

	TEST_CASE("uppercase preserves whitespace and normalizes letters", "[util][stringops]")
	{
		REQUIRE(uppercase("") == "");
		REQUIRE(uppercase(" \t") == " \t");
		REQUIRE(uppercase("ABC123") == "ABC123");
		REQUIRE(uppercase("AbC123") == "ABC123");
	}

	TEST_CASE("collapse_whitespace still needs direct coverage", "[util][stringops][stub]")
	{
		FAIL("TODO: add direct coverage for collapse_whitespace() now that it is used by the UCI parser.");
	}

	TEST_CASE("hash and _hash agree for equivalent strings", "[util][stringops][stub]")
	{
		const std::string all_string = "all";
		const std::string_view all_view = all_string;

		REQUIRE(hash(all_string) == hash(all_view));
		REQUIRE(hash("all") != hash("quiet"));
		REQUIRE(hash(all_string) == "all"_hash);
	}
}
