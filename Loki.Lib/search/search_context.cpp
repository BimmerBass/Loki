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
	search_context::search_context() : m_movesToMake{}, m_state{}, m_legal_moves{}
	{
		m_slider_generator = std::make_shared<movegen::magics::slider_generator>();
		m_eval_parameters = evaluation::make_params<evaluation::hardcoded_params>();
	}

	void search_context::reset()
	{
		m_movesToMake.clear();
		m_state << START_FEN;
		generate_legals();
	}

	void search_context::set_position(const std::string& fen, const std::vector<std::string>& moves)
	{
		m_movesToMake = moves;
		m_state << fen;

		// Generate a new set of legal moves.
		generate_legals();
	}

	void search_context::generate_legals()
	{
		m_legal_moves.clear();
		auto pos = position::position::create_position(
			std::make_shared<position::game_state>(m_state),
			m_slider_generator);
		auto moves = pos->generate_moves();

		for (auto& move : moves)
		{
			if (pos->make_move(move.move))
			{
				m_legal_moves.add(move.move, move.score);
				pos->undo_move();
			}
		}
	}

	void search_context::do_perft(eDepth d)
	{
		std::string fen;
		m_state >> fen;
		utility::perft p(fen);

		p.perform(d, std::cout);
	}
}