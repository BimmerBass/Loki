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
#include "i_move_generator.hpp"
#include "magics/magic_index.hpp"
#include "util/exception.hpp"
#include "move_list.hpp"
#include "position/bitboard.hpp"
#include "position/square.hpp"
#include "defs.hpp"


namespace loki::movegen
{
	class move_generator final : public i_move_generator
	{
		CHILD_EXCEPTION(movegen_exception, loki_exception);
	private:
		using atk_table = std::array<position::bitboard_t, position::NUM_SQUARES>;
		using pos_t = position::i_position_view;
		using ml_t = move_list;

		atk_table m_knight_attacks;
		atk_table m_king_attacks;
		magics::magic_index_t m_rook_index;
		magics::magic_index_t m_bishop_index;
	public:
		/// <summary>
		/// Initialize a move_generator object.
		/// </summary>
		/// <param name="rook_inx">A shared pointer to a magic rook table</param>
		/// <param name="bishop_inx">A shared pointer to a magic bishop table</param>
		move_generator(magics::magic_index_t rook_inx, magics::magic_index_t bishop_inx)
			: m_rook_index{ rook_inx }, m_bishop_index{ bishop_inx }, m_knight_attacks{0ULL}, m_king_attacks{0ULL}
		{
			init_knight_table();
			init_king_table();
		}

		move_generator(const move_generator&) = delete;
		move_generator(move_generator&&) = delete;

		size_t generate_internal(const pos_t* pos, ml_t* ml, side s, move_type mt, piece pt) const;
		position::bitboard_t attackers_to_internal(const pos_t* pos, position::e_square sq, side s, piece pt) const;

	private:
		void init_knight_table();
		void init_king_table();

		
		template<side S, move_type MT> void generate_pawn(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_knight(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_bishop(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_rook(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_queen(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_king(const pos_t* pos, ml_t* ml) const;

		template<side S, move_type MT>
		void generate_all(const pos_t* pos, ml_t* ml, piece pt) const;

		template<side S, move_type MT>
		void add_basic_attacks(
			position::bitboard_t all_attacks,
			position::e_square from_sq,
			const pos_t* pos,
			ml_t* ml) const;

		position::bitboard_t pawn_attackers(const pos_t* pos, position::e_square sq, side s) const;
		position::bitboard_t knight_attackers(const pos_t* pos, position::e_square sq, side s) const;
		position::bitboard_t bishop_attackers(const pos_t* pos, position::e_square sq, side s) const;
		position::bitboard_t rook_attackers(const pos_t* pos, position::e_square sq, side s) const;
		position::bitboard_t queen_attackers(const pos_t* pos, position::e_square sq, side s) const;
		position::bitboard_t king_attackers(const pos_t* pos, position::e_square sq, side s) const;


		void add_promotions(position::e_square from, position::e_square to, ml_t* ml) const
		{
			ml->push_back(move(from, to, PROMOTION, KNIGHT));
			ml->push_back(move(from, to, PROMOTION, BISHOP));
			ml->push_back(move(from, to, PROMOTION, ROOK));
			ml->push_back(move(from, to, PROMOTION, QUEEN));
		}
	};
}

//#include "move_generator_templates.hpp"