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
#include <string>
#include "defs.hpp"
#include "position/square.hpp"
#include "util/exception.hpp"
#include "base_builder.hpp"
#include "position/game_state.hpp"

namespace loki::position::io
{
	class game_state_builder : public base_builder<std::string, game_state>
	{
	public:
		CHILD_EXCEPTION(fen_parsing_error, loki_exception);

		using bb_t = base_builder<std::string, game_state>;
	private:
		std::string m_piece_placements;
		char m_side_to_move;
		std::string m_castling_abilities;
		std::string m_en_passant_sq;
		std::string m_halfmove_clock;
		std::string m_fullmove_clock;

	protected:
		void reset_internal() override;

	public:
		game_state_builder() :
			m_piece_placements{}, m_castling_abilities{}, m_en_passant_sq{}, m_halfmove_clock{}, m_fullmove_clock{}, m_side_to_move{ '\0' }
		{}
		virtual ~game_state_builder() {}

		virtual base_builder& piece_placements() override;
		virtual base_builder& side_to_move() override;
		virtual base_builder& castling_ability() override;
		virtual base_builder& en_passant_square() override;
		virtual base_builder& halfmove_clock() override;
		virtual base_builder& fullmove_clock() override;

		// Necessary for testing
		const std::string& get_piece_placements() const { return m_piece_placements; }
		char get_side_to_move() const { return m_side_to_move; }
		const std::string& get_castling_abilities() const { return m_castling_abilities; }
		const std::string& get_en_passant_sq() const { return m_en_passant_sq; }
		const std::string& get_halfmove_clock() const { return m_halfmove_clock; }
		const std::string& get_fullmove_clock() const { return m_fullmove_clock; }

	private:
		void parse_row(e_rank rank, const std::string& fen_row);
	};
}