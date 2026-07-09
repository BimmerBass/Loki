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

#include "flipped_fen_builder.hpp"
#include "util/stringops.hpp"
#include <algorithm>
#include <cctype>

namespace loki::position::io
{
	namespace
	{
		char flip_piece_color(char c)
		{
			return std::islower((unsigned char)c) ? (char)std::toupper((unsigned char)c) : (char)std::tolower((unsigned char)c);
		}
	}

	void flipped_fen_builder::reset_internal()
	{
		if (m_product != nullptr)
		{
			*m_product = "";
		}

		if (m_resource != nullptr)
		{
			splitted = game_state_builder::splitted_fen(*m_resource);
		}
	}

	flipped_fen_builder::bb_t& flipped_fen_builder::piece_placements()
	{
		auto rows = util::split(splitted.piece_placements, '/');
		std::reverse(rows.begin(), rows.end());

		for (auto& row : rows)
		{
			std::transform(row.begin(), row.end(), row.begin(), [](char c) {
				return std::isdigit((unsigned char)c) ? c : flip_piece_color(c);
			});
		}

		m_product->append(util::join(rows, '/') + " ");

		return *this;
	}
	flipped_fen_builder::bb_t& flipped_fen_builder::side_to_move()
	{
		if (splitted.side_to_move == 'w')
			m_product->append("b ");
		else if (splitted.side_to_move == 'b')
			m_product->append("w ");
		else
			throw_msg<game_state_builder::fen_parsing_error>("invalid side to move: '{}'", splitted.side_to_move);
		return *this;
	}
	flipped_fen_builder::bb_t& flipped_fen_builder::castling_ability()
	{
		if (splitted.castling_abilities == "-")
		{
			m_product->append("- ");
			return *this;
		}
		else if (splitted.castling_abilities.find_first_not_of("KQkq") != std::string::npos)
		{
			throw_msg<game_state_builder::fen_parsing_error>("invalid non-empty castling string: '{}'", splitted.castling_abilities);
		}

		std::string castling_ability = "";
		if (splitted.castling_abilities.find('k') != std::string::npos)
			castling_ability += "K";
		if (splitted.castling_abilities.find('q') != std::string::npos)
			castling_ability += "Q";
		if (splitted.castling_abilities.find('K') != std::string::npos)
			castling_ability += "k";
		if (splitted.castling_abilities.find('Q') != std::string::npos)
			castling_ability += "q";
		m_product->append(castling_ability + " ");
		return *this;
	}
	flipped_fen_builder::bb_t& flipped_fen_builder::en_passant_square()
	{
		if (splitted.en_passant_sq == "-")
		{
			m_product->append("- ");
			return *this;
		}

		auto en_passant_sq = splitted.en_passant_sq;
		en_passant_sq[1] = (char)('9' - en_passant_sq[1]);
		m_product->append(en_passant_sq + " ");
		return *this;
	}
	flipped_fen_builder::bb_t& flipped_fen_builder::halfmove_clock()
	{
		m_product->append(splitted.halfmove_clock + " ");
		return *this;
	}
	flipped_fen_builder::bb_t& flipped_fen_builder::fullmove_clock()
	{
		m_product->append(splitted.fullmove_clock);
		return *this;
	}
}
