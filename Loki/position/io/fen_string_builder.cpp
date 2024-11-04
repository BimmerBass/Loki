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
#include "fen_string_builder.hpp"
#include "position/game_state.hpp"
#include "position/square.hpp"

namespace loki::position::io
{
	void fen_string_builder::reset_internal()
	{
		if (m_product != nullptr)
		{
			*m_product = "";
		}
	}

	inline char get_piece_character(piece p, side c)
	{
		char ch = 'p';
		switch (p)
		{
		case KNIGHT: ch = 'n'; break;
		case BISHOP: ch = 'b'; break;
		case ROOK: ch = 'r'; break;
		case QUEEN: ch = 'q'; break;
		case KING: ch = 'k'; break;
		}
		return c == WHITE ? (char)std::toupper(ch) : ch;
	}

	fen_string_builder::bb_t& fen_string_builder::piece_placements()
	{
		std::vector<std::string> fen_rows;

		for (auto r = RANK_8; r >= RANK_1; r--)
		{
			auto f = FILE_A;
			auto empty_squares = 0;
			std::string row = "";
			while (f <= FILE_H)
			{
				square sq(r, f);
				if (m_resource->piece_placements[WHITE][sq.value()] != NO_PIECE || m_resource->piece_placements[BLACK][sq.value()] != NO_PIECE)
				{
					if (empty_squares != 0)
						row += std::to_string(empty_squares);
					auto color = m_resource->piece_placements[WHITE][sq.value()] != NO_PIECE
						? WHITE : BLACK;
					auto piece_type = m_resource->piece_placements[color][sq.value()];
					row += get_piece_character(piece_type, color);
					empty_squares = 0;
				}
				else
				{
					empty_squares++;
				}
				f++;
			}

			if (empty_squares != 0)
				row += std::to_string(empty_squares);
			fen_rows.push_back(row);
		}
		m_product->append(util::join(fen_rows, '/') + " ");

		return *this;
	}
	fen_string_builder::bb_t& fen_string_builder::side_to_move()
	{
		m_product->append(m_resource->side_to_move == WHITE ? "w " : "b ");
		return *this;
	}
	fen_string_builder::bb_t& fen_string_builder::castling_ability()
	{
		std::string castle_rights = "";
		if (m_resource->castling_rights.can_castle<WHITE, KINGSIDE>())
			castle_rights += "K";
		if (m_resource->castling_rights.can_castle<WHITE, QUEENSIDE>())
			castle_rights += "Q";
		if (m_resource->castling_rights.can_castle<BLACK, KINGSIDE>())
			castle_rights += "k";
		if (m_resource->castling_rights.can_castle<BLACK, QUEENSIDE>())
			castle_rights += "q";

		m_product->append(castle_rights.empty() ? "-" : castle_rights);
		m_product->append(" ");
		return *this;
	}
	fen_string_builder::bb_t& fen_string_builder::en_passant_square()
	{
		m_product->append(m_resource->en_passant_sq == NO_SQ
			? "-" : m_resource->en_passant_sq.to_algebraic());
		m_product->append(" ");
		return *this;
	}
	fen_string_builder::bb_t& fen_string_builder::halfmove_clock()
	{
		m_product->append(m_resource->fifty_move_cnt == 0
			? "-" : std::to_string(m_resource->fifty_move_cnt));
		m_product->append(" ");
		return *this;
	}
	fen_string_builder::bb_t& fen_string_builder::fullmove_clock()
	{
		m_product->append(m_resource->full_move_cnt == 0
			? "-" : std::to_string(m_resource->full_move_cnt));
		return *this;
	}
}