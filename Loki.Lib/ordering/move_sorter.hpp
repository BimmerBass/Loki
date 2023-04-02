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
#pragma once
using namespace loki::movegen;

namespace loki::ordering
{
	/// <summary>
	/// move_sorter is responsible for calling the generate method of a search's position object, scoring the moves, and returning them to the search function
	/// </summary>
	class move_sorter
	{
		EXCEPTION_CLASS(e_moveSorter, e_lokiError);
		using stats_t = search::search_stats;

		inline static constexpr eValue CaptureScale = (eValue)12000;
		inline static constexpr eValue KillerOneScale = (eValue)11000;
		inline static constexpr eValue KillerTwoScale = (eValue)10000;
	private:
		const position::position_t&		m_pos;
		const std::shared_ptr<stats_t>& m_stats;
		move_list_t*					m_moveList;
		const bool						m_is_quiescence;
		const bool						m_perform_scoring;

		size_t							m_currentInx;
	public:
		move_sorter(
			const position::position_t&		pos,
			const std::shared_ptr<stats_t>&	stats,
			bool							isQuiescence = false,
			bool							performScoring = true);

		/// <summary>
		/// Generate the moves and score them.
		/// TODO: Perhaps the template argument should be moved up to cover the entire class?
		/// </summary>
		template<eMoveType _T = ALL>
		void generate();

		/// <summary>
		/// Fetch the next move. If score is not nullptr, assign the score of the move.
		/// </summary>
		/// <returns>The next highest scored move.</returns>
		move_t get_next(eValue* score = nullptr);
	private:
		void bringBestMoveFront();
		void scoreMoves();
	};
}