#include "perft.hpp"
#include "movegen/move_list.hpp"
#include <chrono>

namespace loki::search
{
	using namespace position;
	using namespace movegen;

	void perft::run(const std::string& fen, size_t depth)
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
			//size_t old_nodes = m_nodes;

			run_internal(depth - 1);
			m_position->undo_last_move();

			//m_os << std::format("[{}] {}\t---> {} nodes.\n", legal, moves[i].from(), m_nodes - old_nodes);
		}
	}

	void perft::run_internal(size_t)
	{
		return;
	}

}