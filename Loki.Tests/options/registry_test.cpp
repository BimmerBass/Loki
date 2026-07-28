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
#include "Loki/defs.hpp"
#include "Loki/options/registry.hpp"

namespace options_tests
{
	using namespace loki::options;

	TEST_CASE("registry accepts valid names and rejects invalid registrations",
		"[options][registry][add]")
	{
		registry options;

		SECTION("ordinary names and names containing reserved substrings are valid")
		{
			REQUIRE_NOTHROW(options.add(
				"Hash",
				spin{ 16, 1, 1024 },
				[](const int64_t) {}));
			REQUIRE_NOTHROW(options.add(
				"filename",
				string{ "" },
				[](const std::string&) {}));
			REQUIRE_NOTHROW(options.add(
				"evaluation",
				[]() {}));
		}

		SECTION("reserved words used as complete tokens are invalid")
		{
			REQUIRE_THROWS_AS(
				options.add(
					"file name",
					string{ "" },
					[](const std::string&) {}),
				registry::option_error);
			REQUIRE_THROWS_AS(
				options.add("current value", []() {}),
				registry::option_error);
		}

		SECTION("an existing name cannot be registered a second time")
		{
			options.add("Clear Hash", []() {});

			REQUIRE_THROWS_AS(
				options.add(
					"Clear Hash",
					checkbox{ false },
					[](const bool) {}),
				registry::option_error);
		}
	}

	TEST_CASE("registry prints its registered options",
		"[options][registry][print]")
	{
		registry options;
		std::ostringstream output;

		SECTION("an empty registry produces no output")
		{
			options.print(output);

			REQUIRE(output.str().empty());
		}

		SECTION("every registered option is printed once")
		{
			options.add("Ponder", checkbox{ false }, [](const bool) {});
			options.add("Clear Hash", []() {});

			options.print(output);
			const auto printed = output.str();

			REQUIRE(printed.find(
				"option name Ponder type check default false\n")
				!= std::string::npos);
			REQUIRE(printed.find(
				"option name Clear Hash type button\n")
				!= std::string::npos);
			REQUIRE(std::ranges::count(printed, '\n') == 2);
		}
	}

	TEST_CASE("registry applies checkbox values case-insensitively",
		"[options][registry][apply][checkbox]")
	{
		registry options;
		std::optional<bool> received;
		options.add(
			"Ponder",
			checkbox{ false },
			[&](const bool value) { received = value; });

		SECTION("true values")
		{
			const auto value = GENERATE(
				std::string{ "true" },
				std::string{ "TRUE" },
				std::string{ "TrUe" });

			options.apply("Ponder", value);

			REQUIRE(received == true);
		}

		SECTION("false values")
		{
			const auto value = GENERATE(
				std::string{ "false" },
				std::string{ "FALSE" },
				std::string{ "FaLsE" });

			options.apply("Ponder", value);

			REQUIRE(received == false);
		}
	}

	TEST_CASE("registry rejects missing and malformed checkbox values",
		"[options][registry][apply][checkbox]")
	{
		registry options;
		auto applications = 0;
		options.add(
			"Ponder",
			checkbox{ false },
			[&](const bool) { ++applications; });

		SECTION("missing value")
		{
			REQUIRE_THROWS_AS(
				options.apply("Ponder", std::nullopt),
				registry::option_error);
		}

		SECTION("value outside the boolean lexical domain")
		{
			const auto value = GENERATE(
				std::string{},
				std::string{ "yes" },
				std::string{ "true-ish" });

			REQUIRE_THROWS_AS(
				options.apply("Ponder", value),
				registry::option_error);
		}

		REQUIRE(applications == 0);
	}

	TEST_CASE("registry applies spin values across the inclusive range",
		"[options][registry][apply][spin]")
	{
		registry options;
		std::optional<int64_t> received;
		options.add(
			"Skill Level",
			spin{ 10, 0, 20 },
			[&](const int64_t value) { received = value; });

		const auto value = GENERATE(
			int64_t{ 0 },
			int64_t{ 10 },
			int64_t{ 20 });

		options.apply("Skill Level", std::to_string(value));

		REQUIRE(received == value);
	}

	TEST_CASE("registry rejects invalid spin values",
		"[options][registry][apply][spin]")
	{
		registry options;
		auto applications = 0;
		options.add(
			"Skill Level",
			spin{ 10, 0, 20 },
			[&](const int64_t) { ++applications; });

		SECTION("missing value")
		{
			REQUIRE_THROWS_AS(
				options.apply("Skill Level", std::nullopt),
				registry::option_error);
		}

		SECTION("value below the range")
		{
			REQUIRE_THROWS_AS(
				options.apply("Skill Level", std::string{ "-1" }),
				registry::option_error);
		}

		SECTION("value above the range")
		{
			REQUIRE_THROWS_AS(
				options.apply("Skill Level", std::string{ "21" }),
				registry::option_error);
		}

		SECTION("non-numeric value")
		{
			const auto value = GENERATE(
				std::string{},
				std::string{ "ten" },
				std::string{ "10x" });

			REQUIRE_THROWS_AS(
				options.apply("Skill Level", value),
				registry::option_error);
		}

		REQUIRE(applications == 0);
	}

	TEST_CASE("registry applies only listed combo values",
		"[options][registry][apply][combo]")
	{
		registry options;
		std::optional<std::string> received;
		options.add(
			"Style",
			combo{ "Normal", { "Solid", "Normal", "Risky" } },
			[&](const std::string& value) { received = value; });

		SECTION("listed value")
		{
			const auto value = GENERATE(
				std::string{ "Solid" },
				std::string{ "Normal" },
				std::string{ "Risky" });

			options.apply("Style", value);

			REQUIRE(received == value);
		}

		SECTION("unlisted value")
		{
			REQUIRE_THROWS_AS(
				options.apply("Style", std::string{ "Unknown" }),
				registry::option_error);
			REQUIRE_FALSE(received.has_value());
		}

		SECTION("different casing is not an exact listed value")
		{
			REQUIRE_THROWS_AS(
				options.apply("Style", std::string{ "normal" }),
				registry::option_error);
			REQUIRE_FALSE(received.has_value());
		}

		SECTION("missing value")
		{
			REQUIRE_THROWS_AS(
				options.apply("Style", std::nullopt),
				registry::option_error);
			REQUIRE_FALSE(received.has_value());
		}
	}

	TEST_CASE("registry applies provided string values",
		"[options][registry][apply][string]")
	{
		registry options;
		std::optional<std::string> received;
		options.add(
			"SyzygyPath",
			string{ "" },
			[&](const std::string& value) { received = value; });

		SECTION("non-empty value")
		{
			options.apply("SyzygyPath", std::string{ "C:/tablebases" });

			REQUIRE(received == "C:/tablebases");
		}

		SECTION("an explicitly provided empty value")
		{
			options.apply("SyzygyPath", std::string{});

			REQUIRE(received == "");
		}

		SECTION("missing value")
		{
			REQUIRE_THROWS_AS(
				options.apply("SyzygyPath", std::nullopt),
				registry::option_error);
			REQUIRE_FALSE(received.has_value());
		}
	}

	TEST_CASE("registry applies buttons without a value",
		"[options][registry][apply][button]")
	{
		registry options;
		auto applications = 0;
		options.add("Clear Hash", [&]() { ++applications; });

		SECTION("missing value represents a button press")
		{
			options.apply("Clear Hash", std::nullopt);

			REQUIRE(applications == 1);
		}

		SECTION("a supplied value is invalid for a button")
		{
			REQUIRE_THROWS_AS(
				options.apply("Clear Hash", std::string{ "now" }),
				registry::option_error);
			REQUIRE(applications == 0);
		}
	}

	TEST_CASE("registry rejects applying an unknown option",
		"[options][registry][apply]")
	{
		registry options;
		options.add("Ponder", checkbox{ false }, [](const bool) {});

		REQUIRE_THROWS_AS(
			options.apply("ponder", std::string{ "true" }),
			registry::option_error);
		REQUIRE_THROWS_AS(
			options.apply("Unknown", std::nullopt),
			registry::option_error);
	}
}
