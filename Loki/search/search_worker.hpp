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
#include <algorithm>
#include <variant>

#include "defs.hpp"
#include "position/search_position.hpp"
#include "limits.hpp"
#include "util/exception.hpp"
#include "evaluation/evaluator.hpp"
#include "info_sink.hpp"
#include "stats.hpp"

namespace loki::search
{
	/// <summary>
	/// search_worker is the class responsible for the actual alpha-beta search.
	/// </summary>
	class search_worker final
	{
		using evaluator_t = evaluation::evaluator<
			position::search_position::position_proxy>;
		CHILD_EXCEPTION(search_error, loki_exception);

		enum class node
		{
			ROOT,
			INTERNAL
		};
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
			std::stop_token stop_token,
			info_sink_t sink) noexcept
		{
			try
			{
				if (!position)
					throw_msg<search_error>("null position");
				if (!sink)
					throw_msg<search_error>("null info sink");
				_info_sink = std::move(sink);

				// Preprocess to clear any earlier search statistics.
				preprocess_search(*position);

				const auto max_depth = std::min(
					limits.depth.value_or(constants::MAX_DEPTH),
					constants::MAX_DEPTH);
				search_score_t last_it_score = cp_score{ -(constants::SCORE_MATE - 1) };
				std::vector<movegen::move> last_it_pv{};

				for (depth_t depth = 1; depth <= max_depth; depth++)
				{
					const auto score = position->side_to_move() == WHITE
						? search_ab<WHITE, node::ROOT>(*position, limits, stop_token, depth, -constants::SCORE_INF, constants::SCORE_INF)
						: search_ab<BLACK, node::ROOT>(*position, limits, stop_token, depth, -constants::SCORE_INF, constants::SCORE_INF);

					if (stop_token.stop_requested())
						break;

					if (is_mate(score))
						last_it_score = mate_score(score);
					else
						last_it_score = cp_score{ score };

					auto time = limits.time_elapsed();
					auto seconds = static_cast<double>(std::max(time.count(), 1LL)) / 1000.0;
					auto nps = static_cast<size_t>(
						static_cast<double>(statistics.nodes) / seconds);

					last_it_pv = statistics.pv_table.get_pv(ROOT_PLY);
					
					_info_sink->info(
						depth,
						last_it_score,
						statistics.selective_depth,
						time,
						statistics.nodes,
						nps,
						last_it_pv);

					if (limits.mate && std::holds_alternative<mate_score>(last_it_score)
						&& std::get<mate_score>(last_it_score).in_moves == *limits.mate)
					{
						break;
					}
				}

				if (last_it_pv.empty())
					throw_msg<search_error>("search returned no principal variation");
				else if (last_it_pv.size() >= 2)
					_info_sink->bestmove(last_it_pv[0], last_it_pv[1]);
				else
					_info_sink->bestmove(last_it_pv.front());

				return last_it_score;
			}
			catch (...)
			{
				return std::unexpected(std::current_exception());
			}
		}

	private:
		void preprocess_search(position::search_position& position)
		{
			statistics.clear();
			position.ply() = ROOT_PLY;
		}

		template<side S, node Node> requires (S < NUM_SIDES)
			[[nodiscard]]
		score_t search_ab(
			position::search_position& position,
			const limits& limits,
			std::stop_token stop_token,
			size_t depth,
			score_t alpha,
			score_t beta);
	};

	extern template score_t search_worker::search_ab<WHITE, search_worker::node::ROOT>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
	extern template score_t search_worker::search_ab<WHITE, search_worker::node::INTERNAL>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
	extern template score_t search_worker::search_ab<BLACK, search_worker::node::ROOT>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
	extern template score_t search_worker::search_ab<BLACK, search_worker::node::INTERNAL>(
		position::search_position&, const limits&,std::stop_token, size_t, score_t, score_t);
}
