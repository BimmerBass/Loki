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
#include "Loki/loki_engine.hpp"

namespace loki_tests
{
	using namespace loki;

	TEST_CASE("loki_engine starts uninitialized and can accept a position", "[engine][loki_engine]")
	{
		loki_engine engine;
		REQUIRE_THROWS_AS(engine.state(), loki_exception);

		auto state = position::game_state::from_fen(constants::START_FEN);
		REQUIRE(engine.set_position(*state, std::vector<loki::movegen::move>{}));
		REQUIRE(position::game_state::to_fen(std::make_shared<position::game_state>(engine.state())) == constants::START_FEN);
	}

	TEST_CASE("loki_engine applies a short opening move sequence", "[engine][loki_engine]")
	{
		loki_engine engine;
		auto state = position::game_state::from_fen(constants::START_FEN);
		const std::vector<loki::movegen::move> moves{
			{position::E2, position::E4},
			{position::E7, position::E5},
		};

		REQUIRE(engine.set_position(*state, moves));
		const auto result = position::game_state::to_fen(std::make_shared<position::game_state>(engine.state()));
		REQUIRE(result == "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
	}
}
