// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

#include "pch.hpp"
#include "Loki/search/limits.hpp"

namespace search_tests
{
	using namespace std::chrono_literals;
	using namespace loki::search;

	namespace
	{
		const limits::timepoint_t START = limits::timepoint_t{} + 1s;

		limits timed_limits()
		{
			limits result{};
			result.start_time = START;
			return result;
		}
	}

	TEST_CASE("limits reports elapsed time from a supplied clock value", "[search][limits]")
	{
		auto search_limits = timed_limits();

		REQUIRE(search_limits.time_elapsed(START + 275ms) == 275ms);
	}

	TEST_CASE("limits applies node limits at their exact boundary", "[search][limits]")
	{
		auto search_limits = timed_limits();
		search_limits.nodes = 10;

		REQUIRE_FALSE(search_limits.should_stop(9, START));
		REQUIRE(search_limits.should_stop(10, START));
		REQUIRE(search_limits.should_stop(11, START));
	}

	TEST_CASE("limits gives movetime precedence over clock allocation", "[search][limits]")
	{
		auto search_limits = timed_limits();
		search_limits.movetime = 100;
		search_limits.time = 1;
		search_limits.inc = 5'000;
		search_limits.movestogo = 1;

		REQUIRE_FALSE(search_limits.should_stop(0, START + 99ms));
		REQUIRE(search_limits.should_stop(0, START + 100ms));
	}

	TEST_CASE("limits allocates clock time using moves-to-go and increment", "[search][limits]")
	{
		auto search_limits = timed_limits();

		SECTION("uses the default moves-to-go estimate")
		{
			search_limits.time = 4'500;
			REQUIRE_FALSE(search_limits.should_stop(0, START + 99ms));
			REQUIRE(search_limits.should_stop(0, START + 100ms));
		}

		SECTION("uses an explicit moves-to-go value")
		{
			search_limits.time = 1'000;
			search_limits.inc = 50;
			search_limits.movestogo = 10;
			REQUIRE_FALSE(search_limits.should_stop(0, START + 149ms));
			REQUIRE(search_limits.should_stop(0, START + 150ms));
		}

		SECTION("normalizes zero moves-to-go to one")
		{
			search_limits.time = 1'000;
			search_limits.movestogo = 0;
			REQUIRE_FALSE(search_limits.should_stop(0, START + 999ms));
			REQUIRE(search_limits.should_stop(0, START + 1s));
		}
	}

	TEST_CASE("limits checks time only every 1024 nodes", "[search][limits]")
	{
		auto search_limits = timed_limits();
		search_limits.movetime = 1;
		const auto expired = START + 10ms;

		REQUIRE(search_limits.should_stop(0, expired));
		REQUIRE_FALSE(search_limits.should_stop(1, expired));
		REQUIRE_FALSE(search_limits.should_stop(1023, expired));
		REQUIRE(search_limits.should_stop(1024, expired));
	}

	TEST_CASE("limits without time fields never expires by time", "[search][limits]")
	{
		auto search_limits = timed_limits();

		REQUIRE_FALSE(search_limits.should_stop(0, START + std::chrono::hours{ 24 }));
	}

	TEST_CASE("infinite and mate searches suppress time but retain node limits", "[search][limits]")
	{
		auto search_limits = timed_limits();
		search_limits.movetime = 1;
		search_limits.nodes = 10;
		const auto expired = START + 10ms;

		SECTION("infinite")
		{
			search_limits.infinite = true;
			REQUIRE_FALSE(search_limits.should_stop(0, expired));
			REQUIRE(search_limits.should_stop(10, expired));
		}

		SECTION("mate")
		{
			search_limits.mate = 2;
			REQUIRE_FALSE(search_limits.should_stop(0, expired));
			REQUIRE(search_limits.should_stop(10, expired));
		}
	}
}
