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
	using side_feature_counts = std::array<std::array<size_t, feature_count()>, NUM_SIDES>;

	template<piece P> requires (P < KING)
	void count_material_feature(
		side_feature_counts& counts,
		const search_position::position_proxy& position)
	{
		using layout_t = term_layout<evaluation_term::MATERIAL>;
		constexpr auto id = layout_t::id<P>();
		counts[WHITE][id] = popcount(position.piece_bb(WHITE, P));
		counts[BLACK][id] = popcount(position.piece_bb(BLACK, P));
	}

	void count_material_features(
		side_feature_counts& counts,
		const search_position::position_proxy& position)
	{
		count_material_feature<PAWN>(counts, position);
		count_material_feature<KNIGHT>(counts, position);
		count_material_feature<BISHOP>(counts, position);
		count_material_feature<ROOK>(counts, position);
		count_material_feature<QUEEN>(counts, position);
	}

	side_feature_counts expected_trace_counts(const search_position::position_proxy& position)
	{
		side_feature_counts counts{};
		count_material_features(counts, position);
		return counts;
	}

	feature_trace evaluate_trace_for_side_to_move(
		const evaluator<search_position::position_proxy>& evaluator,
		const search_position::position_proxy& position)
	{
		return position.game_state()->side_to_move == WHITE
			? evaluator.evaluate_trace<WHITE>(position)
			: evaluator.evaluate_trace<BLACK>(position);
	}

	score_t evaluate_for_side_to_move(
		const evaluator<search_position::position_proxy>& evaluator,
		const search_position::position_proxy& position)
	{
		return position.game_state()->side_to_move == WHITE
			? evaluator.evaluate<WHITE>(position)
			: evaluator.evaluate<BLACK>(position);
	}

	template<piece P> requires (P < KING)
	void require_material_feature_counts(
		const feature_trace& trace,
		const side_feature_counts& expected)
	{
		using layout_t = term_layout<evaluation_term::MATERIAL>;
		constexpr auto id = layout_t::id<P>();

		CAPTURE(id, P);
		REQUIRE(trace.features[id].id == id);
		REQUIRE(trace.features[id].count<WHITE>() == expected[WHITE][id]);
		REQUIRE(trace.features[id].count<BLACK>() == expected[BLACK][id]);
	}

	void require_material_counts(const feature_trace& trace, const side_feature_counts& expected)
	{
		require_material_feature_counts<PAWN>(trace, expected);
		require_material_feature_counts<KNIGHT>(trace, expected);
		require_material_feature_counts<BISHOP>(trace, expected);
		require_material_feature_counts<ROOK>(trace, expected);
		require_material_feature_counts<QUEEN>(trace, expected);
	}

	score_t expected_feature_dot_product(const side_feature_counts& counts, side relative_to)
	{
		constexpr auto feature_weights = builtin_weight_source::defaults();
		const auto us = static_cast<size_t>(relative_to);
		const auto them = static_cast<size_t>(!relative_to);
		score_t score = 0;

		for (id_t id = 0; id < feature_count(); ++id)
		{
			const auto relative_count = static_cast<score_t>(counts[us][id])
				- static_cast<score_t>(counts[them][id]);
			score += feature_weights[id] * relative_count;
		}
		return score;
	}

	void require_trace_contract(
		const evaluator<search_position::position_proxy>& evaluator,
		const search_position::position_proxy& position,
		const side_feature_counts& expected)
	{
		const auto trace = evaluate_trace_for_side_to_move(evaluator, position);

		REQUIRE(trace.relative_to == position.game_state()->side_to_move);
		REQUIRE(trace.features.size() == feature_count());
		REQUIRE(trace.score == evaluate_for_side_to_move(evaluator, position));
		REQUIRE(trace.score == expected_feature_dot_product(expected, trace.relative_to));

		require_material_counts(trace, expected);
	}

	TEST_CASE_METHOD(position_tests::io_tests::fen_reader,
		"evaluation trace matches the ordinary evaluation across the shared FEN corpus",
		"[evaluation][evaluator][trace][fen]")
	{
		const auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		const auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
		const evaluator<search_position::position_proxy> evaluator;

		for (const auto& test_case : fens)
		{
			const std::array positions_to_test{
				test_case.fen,
				game_state::flip_fen(test_case.fen)
			};

			for (const auto& fen : positions_to_test)
			{
				CAPTURE(test_case.fen, fen);
				const auto position = make(
					game_state::from_fen(fen), bishop_index, rook_index);
				const auto position_view = position->make_view();
				const auto expected = expected_trace_counts(position_view);

				require_trace_contract(evaluator, position_view, expected);
			}
		}
	}
}
