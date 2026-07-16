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
#include "Loki/uci/uci_sink.hpp"

namespace uci_tests
{
	using namespace loki;
	using namespace loki::uci;

	TEST_CASE("uci sink writes centipawn search information through its context", "[uci][sink]")
	{
		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{ engine, UCI_STATE::Searching, input, output, error };
		uci_sink sink{ ctx };

		const std::vector pv{
			movegen::move{ position::E2, position::E4 },
			movegen::move{ position::E7, position::E5 }
		};
		sink.info(8, cp_score{ 42 }, 12, std::chrono::milliseconds{ 345 }, 6789, 19678, pv);

		REQUIRE(output.str() ==
			"info depth 8 score cp seldepth 12 time 345 nodes 6789 nps 19678 pv e2e4 e7e5\n");
	}

	TEST_CASE("uci sink writes mate search information through its context", "[uci][sink]")
	{
		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{ engine, UCI_STATE::Searching, input, output, error };
		uci_sink sink{ ctx };

		sink.info(8, mate_score{ constants::SCORE_INF - 3 }, 12,
			std::chrono::milliseconds{ 345 }, 6789, 19678, {});

		REQUIRE(output.str() ==
			"info depth 8 score mate seldepth 12 time 345 nodes 6789 nps 19678\n");
	}

	TEST_CASE("uci sink writes best and null moves through its context", "[uci][sink]")
	{
		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{ engine, UCI_STATE::Searching, input, output, error };
		uci_sink sink{ ctx };

		sink.bestmove(movegen::move{ position::G1, position::F3 });
		sink.bestmove(movegen::move{ movegen::MOVE_NULL });

		REQUIRE(output.str() == "bestmove g1f3\nbestmove 0000\n");
	}
}
