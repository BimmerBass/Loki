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

#pragma once
#include "terms.hpp"
#include "trace.hpp"
#include "position/position_view.hpp"
#include "weights/builtin_weight_source.hpp"
#include <array>

namespace loki::evaluation
{
	template<position::position_view pos_t>
	class evaluator final
	{
	private:
		const weights _features;
	public:
		evaluator()
			: _features{ builtin_weight_source::defaults() }
		{}

		explicit evaluator(const i_weight_source& src)
			: _features{ src.load() }
		{}

		template<side S> requires (S != NUM_SIDES)
		[[nodiscard]] inline score_t evaluate(const pos_t& position) const noexcept
		{
			null_trace t;
			return evaluate<S>(t, position);
		}

		template<side S> requires (S != NUM_SIDES)
		[[nodiscard]] inline feature_trace evaluate_trace(const pos_t& position) const noexcept
		{
			feature_trace trace;
			trace.relative_to = S;
			trace.score = evaluate<S>(trace, position);
			return trace;
		}
		
		constexpr score_t to_centipawns(score_t raw) const noexcept
		{
			using layout_t = term_layout<evaluation_term::MATERIAL>;
			const auto pawn_value = _features[layout_t::id<PAWN>()];

			return static_cast<score_t>(raw * 100 / pawn_value);
		}

	private:

		template<side S, feature_sink sink_t>
		inline score_t evaluate(sink_t& sink, const pos_t& position) const noexcept
		{
			auto score =
				material<S>(sink, position) - material<!S>(sink, position)
				+ psqt<S>(sink, position) - psqt<!S>(sink, position);
			return score;
		}

		template<side S, feature_sink sink_t>
		inline score_t add_feature(sink_t& sink, id_t id, size_t count) const noexcept
		{
			if constexpr (sink_t::enabled)
			{
				sink.add_feature<S>(id, count);
			}
			return _features[id] * count;
		}

		template<side S, feature_sink sink_t>
		inline score_t material(sink_t& sink, const pos_t& position) const noexcept
		{
			using namespace loki::position;
			using layout_t = term_layout<evaluation_term::MATERIAL>;

			score_t material =
				add_feature<S>(sink, layout_t::id<PAWN>(), popcount(position.piece_bb(S, PAWN)))
				+ add_feature<S>(sink, layout_t::id<KNIGHT>(), popcount(position.piece_bb(S, KNIGHT)))
				+ add_feature<S>(sink, layout_t::id<BISHOP>(), popcount(position.piece_bb(S, BISHOP)))
				+ add_feature<S>(sink, layout_t::id<ROOK>(), popcount(position.piece_bb(S, ROOK)))
				+ add_feature<S>(sink, layout_t::id<QUEEN>(), popcount(position.piece_bb(S, QUEEN)));

			return material;
		}

		template<side S, feature_sink sink_t>
		inline score_t psqt(sink_t& sink, const pos_t& position) const noexcept
		{
			const auto psqt_score =
				psqt_piece<S, PAWN>(sink, position)
				+ psqt_piece<S, KNIGHT>(sink, position)
				+ psqt_piece<S, BISHOP>(sink, position)
				+ psqt_piece<S, ROOK>(sink, position)
				+ psqt_piece<S, QUEEN>(sink, position)
				+ psqt_piece<S, KING>(sink, position);
			return psqt_score;
		}

		template<side S, piece P, feature_sink sink_t>
		inline score_t psqt_piece(sink_t& sink, const pos_t& position) const noexcept
		{
			using namespace loki::position;
			using layout_t = term_layout<evaluation_term::PSQT>;

			score_t score = 0;
			auto bb = position.piece_bb(S, P);

			while (bb)
			{
				const auto inx = pop_lsb_nonzero(bb);
				score += add_feature<S>(
					sink,
					layout_t::id<S, P>(static_cast<e_square>(inx)),
					1);
			}
			return score;
		}
	};
}