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
#include "Loki/options/option.hpp"
#include "Loki/options/registry.hpp"

namespace options_tests
{
	using namespace loki::options;

	namespace
	{
		std::string output_for(std::string name, option_type type)
		{
			std::ostringstream output;
			loki_option{
				.name = std::move(name),
				.type = std::move(type),
				.apply_callback = {}
			}.print(output);
			return output.str();
		}
	}

	TEST_CASE("checkbox options print their boolean default", "[options][output]")
	{
		REQUIRE(output_for("Ponder", checkbox{ true }) ==
			"option name Ponder type check default true\n");
		REQUIRE(output_for("UCI_Chess960", checkbox{ false }) ==
			"option name UCI_Chess960 type check default false\n");
	}

	TEST_CASE("spin options print their default and bounds", "[options][output]")
	{
		REQUIRE(output_for("Hash", spin{ 16, 1, 1'048'576 }) ==
			"option name Hash type spin default 16 min 1 max 1048576\n");
	}

	TEST_CASE("combo options print their default and values in order", "[options][output]")
	{
		REQUIRE(output_for(
			"Style",
			combo{ "Normal", { "Solid", "Normal", "Risky" } }) ==
			"option name Style type combo default Normal"
			" var Solid var Normal var Risky\n");
	}

	TEST_CASE("string options print their default", "[options][output]")
	{
		REQUIRE(output_for("SyzygyPath", string{ "<empty>" }) ==
			"option name SyzygyPath type string default <empty>\n");
	}

	TEST_CASE("button options print without a value", "[options][output]")
	{
		REQUIRE(output_for("Clear Hash", button{}) ==
			"option name Clear Hash type button\n");
	}

	TEST_CASE("checkbox accepts every boolean value", "[options][validation]")
	{
		const checkbox option{ false };

		REQUIRE(option.is_valid(false));
		REQUIRE(option.is_valid(true));
	}

	TEST_CASE("spin accepts values within its inclusive range", "[options][validation]")
	{
		const spin option{ 5, 1, 10 };

		REQUIRE_FALSE(option.is_valid(0));
		REQUIRE(option.is_valid(1));
		REQUIRE(option.is_valid(5));
		REQUIRE(option.is_valid(10));
		REQUIRE_FALSE(option.is_valid(11));
	}

	TEST_CASE("combo accepts only listed values without reserved tokens",
		"[options][validation]")
	{
		const combo option{
			"Normal",
			{
				"Solid",
				"Normal",
				"Risky",
				"filename",
				"value-type",
				"file name . txt",
				"some value"
			}
		};

		REQUIRE(option.is_valid("Solid"));
		REQUIRE(option.is_valid("Normal"));
		REQUIRE(option.is_valid("filename"));
		REQUIRE(option.is_valid("value-type"));
		REQUIRE_FALSE(option.is_valid("Unknown"));
		REQUIRE_FALSE(option.is_valid("file name . txt"));
		REQUIRE_FALSE(option.is_valid("some value"));
	}

	TEST_CASE("string rejects values containing reserved tokens",
		"[options][validation]")
	{
		const string option{ "" };

		REQUIRE(option.is_valid(""));
		REQUIRE(option.is_valid("SyzygyPath"));
		REQUIRE(option.is_valid("filename.txt"));
		REQUIRE(option.is_valid("some-value"));
		REQUIRE_FALSE(option.is_valid("file name . txt"));
		REQUIRE_FALSE(option.is_valid("some\tvalue"));
	}

	TEST_CASE("registry rejects option names containing reserved tokens",
		"[options][registry]")
	{
		registry options;

		REQUIRE_NOTHROW(options.add(
			"filename",
			string{ "" },
			[](const std::string&) {}));
		REQUIRE_NOTHROW(options.add(
			"evaluation",
			[]() {}));
		REQUIRE_THROWS(options.add(
			"file name",
			string{ "" },
			[](const std::string&) {}));
		REQUIRE_THROWS(options.add(
			"current value",
			[]() {}));
	}
}
