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
#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>
#include <expected>

#include "defs.hpp"
#include "position/search_position.hpp"
#include "limits.hpp"
#include "util/exception.hpp"
#include "evaluation/evaluator.hpp"
#include "info_sink.hpp"
#include "stats.hpp"

namespace loki::search
{
	using search_result_t = std::expected<score_t, std::exception_ptr>;

	/// <summary>
	/// search_worker is the class responsible for the actual alpha-beta search.
	/// </summary>
	class search_worker final
	{
		using evaluator_t = evaluation::evaluator<
			position::search_position::position_proxy>;
		CHILD_EXCEPTION(search_error, loki_exception);

	private:
		info_sink_t _info_sink;
		search_statistics statistics;
		const evaluator_t evaluator;
		
	public:
		search_worker() : evaluator(), _info_sink{ std::make_unique<null_sink>() }, statistics{}
		{}

		explicit search_worker(std::unique_ptr<info_sink> sink, const evaluation::i_weight_source& weight_source) :
			_info_sink{ std::move(sink) },
			evaluator(weight_source),
			statistics{}
		{}

		search_result_t search(
			std::unique_ptr<position::search_position> position,
			const limits& limits,
			std::stop_token stop_token) noexcept
		{
			try
			{
				if (!position)
					throw_msg<search_error>("null position");

				// Preprocess to clear any earlier search statistics.
				preprocess_search();
				
				auto max_depth = limits.depth.value_or(constants::MAX_DEPTH);
				score_t score = -(constants::SCORE_MATE - 1);
				movegen::move bestmove;

				for (depth_t depth = 1; depth <= max_depth; depth++)
				{
					score = position->side_to_move() == WHITE
						? search_ab<WHITE>(*position, limits, stop_token, depth, -constants::SCORE_INF, constants::SCORE_INF)
						: search_ab<BLACK>(*position, limits, stop_token, depth, -constants::SCORE_INF, constants::SCORE_INF);

					if (stop_token.stop_requested())
						break;


					auto time = limits.time_elapsed();
					auto seconds = static_cast<double>(time.count()) / 1000.0;
					auto nps = static_cast<size_t>(
						static_cast<double>(statistics.nodes) / seconds);
					_info_sink->info(
						depth,
						statistics.selective_depth,
						time,
						statistics.nodes,
						nps,
						statistics.pv_table.get_pv(ROOT_PLY));
				}

				_info_sink->bestmove(bestmove);

				return score;
			}
			catch (...)
			{
				return std::unexpected(std::current_exception());
			}
		}

	private:
		void preprocess_search()
		{
			statistics.clear();
		}
		
		template<side S> requires (S < NUM_SIDES)
		[[nodiscard]]
		score_t search_ab(
			position::search_position& position,
			const limits& limits,
			std::stop_token stop_token,
			size_t depth,
			score_t alpha,
			score_t beta)
		{
			assert(S == position.side_to_move());
			return 0;
		}
	};
}