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
#include <iostream>

#include "loki_context.hpp"
#include "versioninfo.hpp"
#include "position/square.hpp"
#include "defs.hpp"

namespace loki::uci
{
	loki_context::loki_context(std::ostream& os) :
		m_os{ os }, m_gamestate{ nullptr }
	{}

	void loki_context::uci() const
	{
		m_os << std::format("id name {} {}", NAME, VERSION) << std::endl;
		m_os << std::format("id author {}", AUTHOR) << std::endl;
		// options
		m_os << "uciok" << std::endl;
	}

	void loki_context::debug() const
	{
		throw_msg<not_implemented_error>("not implemented");
	}

	void loki_context::isready() const
	{
		// TODO: Also output this while searching if we're asked to. Perhaps have the main_thread listening for inputs...
		m_os << "readyok" << std::endl;
	}
	void loki_context::setoption(std::string name, std::optional<std::string> value)
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::register_() const
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::ucinewgame()
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::position(const std::string& fen, const std::vector<std::string>&)
	{
		m_gamestate = position::game_state::from_fen(fen);
		// TODO: Add moves
	}
	void loki_context::go(const search::limits*)
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::stop()
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::ponderhit()
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::printpos() const
	{
		if (m_gamestate == nullptr)
			throw_msg<loki_exception>("game_state pointer was null");

		m_os << "POSITION:" << std::endl;

		for (auto rank = position::RANK_8; rank >= position::RANK_1; rank--)
		{
			m_os << "\t" << rank + 1 << " ";

			for (auto file = position::FILE_A; file <= position::FILE_H; file++)
			{
				position::square sq(rank, file);
				side color = NUM_SIDES;
				auto piece_type = m_gamestate->get_piece(sq, &color);
				std::string piece_string = enum_to_string(piece_type);
				if (piece_type != NO_PIECE)
					piece_string = color == WHITE ?
						util::uppercase(piece_string) : util::lowercase(piece_string);
				m_os << piece_string << " ";
			}
			m_os << std::endl;
		}
		m_os << "\t" << " " << " "
			<< enum_to_string(position::FILE_A) << " "
			<< enum_to_string(position::FILE_B) << " "
			<< enum_to_string(position::FILE_C) << " "
			<< enum_to_string(position::FILE_D) << " "
			<< enum_to_string(position::FILE_E) << " "
			<< enum_to_string(position::FILE_F) << " "
			<< enum_to_string(position::FILE_G) << " "
			<< enum_to_string(position::FILE_H) << std::endl;
		m_os << "/POSITION" << std::endl;
		m_os << "INFORMATION" << std::endl;
		m_os << "\tSide to move:\t\t" << enum_to_string(m_gamestate->side_to_move) << std::endl;
		m_os << "\tEn-passant square:\t" << m_gamestate->en_passant_sq.to_algebraic() << std::endl;;
		m_os << "\tCastling rights:\t" << m_gamestate->castling_rights.to_string() << std::endl;
		m_os << "\tFifty move counter:\t" << m_gamestate->fifty_move_cnt << std::endl;
		m_os << "\tFull move counter:\t" << m_gamestate->full_move_cnt << std::endl;
		m_os << "\t" << "FEN:\t\t\t" << position::game_state::to_fen(m_gamestate) << std::endl;
		m_os << "/INFORMATION" << std::endl;
	}
}