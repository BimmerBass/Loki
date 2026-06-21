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
#include "game_state_builder.hpp"
#include "../game_state.hpp"
#include <algorithm>
#include <cassert>
#include <sstream>

namespace loki::position::io
{
	void game_state_builder::reset_internal()
	{
		// Reset the game_state instance.
		if (m_product != nullptr)
		{
			m_product->piece_placements[WHITE].fill(NO_PIECE);
			m_product->piece_placements[BLACK].fill(NO_PIECE);
			m_product->side_to_move = WHITE;
			m_product->fifty_move_cnt = m_product->full_move_cnt = 0;
			m_product->en_passant_sq = NO_SQ;
			m_product->castling_rights = castle_rights();
		}

		if (m_resource != nullptr)
		{
			std::istringstream iss(*m_resource);
			if (!(iss >> m_piece_placements >> m_side_to_move >> m_castling_abilities))
				throw_msg<fen_parsing_error>("invalid fen (missing either pieces, side to move or castling rights): '{}'", *m_resource);

			if (!(iss >> m_en_passant_sq))
				m_en_passant_sq = "-";
			if (!(iss >> m_halfmove_clock >> m_fullmove_clock))
				m_halfmove_clock = m_fullmove_clock = "-";
		}
	}
	game_state_builder::bb_t& game_state_builder::piece_placements()
	{
		auto rows = util::split(m_piece_placements, '/');
		std::reverse(rows.begin(), rows.end());
		if (rows.size() != NUM_RANKS)
			throw_msg<fen_parsing_error>("invalid piece placements. the string represents '{}' ranks, expected '{}'", rows.size(), (int)NUM_RANKS);

		for (auto rank = RANK_1; rank <= RANK_8; rank++)
		{
			parse_row(rank, rows[rank]);
		}

		return *this;
	}
	game_state_builder::bb_t& game_state_builder::side_to_move()
	{
		if (m_side_to_move == 'w')
			m_product->side_to_move = WHITE;
		else if (m_side_to_move == 'b')
			m_product->side_to_move = BLACK;
		else
			throw_msg<fen_parsing_error>("invalid side to move: '{}'", m_side_to_move);
		return *this;
	}
	game_state_builder::bb_t& game_state_builder::castling_ability()
	{
		m_product->castling_rights = castle_rights();
		if (m_castling_abilities == "-")
			return *this;
		else if (m_castling_abilities.find_first_not_of("KQkq") != std::string::npos)
			throw_msg<fen_parsing_error>("invalid non-empty castling string: '{}'", m_castling_abilities);
		
		// now "KQkq" are the only possible elements in the string.
		// and the object in our product is empty.
		for (auto c : m_castling_abilities)
		{
			switch (c)
			{
			case 'K': m_product->castling_rights.set<WHITE, KINGSIDE>(true); break;
			case 'Q': m_product->castling_rights.set<WHITE, QUEENSIDE>(true); break;
			case 'k': m_product->castling_rights.set<BLACK, KINGSIDE>(true); break;
			case 'q': m_product->castling_rights.set<BLACK, QUEENSIDE>(true); break;
			default: // guaranteed not to happen, but better safe than sorry
				assert(false);
			}
		}
		return *this;
	}
	game_state_builder::bb_t& game_state_builder::en_passant_square()
	{
		m_product->en_passant_sq = NO_SQ;
		if (m_en_passant_sq != "-")
			m_product->en_passant_sq = square(m_en_passant_sq);
		return *this;
	}
	game_state_builder::bb_t& game_state_builder::halfmove_clock()
	{
		m_product->fifty_move_cnt = 0;
		if (m_halfmove_clock != "-")
			m_product->fifty_move_cnt = std::stoll(m_halfmove_clock);
		return *this;
	}
	game_state_builder::bb_t& game_state_builder::fullmove_clock()
	{
		m_product->full_move_cnt = 0;
		if (m_fullmove_clock != "-")
			m_product->full_move_cnt = std::stoll(m_fullmove_clock);
		return *this;
	}

	void game_state_builder::parse_row(e_rank rank, const std::string& fen_row)
	{
		auto file = FILE_A;
		auto it =  fen_row.begin();

		while (it != fen_row.end())
		{
			if (std::isdigit(*it))
			{
				auto increment = (*it) - '0';
				if ((int)file + increment > FILE_H + 1)
					throw_msg<fen_parsing_error>("row '{}' (increment '{}') is too wide", fen_row, *it);
				file += increment;
			}
			else
			{
				auto color = std::isupper(*it) ? WHITE : BLACK;
				square sq(rank, file);
				switch (std::tolower(*it))
				{
				case 'p':
					m_product->piece_placements[color][sq.value()] = PAWN;
					break;
				case 'n':
					m_product->piece_placements[color][sq.value()] = KNIGHT;
					break;
				case 'b':
					m_product->piece_placements[color][sq.value()] = BISHOP;
					break;
				case 'r':
					m_product->piece_placements[color][sq.value()] = ROOK;
					break;
				case 'q':
					m_product->piece_placements[color][sq.value()] = QUEEN;
					break;
				case 'k':
					m_product->piece_placements[color][sq.value()] = KING;
					break;
				default:
					throw_msg<fen_parsing_error>("invalid character '{}' in row '{}'", *it, fen_row);
				}
				file += 1;
			}

			it++;
		}
	}
}
