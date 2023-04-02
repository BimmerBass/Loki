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
#include "loki.pch.hpp"

namespace loki::search
{
	search_stats::search_stats() : m_plyStats{}, info{ZeroInfo}
	{
		pvTable = std::make_unique<util::tri_pv_table<>>();
	}

	/// <summary>
	/// Update the heuristics for quiet moves based on the best move from a search at a given depth.
	/// </summary>
	void search_stats::update_quiet_heuristics(move_t bestMove, eDepth ply)
	{
		// Currently, the only heuristic we use is the killer moves.
		update_killers(bestMove, ply);
	}

	/// <summary>
	/// Clear all data in the structure.
	/// </summary>
	void search_stats::clear()
	{
		m_plyStats.fill(ZeroStats);
		pvTable->clear();
		info = ZeroInfo;
	}

	/// <summary>
	/// Add the best move as the highest-priority killer and move the previous one into the other "bucket".
	/// Note: To avoid duplicate killers, don't perform the shift if the best move is already the best killer.
	/// </summary>
	void search_stats::update_killers(move_t newBest, eDepth ply)
	{
		if (newBest != m_plyStats[ply].killers.first)
		{
			// Firstly, move the previous best move back, then set the new
			m_plyStats[ply].killers.second = m_plyStats[ply].killers.first;
			m_plyStats[ply].killers.first = newBest;
		}
	}
}