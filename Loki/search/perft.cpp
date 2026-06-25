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

#include "perft.hpp"
#include "movegen/move_list.hpp"
#include <chrono>

namespace loki::search
{
	using namespace position;
	using namespace movegen;

	size_t perft::run(const std::string& fen, size_t depth)
	{
		m_fen = fen;
		m_nodes = m_nps = 0;
		m_position = make(game_state::from_fen(fen), m_bishop_index, m_rook_index);

		m_os << "<PERFT TEST FOR DEPTH = " << depth << ">" << std::endl;
		m_os << "FEN: " << m_fen << std::endl;

		std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();
		move_list moves;
		auto move_count = m_position->generate_moves(&moves);
		size_t legal = 0;

		for (auto i = 0; i < move_count; i++)
		{
			if (!m_position->make_move(moves[i]))
				continue;
			legal++;
			size_t old_nodes = m_nodes;

			run_internal(depth - 1);
			m_position->undo_last_move();

			m_os << std::format("[{}] {}\t---> {} nodes.\n", legal, moves[i].to_string(), m_nodes - old_nodes);
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(start_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(end_time).time_since_epoch().count();
		auto elapsed = end - start;
		m_nps = static_cast<int64_t>(double(m_nodes) / (double(elapsed <= 0 ? 1 : elapsed) / 1000.0));


		m_os << "\nPerft test completed after: " << elapsed << "ms.\n";
		m_os << "Nodes/second: " << m_nps << "\n";
		m_os << "\nNodes visited: " << m_nodes << "\n";
		m_os << "</PERFT TEST FOR DEPTH = " << depth << ">" << std::endl;
		return m_nodes;
	}

	void perft::run_internal(size_t depth)
	{
		if (depth <= 0)
		{
			m_nodes++;
			return;
		}

		move_list moves;
		m_position->generate_moves(&moves);

		for (const auto& move : moves)
		{
			if (!m_position->make_move(move))
				continue;
			run_internal(depth - 1);
			m_position->undo_last_move();
		}
	}

}