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

namespace loki::search
{
	/// <summary>
	/// search_stats stores all information collected throughout a search.
	/// </summary>
	class search_stats
	{
		struct _search_info
		{
			eDepth selective_depth;		/* The actual depth that the thread has searched due to reductions, pruning and extensions */
			uint64_t nodes;				/* The total amount of nodes, the thread has analyzed */
			uint64_t fail_high;			/* The amount of nodes where the thread has found a beta-cutoff */
			uint64_t fail_high_first;	/* The amount of nodes where the thread has found a beta-cutoff on the first tested move. fail_high_first / fail_high gives an indication on the quality of the move ordering */
		};
		inline static constexpr _search_info ZeroInfo{ static_cast<eDepth>(0), 0, 0, 0 };

		struct _depthBasedStats
		{
			std::pair<move_t, move_t> killers;
		};
		inline static constexpr _depthBasedStats ZeroStats{{MOVE_NULL, MOVE_NULL}};
		
		// Indexed by side-to-move, from-square, to-square
		using history_table_t = utility::NDimensionalArray<eValue, SIDE_NB, SQ_NB, SQ_NB>;
	private:
		std::array<_depthBasedStats, MAX_DEPTH>		m_plyStats;
		history_table_t								m_historyTable;
	public:
		util::tri_pv_table_t						pvTable;
		_search_info								info;
	public:
		search_stats();

		inline _depthBasedStats* ply_stats(eDepth ply)	{ return &m_plyStats[ply]; }
		inline eValue history_score(eSide side, eSquare fromSq, eSquare toSq) const { return m_historyTable[side][fromSq][toSq]; }
		void update_quiet_heuristics(const position::position_t& pos, move_t bestMove, eDepth ply);
		void clear();
	private:
		void update_killers(move_t newBest, eDepth ply);
	};
}