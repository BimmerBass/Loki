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
#include "position/search_position.hpp"
#include "magics/magic_index.hpp"
#include "util/exception.hpp"
#include "move_list.hpp"

namespace loki::movegen
{
	class move_generator
	{
		CHILD_EXCEPTION(movegen_exception, loki_exception);
	private:
		using atk_table = std::array<position::bitboard_t, position::NUM_SQUARES>;
		using inx_t = std::shared_ptr<magics::magic_index>;
		using proxy_t = position::position_proxy;
		using ml_t = move_list;

		atk_table m_knight_attacks;
		atk_table m_king_attacks;
		inx_t m_rook_index;
		inx_t m_bishop_index;
	public:
		/// <summary>
		/// Initialize a move_generator object.
		/// </summary>
		/// <param name="rook_inx">A shared pointer to a magic rook table</param>
		/// <param name="bishop_inx">A shared pointer to a magic bishop table</param>
		move_generator(inx_t rook_inx, inx_t bishop_inx)
			: m_rook_index{ rook_inx }, m_bishop_index{ bishop_inx }, m_knight_attacks{0ULL}, m_king_attacks{0ULL}
		{
			init_knight_table();
			init_king_table();
		}

		move_generator(const move_generator&) = delete;
		move_generator(move_generator&&) = delete;

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
		size_t generate(const proxy_t* pos, ml_t* ml) const
		{
			if (pos == nullptr || ml == nullptr)
				throw_msg<movegen_exception>("invalid arguments to generate(): either pos ({}) or ml ({}) is nullptr.", (intptr_t)pos, (intptr_t)ml);
			ml->clear();
			
			if constexpr (PT == NUM_PIECES)
				generate_all<S, MT>(pos, ml);
			else if constexpr (PT == PAWN)
				generate_pawn<S, MT>(pos, ml);
			else if constexpr (PT == KNIGHT)
				generate_knight<S, MT>(pos, ml);
			else if constexpr (PT == BISHOP)
				generate_bishop<S, MT>(pos, ml);
			else if constexpr (PT == ROOK)
				generate_rook<S, MT>(pos, ml);
			else if constexpr (PT == QUEEN)
				generate_queen<S, MT>(pos, ml);
			else
				generate_king<S, MT>(pos, ml);
			return ml->size();
		}

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
		void generate_king(const proxy_t* pos, ml_t* ml) const;

		template<side S, move_type MT>
		void generate_queen(const proxy_t* pos, ml_t* ml) const
		{
			return generate_rook<S, MT>(pos, ml) + generate_bishop<S, MT>(pos, ml);
		}

		template<side S, move_type MT>
		void generate_all(const proxy_t* pos, ml_t* ml)
		{
			generate_pawn<S, MT>(pos, ml);
			generate_knight<S, MT>(pos, ml);
			generate_bishop<S, MT>(pos, ml);
			generate_rook<S, MT>(pos, ml);
			generate_queen<S, MT>(pos, ml);
			generate_king<S, MT>(pos, ml);
		}

		template<side S, move_type MT>
		void add_basic_attacks(
			position::bitboard_t all_attacks,
			position::e_square from_sq,
			const proxy_t* pos,
			ml_t* ml) const
		{
			auto valid_attacks = 0ULL;

			if constexpr (MT == ACTIVE || MT == ALL)
				valid_attacks |= all_attacks & pos->all_pieces<!S>();
			if constexpr (MT == QUIET || MT == ALL)
				valid_attacks |= all_attacks & ~pos->all_pieces<!S>();

			while (valid_attacks != 0)
			{
				auto to_sq = position::pop_lsb(valid_attacks).value();
				ml->push_back(from_sq, to_sq);
			}
		}

		void add_promotions(position::e_square from, position::e_square to, ml_t* ml)
		{
			ml->push_back(move(from, to, PROMOTION, KNIGHT));
			ml->push_back(move(from, to, PROMOTION, BISHOP));
			ml->push_back(move(from, to, PROMOTION, ROOK));
			ml->push_back(move(from, to, PROMOTION, QUEEN));
		}
	};
}