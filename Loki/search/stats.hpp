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
#include "defs.hpp"
#include "pv_table.hpp"
#include "util/arrayops.hpp"
#include "ordering/move_scores.hpp"
#include <algorithm>

namespace loki::search
{
	/// <summary>
	/// search_statistics holds the statistics gathered during search that will be used for UCI outputting.
	/// </summary>
	struct search_statistics
	{
	private:
		static constexpr auto num_squares = position::NUM_SQUARES;
		using history_table_t = util::nested_array_t<movegen::move_score_t, NUM_SIDES, num_squares, num_squares>;
	public:
		pv_table<> pv_table;
		size_t selective_depth = 0;
		size_t nodes = 0;
		size_t fail_high = 0;
		size_t fail_high_first_move = 0;

		// Move ordering heuristics
		std::array<
			std::tuple<movegen::move_t, movegen::move_t>, MAX_PLY
		> killer_moves;

		void clear(bool full = false) noexcept
		{
			pv_table.clear();
			selective_depth = 0;
			nodes = 0;
			fail_high = 0;
			fail_high_first_move = 0;

			killer_moves.fill(std::make_tuple(movegen::MOVE_NULL, movegen::MOVE_NULL));
			if (full)
			{
				util::fill_all(history_table, (movegen::move_score_t)0);
			}
		}

		template<side S>
		inline void update_history(const movegen::move& move, int32_t bonus) noexcept
		{
			using namespace ordering;
			
			assert(!move.is_active());

			constexpr auto max = static_cast<int32_t>(HISTORY_MAX);
			auto clamped = std::clamp<int32_t>(bonus, -max, max);

			auto history_value = static_cast<int32_t>(history_table[S][move.from()][move.to()]);
			auto updated = history_value + clamped - history_value * std::abs(clamped) / max;
			history_table[S][move.from()][move.to()] = static_cast<int16_t>(updated);
		}

		inline movegen::move_score_t history_score(side stm, const movegen::move& move) const noexcept
		{
			return history_table[stm][move.from()][move.to()];
		}
	private:
		history_table_t history_table{};
	};
}
