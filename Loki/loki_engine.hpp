#pragma once
//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#include "position/search_position.hpp"
#include "movegen/move.hpp"
#include "movegen/magics/magic_index.hpp"
#include "search/limits.hpp"
#include "search/search_thread.hpp"

#include <iostream>
#include <optional>
#include <vector>

namespace loki
{
	class i_loki_engine
	{
	public:
		virtual ~i_loki_engine() = default;

		virtual bool set_position(const position::game_state& state, const std::vector<movegen::move>& moves) = 0;
		virtual void set_position(const position::game_state& state) = 0;
		virtual void set_position(position::search_position_t state) = 0;
		virtual position::search_position_t make_position(const position::game_state& state) const = 0;
		virtual void clear() = 0;
		virtual void search(
			const search::limits limits,
			search::search_thread::callback_t finished_callback,
			search::info_sink_t sink) = 0;
		virtual void stop_search(bool wait = false) = 0;
		virtual size_t perft(size_t depth, std::ostream& out) const = 0;
		virtual const position::search_position_t& position() const = 0;
	};

	class loki_engine final : public i_loki_engine
	{
	public:
		loki_engine();

		/// <summary>
		/// Set a position a given initial state and a list of moves to be applied
		/// </summary>
		/// <param name="state"></param>
		/// <param name="moves"></param>
		/// <returns></returns>
		bool set_position(const position::game_state& state, const std::vector<movegen::move>& moves) override;

		/// <summary>
		/// Set the position given a complete initial state.
		/// </summary>
		/// <param name="state"></param>
		void set_position(const position::game_state& state) override;
		void set_position(position::search_position_t state) override;

		/// <summary>
		/// Make a new position pointer using the already-initialized rook and bishop index.
		/// </summary>
		/// <param name="state"></param>
		/// <returns></returns>
		position::search_position_t make_position(const position::game_state& state) const override;

		/// <summary>
		/// Clear engine-owned state for a new game.
		/// </summary>
		void clear() override;

		void search(
			const search::limits limits,
			search::search_thread::callback_t finished_callback,
			search::info_sink_t sink) override;

		/// <summary>
		/// Stop the current search. If no search is running, this method does nothing.
		/// </summary>
		void stop_search(bool wait = false) override
		{
			_main_thread.stop_search();
			if (wait)
				_main_thread.wait_for_finished_search();
		}

		[[maybe_unused]]
		size_t perft(size_t depth, std::ostream& out) const override;

		const position::search_position_t& position() const override
		{
			if (!_position.has_value())
				throw_msg<loki_exception>("position has not been initialized.");
			return _position.value();
		}

	private:
		const movegen::magics::magic_index_t rook_index;
		const movegen::magics::magic_index_t bishop_index;

		std::optional<position::search_position_t> _position;
		search::search_thread _main_thread;
	};

}
