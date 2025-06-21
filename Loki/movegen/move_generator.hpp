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
#include "magics/magic_index.hpp"
#include "util/exception.hpp"
#include "move_list.hpp"
#include "defs.hpp"


namespace loki::movegen
{
	class move_generator
	{
		CHILD_EXCEPTION(movegen_exception, loki_exception);
	private:
		using atk_table = std::array<position::bitboard_t, position::NUM_SQUARES>;
		using proxy_t = position::position_proxy;
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

		template<move_type MT = ALL, piece PT = NUM_PIECES>
		inline size_t generate(const proxy_t* pos, ml_t* ml) const;

		/// <summary>
		/// Generate all moves for a position given a piece type and side.
		/// </summary>
		/// <typeparam name="S">Side to generate for</typeparam>
		/// <typeparam name="MT">Move type to generate</typeparam>
		/// <typeparam name="PT">Piece type to generate for. To generate for all pieces, use the default NUM_PIECES</typeparam>
		/// <param name="pos">A pointer to a position object to generate for.</param>
		/// <param name="move_list">A move_list pointer to insert the result. Note that the original data will be cleared.</param>
		/// <returns>A size_t integer denoting the number of generated moves.</returns>
		template<side S, move_type MT = ALL, piece PT = NUM_PIECES> requires (PT <= NUM_PIECES) && (S < NUM_SIDES) && (MT <= PROMOTION)
		size_t generate(const proxy_t* pos, ml_t* ml) const;

		/// <summary>
		/// Get the attackers of some piece type to a given square
		/// </summary>
		/// <typeparam name="PT">The piece type. Default: All pieces</typeparam>
		/// <param name="sq">The square</param>
		/// <param name="s">The side</param>
		/// <returns>A bitboard with the attacking pieces to the square</returns>
		template<piece PT = NUM_PIECES> requires (PT >= PAWN && PT <= NUM_PIECES)
		position::bitboard_t attackers_to(const proxy_t* pos, position::e_square sq, side s) const;

	private:
		void init_knight_table();
		void init_king_table();

		template<side S, move_type MT>
		void generate_pawn(const proxy_t* pos, ml_t* ml) const;
		template<side S, move_type MT>
		void generate_knight(const proxy_t* pos, ml_t* ml) const;
		template<side S, move_type MT>
		void generate_bishop(const proxy_t* pos, ml_t* ml) const;
		template<side S, move_type MT>
		void generate_rook(const proxy_t* pos, ml_t* ml) const;
		template<side S, move_type MT>
		void generate_queen(const proxy_t* pos, ml_t* ml) const;
		template<side S, move_type MT>
		void generate_king(const proxy_t* pos, ml_t* ml) const;

		template<side S, move_type MT>
		void generate_all(const proxy_t* pos, ml_t* ml) const;

		template<side S, move_type MT>
		void add_basic_attacks(
			position::bitboard_t all_attacks,
			position::e_square from_sq,
			const proxy_t* pos,
			ml_t* ml) const;

		void add_promotions(position::e_square from, position::e_square to, ml_t* ml)
		{
			ml->push_back(move(from, to, PROMOTION, KNIGHT));
			ml->push_back(move(from, to, PROMOTION, BISHOP));
			ml->push_back(move(from, to, PROMOTION, ROOK));
			ml->push_back(move(from, to, PROMOTION, QUEEN));
		}
	};
}

#include "move_generator_templates.hpp"