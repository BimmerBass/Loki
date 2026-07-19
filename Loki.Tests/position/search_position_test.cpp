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

	namespace
	{
		search_position_t make_position(const std::string& fen)
		{
			auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
			auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
			return make(game_state::from_fen(fen), bishop_index, rook_index);
		}

		void require_activity(const move_list& moves, const std::string& move_string, bool expected)
		{
			const auto generated_move = moves.find(move_string);
			REQUIRE(generated_move.has_value());
			REQUIRE(generated_move->is_active() == expected);
		}

		bool contains_move_string(const move_list& moves, const std::string& move_string)
		{
			return std::ranges::any_of(moves, [&move_string](const move& generated_move)
				{
					return generated_move.to_string() == move_string;
				});
		}

		void generate_and_require_activity_partitions(search_position& position, move_list& all_moves)
		{
			move_list active_moves;
			move_list quiet_moves;

			const auto active_count = position.generate_moves<ACTIVE>(&active_moves);
			const auto quiet_count = position.generate_moves<QUIET>(&quiet_moves);
			const auto all_count = position.generate_moves<ALL>(&all_moves);

			REQUIRE(active_count == active_moves.size());
			REQUIRE(quiet_count == quiet_moves.size());
			REQUIRE(all_count == all_moves.size());
			REQUIRE(all_count == active_count + quiet_count);

			for (const auto& generated_move : active_moves)
				REQUIRE(generated_move.is_active());
			for (const auto& generated_move : quiet_moves)
				REQUIRE_FALSE(generated_move.is_active());

			for (const auto& generated_move : all_moves)
			{
				const auto move_string = generated_move.to_string();
				const bool appears_in_active = contains_move_string(active_moves, move_string);
				const bool appears_in_quiet = contains_move_string(quiet_moves, move_string);

				REQUIRE(appears_in_active != appears_in_quiet);
				REQUIRE(generated_move.is_active() == appears_in_active);
			}
		}
	}

	TEST_CASE("search_position can generate moves from the initial position", "[position][search_position]")
	{
		auto pos = make_position(constants::START_FEN);

		move_list moves;
		REQUIRE(pos->generate_moves(&moves) == 20);
		REQUIRE(std::ranges::none_of(moves, [](const move& generated_move)
			{
				return generated_move.is_active();
			}));
		REQUIRE_FALSE(pos->in_check());
	}

	TEST_CASE("search_position move-type templates partition active and quiet moves", "[position][search_position]")
	{
		auto pos = make_position("8/8/8/3p4/4P3/8/8/4K2k w - - 0 1");
		move_list all_moves;
		generate_and_require_activity_partitions(*pos, all_moves);

		require_activity(all_moves, "e4d5", true);
		require_activity(all_moves, "e4e5", false);
	}

	TEST_CASE("search_position assigns activity flags to generated move categories", "[position][search_position][movegen]")
	{
		SECTION("sliding piece captures and quiet moves")
		{
			auto pos = make_position("7k/8/3p4/8/3R4/8/8/K7 w - - 0 1");
			move_list moves;
			generate_and_require_activity_partitions(*pos, moves);

			require_activity(moves, "d4d6", true);
			require_activity(moves, "d4d5", false);
		}

		SECTION("king captures and quiet moves")
		{
			auto pos = make_position("7k/8/8/5p2/4K3/8/8/8 w - - 0 1");
			move_list moves;
			generate_and_require_activity_partitions(*pos, moves);

			require_activity(moves, "e4f5", true);
			require_activity(moves, "e4d4", false);
		}

		SECTION("en passant")
		{
			auto pos = make_position("7k/8/8/3pP3/8/8/8/7K w - d6 0 1");
			move_list moves;
			generate_and_require_activity_partitions(*pos, moves);

			require_activity(moves, "e5d6", true);
			require_activity(moves, "e5e6", false);
		}

		SECTION("capturing and non-capturing promotions")
		{
			auto pos = make_position("1r5k/P7/8/8/8/8/8/7K w - - 0 1");
			move_list moves;
			generate_and_require_activity_partitions(*pos, moves);

			for (const char promotion_piece : { 'n', 'b', 'r', 'q' })
			{
				require_activity(moves, std::string("a7a8") + promotion_piece, true);
				require_activity(moves, std::string("a7b8") + promotion_piece, true);
			}
		}

		SECTION("castling")
		{
			auto pos = make_position("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
			move_list moves;
			generate_and_require_activity_partitions(*pos, moves);

			require_activity(moves, "e1g1", false);
			require_activity(moves, "e1c1", false);
		}
	}

	TEST_CASE("search_position clone preserves move history independently of search ply", "[position][search_position]")
	{
		auto state = game_state::from_fen(constants::START_FEN);
		auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
		auto pos = make(state, bishop_index, rook_index);

		move_list moves;
		pos->generate_all_legals(&moves);
		const auto e2e4 = moves.find("e2e4");
		REQUIRE(e2e4.has_value());
		REQUIRE(pos->make_move(*e2e4));
		REQUIRE(pos->ply() == static_cast<ply_t>(1));

		auto clone = pos->clone();
		REQUIRE(clone->ply() == static_cast<ply_t>(1));
		clone->undo_last_move();
		REQUIRE(clone->ply() == ROOT_PLY);
		REQUIRE(clone->to_fen() == constants::START_FEN);

		REQUIRE(pos->ply() == static_cast<ply_t>(1));
		REQUIRE(pos->to_fen() != constants::START_FEN);
	}

	TEST_CASE("search_position caps ply at zero while undoing pre-root history", "[position][search_position]")
	{
		auto state = game_state::from_fen(constants::START_FEN);
		auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
		auto pos = make(state, bishop_index, rook_index);

		move_list moves;
		pos->generate_all_legals(&moves);
		const auto e2e4 = moves.find("e2e4");
		REQUIRE(e2e4.has_value());
		REQUIRE(pos->make_move(*e2e4));
		pos->ply() = ROOT_PLY;

		pos->undo_last_move();
		REQUIRE(pos->ply() == ROOT_PLY);
		REQUIRE(pos->to_fen() == constants::START_FEN);
	}

	TEST_CASE("search_position detects fifty-move draws", "[position][search_position][draw]")
	{
		auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		auto rook_index = std::make_shared<hardcoded_index<ROOK>>();

		auto drawn = make(
			game_state::from_fen("8/8/8/8/8/8/4K3/7k w - - 100 1"),
			bishop_index,
			rook_index);
		REQUIRE(drawn->is_draw());
		REQUIRE_FALSE(drawn->is_material_draw());
		REQUIRE_FALSE(drawn->is_repetition());

		auto not_drawn = make(
			game_state::from_fen("8/8/8/8/8/8/4K3/7k w - - 99 1"),
			bishop_index,
			rook_index);
		REQUIRE_FALSE(not_drawn->is_draw());
	}
}
