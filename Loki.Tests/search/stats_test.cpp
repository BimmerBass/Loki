// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

#include "pch.hpp"
#include "Loki/search/info_sink.hpp"
#include "Loki/search/stats.hpp"

namespace search_tests
{
	using namespace loki;
	using namespace loki::movegen;
	using namespace loki::position;
	using namespace loki::search;

	TEST_CASE("search_statistics starts cleared", "[search][stats]")
	{
		search_statistics statistics;

		REQUIRE(statistics.nodes == 0);
		REQUIRE(statistics.selective_depth == 0);
		REQUIRE(statistics.pv_table.get_pv(ROOT_PLY).empty());
	}

	TEST_CASE("search_statistics clear resets all state and permits reuse", "[search][stats]")
	{
		search_statistics statistics;
		statistics.nodes = 42;
		statistics.selective_depth = 3;
		statistics.pv_table.update_pv(static_cast<ply_t>(1), move{ E7, E5, false });
		statistics.pv_table.update_pv(ROOT_PLY, move{ E2, E4, false });

		statistics.clear();

		REQUIRE(statistics.nodes == 0);
		REQUIRE(statistics.selective_depth == 0);
		REQUIRE(statistics.pv_table.get_pv(ROOT_PLY).empty());
		REQUIRE(statistics.pv_table.get_pv(static_cast<ply_t>(1)).empty());

		statistics.nodes = 1;
		statistics.pv_table.update_pv(ROOT_PLY, move{ D2, D4, false });
		REQUIRE(statistics.nodes == 1);
		REQUIRE(statistics.pv_table.get_pv(ROOT_PLY) == std::vector<move>{ move{ D2, D4 , false} });
	}

	TEST_CASE("null_sink accepts search reports", "[search][info_sink]")
	{
		null_sink sink;
		const std::vector<move> pv{ move{ E2, E4 , false}, move{ E7, E5 , false} };

		REQUIRE_NOTHROW(sink.info(
			1,
			cp_score{ 0 },
			1,
			std::chrono::milliseconds{ 0 },
			1,
			1,
			pv));
		REQUIRE_NOTHROW(sink.bestmove(pv[0], pv[1]));
	}
}
