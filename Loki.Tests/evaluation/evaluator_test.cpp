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
#include "Loki/evaluation/evaluator.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"
#include "Loki/position/search_position.hpp"
#include "position/io/fen_reader.hpp"

namespace evaluation_tests
{
	using namespace loki;
	using namespace loki::evaluation;
	using namespace loki::movegen::magics;
	using namespace loki::position;

	score_t eval_runtime(const evaluator<search_position::position_proxy>& evaluator, const search_position::position_proxy& position)
	{
		return position.game_state()->side_to_move == WHITE
			? evaluator.evaluate<WHITE>(position)
			: evaluator.evaluate<BLACK>(position);
	}

	TEST_CASE_METHOD(position_tests::io_tests::fen_reader,
		"evaluator is symmetric for flipped positions in the shared FEN corpus",
		"[evaluation][evaluator][fen]")
	{
		const auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		const auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
		const evaluator<search_position::position_proxy> evaluator;
		bool has_non_zero_evaluation = false;

		for (const auto& test_case : fens)
		{
			CAPTURE(test_case.fen);

			const bool breakhere = (test_case.fen == "rnbqkb1r/ppppp1pp/7n/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 88 44");

			const auto position = make(
				game_state::from_fen(test_case.fen), bishop_index, rook_index);
			const auto flipped_position = make(
				game_state::from_fen(game_state::flip_fen(test_case.fen)), bishop_index, rook_index);
			const auto position_view = position->make_view();
			const auto flipped_position_view = flipped_position->make_view();

			const auto score = eval_runtime(evaluator, *position_view);
			const auto flipped_score = eval_runtime(evaluator, *flipped_position_view);

			REQUIRE(score == flipped_score);
			has_non_zero_evaluation |= score != 0 || flipped_score != 0;
		}

		REQUIRE(has_non_zero_evaluation);
	}
}
