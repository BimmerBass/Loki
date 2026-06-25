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
#include "Loki/position/search_position.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"

namespace position_tests
{
	using namespace loki;
	using namespace loki::position;
	using namespace loki::movegen;
	using namespace loki::movegen::magics;

	TEST_CASE("search_position can generate moves from the initial position", "[position][search_position]")
	{
		auto state = game_state::from_fen(constants::START_FEN);
		auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
		auto pos = make(state, bishop_index, rook_index);

		move_list moves;
		REQUIRE(pos->generate_moves(&moves) == 20);
		REQUIRE_FALSE(pos->in_check());
	}

	TEST_CASE("search_position move-type templates partition active and quiet moves", "[position][search_position][stub]")
	{
		auto state = game_state::from_fen("8/8/8/3p4/4P3/8/8/4K2k w - - 0 1");
		auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
		auto pos = make(state, bishop_index, rook_index);

		move_list active_moves;
		move_list quiet_moves;
		move_list all_moves;

		REQUIRE(pos->generate_moves<ACTIVE>(&active_moves) == active_moves.size());
		REQUIRE(pos->generate_moves<QUIET>(&quiet_moves) == quiet_moves.size());
		REQUIRE(pos->generate_moves<ALL>(&all_moves) == all_moves.size());

		REQUIRE(active_moves.size() == 1);
		REQUIRE(active_moves[0].to_string() == "e4d5");
		REQUIRE(std::ranges::none_of(quiet_moves, [](const move& m) { return m.to_string() == "e4d5"; }));
		REQUIRE(all_moves.size() == active_moves.size() + quiet_moves.size());

		const auto contains = [](const move_list& moves, const move& expected)
			{
				return std::ranges::any_of(moves, [&expected](const move& actual) { return actual == expected; });
			};

		for (const auto& move : active_moves)
			REQUIRE(contains(all_moves, move));
		for (const auto& move : quiet_moves)
			REQUIRE(contains(all_moves, move));

		FAIL("TODO: complete move-type template coverage after validating this mixed FEN.");
	}
}
