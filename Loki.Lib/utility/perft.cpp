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

namespace loki::utility
{


	/// <summary>
	/// Set the FEN and create a new position object.
	/// </summary>
	/// <param name="fen"></param>
	perft::perft(const std::string& fen) : m_initial_fen(fen), m_nodes(0)
	{
		m_pos = position::position::create_position(std::make_shared<position::game_state>(), std::make_shared<movegen::magics::slider_generator>());
	}

	/// <summary>
	/// Set up the position with the FEN and perform a perft on a given depth.
	/// </summary>
	/// <param name="d"></param>
	/// <returns></returns>
	size_t perft::perform(eDepth d, std::ostream& os)
	{
		*m_pos << m_initial_fen;
		m_nodes = 0;
		m_nps = 0;

		return perft_test(d, os);
	}

	/// <summary>
	/// Set a new fen.
	/// </summary>
	/// <param name="new_fen"></param>
	void perft::load(const std::string& new_fen)
	{
		m_initial_fen = new_fen;
	}

	size_t perft::perft_test(eDepth d, std::ostream& os)
	{
		os << "Starting perft test for depth " << static_cast<size_t>(d) << ".\n";
		os << "[FEN]: " << m_initial_fen << "\n";
		size_t legal = 0;

		std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();
		
		ordering::move_sorter sorter(m_pos, false, false);
		move_t move;
		while (move = sorter.get_next())
		{
			if (!m_pos->make_move(move))
				continue;
			legal++;
			size_t old_nodes = m_nodes;

			perft_internal(d - 1);
			m_pos->undo_move();

			os << std::format("[{}] {}\t---> {} nodes.\n", legal, movegen::to_string(move), std::to_string(m_nodes - old_nodes));
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(start_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(end_time).time_since_epoch().count();
		auto elapsed = end - start;
		m_nps = static_cast<int64_t>(double(m_nodes) / (double(elapsed <= 0 ? 1 : elapsed) / 1000.0));

		os << "\nPerft test completed after: " << (end - start) << "ms.\n";
		os << "Nodes/second: " << m_nps << "\n";
		os << "\nNodes visited: " << m_nodes << "\n";

		return m_nodes;
	}

	/// <summary>
	/// The perft function itself.
	/// </summary>
	/// <param name="d"></param>
	/// <returns></returns>
	void perft::perft_internal(eDepth d)
	{

		if (d <= 0)
		{
			m_nodes++;
			return;
		}
		size_t nodes = 0;

		ordering::move_sorter sorter(m_pos, false, false);
		move_t move;
		while (move = sorter.get_next())
		{
			if (!m_pos->make_move(move))
				continue;
			perft_internal(d - 1);

			m_pos->undo_move();
		}
	}

}