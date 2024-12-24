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
#include "move_generator.hpp"
#include "defs.hpp"

namespace loki::movegen
{
	using namespace loki::position;

	void move_generator::init_king_table()
	{
		for (auto sq = A1; sq <= H8; sq++)
		{
			auto sqBB = 1ULL << sq;
			m_king_attacks[sq] =
				shift<UP>(sqBB) | shift<DOWN>(sqBB) | shift<LEFT>(sqBB) | shift<RIGHT>(sqBB) |
				shift<UP, LEFT>(sqBB) | shift<UP, RIGHT>(sqBB) | shift<DOWN, LEFT>(sqBB) | shift<DOWN, RIGHT>(sqBB);
		}
	}
	void move_generator::init_knight_table()
	{
		for (auto sq = A1; sq <= H8; sq++)
		{
			auto sqBB = 1ULL << sq;
			m_knight_attacks[sq] = 0ULL;
			
			
			m_knight_attacks[sq] |=
				shift_combine<UP, UP, UP, LEFT>(sqBB) | shift_combine<UP, UP, UP, RIGHT>(sqBB) |
				shift_combine<DOWN, DOWN, DOWN, LEFT>(sqBB) | shift_combine<DOWN, DOWN, DOWN, RIGHT>(sqBB) |
				shift_combine<LEFT, LEFT, LEFT, UP>(sqBB) | shift_combine<LEFT, LEFT, LEFT, DOWN>(sqBB) |
				shift_combine<RIGHT, RIGHT, RIGHT, UP>(sqBB) | shift_combine<RIGHT, RIGHT, RIGHT, DOWN>(sqBB);
		}
	}

	template<side S, move_type MT>
	void move_generator::generate_pawn(const proxy_t* pos, ml_t* ml) const
	{
		constexpr e_rank last_rank = S == WHITE ? RANK_8 : RANK_1;
		constexpr e_rank third_rank = S == WHITE ? RANK_3 : RANK_6;
		constexpr e_direction up = S == WHITE ? UP : DOWN;
		constexpr e_direction down = S == WHITE ? DOWN : UP;
		constexpr e_direction left = S == WHITE ? LEFT : RIGHT;
		constexpr e_direction right = S == WHITE ? RIGHT : LEFT;
		constexpr int left_attack_origin = S == WHITE ? 7 : -9;
		constexpr int right_attack_origin = S == WHITE ? 9 : -7;

		auto old_size = ml->size();
		bitboard_t pawns = pos->piece_bb<S, PAWN>();
		bitboard_t occ = pos->all_pieces<S>() | pos->all_pieces<!S>();
		bitboard_t opp_pieces = pos->all_pieces<!S>();

		if (pawns == 0)
			return;

		if constexpr (MT == QUIET || MT == ALL) // one-up, two-up
		{
			bitboard_t one_up = shift<up>(pawns) & ~(occ | RANK_MASKS[last_rank]);
			bitboard_t two_up = shift<up>(one_up & RANK_MASKS[third_rank]) & ~occ;

			while (one_up)
			{
				auto inx = pop_lsb(one_up).value();
				ml->push_back(S == WHITE ? inx - 8 : inx + 8, inx);
			}
			while (two_up)
			{
				auto inx = pop_lsb(two_up).value();
				ml->push_back(S == WHITE ? inx - 16 : inx + 16, inx);
			}
		}
		if constexpr (MT == ACTIVE || MT == ALL) // attacks, en-passant, promotions
		{
			// Attacks
			bitboard_t left_attacks = shift<up, left>(pawns) & opp_pieces;
			bitboard_t right_attacks = shift<up, right>(pawns) & opp_pieces;
			bitboard_t promotions = shift<up>(pawns) & (RANK_MASKS[last_rank] & ~occ);

			while (left_attacks)
			{
				square inx = pop_lsb(left_attacks).value();

				if (inx.rank() == last_rank) // promotion capture.
					add_promotions(inx - left_attack_origin, inx, ml);
				else
					ml->push_back(inx - left_attack_origin, inx);
			}
			while (right_attacks)
			{
				square inx = pop_lsb(right_attacks).value();

				if (inx.rank() == last_rank) // promotion capture.
					add_promotions(inx - right_attack_origin, inx, ml);
				else
					ml->push_back(inx - right_attack_origin, inx);
			}

			// Promotions
			while (promotions)
			{
				auto inx = pop_lsb(promotions).value();
				add_promotions(S == WHITE ? inx - 8 : inx + 8, inx, ml);
			}

			// En-passant
			auto en_passant_sq = pos->game_state()->en_passant_sq.value();
			if (en_passant_sq != NUM_SQUARES)
			{
				bitboard_t ep_board = 1ULL << en_passant_sq;

				if (shift<down, left>(ep_board) & pawns) // we use right attack origin since from the attacker's perspective it is a right-attack
					ml->push_back(en_passant_sq - right_attack_origin, en_passant_sq, ENPASSANT, KNIGHT);
				if (shift<down, right>(ep_board) & pawns)
					ml->push_back(en_passant_sq - left_attack_origin, en_passant_sq, ENPASSANT, KNIGHT);
			}
		}
	}
	template<side S, move_type MT>
	void move_generator::generate_knight(const proxy_t* pos, ml_t* ml) const
	{
		bitboard_t knightbb = pos->piece_bb<S, KNIGHT>();
		if (knightbb == 0)
			return;

		while (knightbb != 0)
		{
			auto from_sq = pop_lsb(knightbb).value();
			bitboard_t all_attacks = m_knight_attacks[from_sq] & ~pos->all_pieces<S>();
			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<side S, move_type MT>
	void move_generator::generate_bishop(const proxy_t* pos, ml_t* ml) const
	{
		bitboard_t bishopbb = pos->piece_bb<S, BISHOP>();
		if (bishopbb == 0)
			return;

		bitboard_t occupancy =
			pos->all_pieces<S>() |
			pos->all_pieces<!S>();

		while (bishopbb != 0)
		{
			auto from_sq = pop_lsb(bishopbb).value();
			bitboard_t all_attacks = m_bishop_index->attacks(from_sq, occupancy);
			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<side S, move_type MT>
	void move_generator::generate_rook(const proxy_t* pos, ml_t* ml) const
	{
		bitboard_t rookbb = pos->piece_bb<S, ROOK>();
		if (rookbb == 0)
			return;

		bitboard_t occupancy =
			pos->all_pieces<S>() |
			pos->all_pieces<!S>();

		while (rookbb != 0)
		{
			auto from_sq = pop_lsb(rookbb).value();
			bitboard_t all_attacks = m_rook_index->attacks(from_sq, occupancy);
			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<side S, move_type MT>
	void move_generator::generate_king(const proxy_t* pos, ml_t* ml) const
	{
		assert(pos->king_square<S>() == scan_lsb(pos->piece_bb<S,KING>()).value());
		auto king_sq = pos->king_square<S>();
		bitboard_t attacks = m_king_attacks[king_sq];
		bitboard_t valid_attacks = 0;

		if constexpr (MT == ACTIVE || MT == ALL)
			valid_attacks |= attacks & pos->all_pieces<!S>();
		if constexpr (MT == QUIET || MT == ALL)
		{
			valid_attacks |= attacks & ~pos->all_pieces<!S>();

			// castling
			if (pos->game_state()->castling_rights.can_castle<S, KINGSIDE>())
				ml->push_back(king_sq, S == WHITE ? G1 : G8, CASTLING, KNIGHT);
			if (pos->game_state()->castling_rights.can_castle<S, QUEENSIDE>())
				ml->push_back(king_sq, S == WHITE ? C1 : C8, CASTLING, KNIGHT);
		}
		while (valid_attacks != 0)
		{
			auto to_sq = pop_lsb(valid_attacks).value();
			ml->push_back(king_sq, to_sq);
		}
	}

#pragma region templates
#define INSTANTIATE_GENERATE_METHOD(MethodName) \
	template<> void move_generator::MethodName<WHITE, ACTIVE>(const proxy_t*, ml_t*) const;		\
	template<> void move_generator::MethodName<WHITE, QUIET>(const proxy_t*, ml_t*) const;		\
	template<> void move_generator::MethodName<WHITE, ALL>(const proxy_t*, ml_t*) const;			\
	template<> void move_generator::MethodName<BLACK, ACTIVE>(const proxy_t*, ml_t*) const;		\
	template<> void move_generator::MethodName<BLACK, QUIET>(const proxy_t*, ml_t*) const;		\
	template<> void move_generator::MethodName<BLACK, ALL>(const proxy_t*, ml_t*) const;

	INSTANTIATE_GENERATE_METHOD(generate_pawn);
	INSTANTIATE_GENERATE_METHOD(generate_knight);
	INSTANTIATE_GENERATE_METHOD(generate_bishop);
	INSTANTIATE_GENERATE_METHOD(generate_rook);
	INSTANTIATE_GENERATE_METHOD(generate_king);
#undef INSTANTIATE_GENERATE_METHOD
#pragma endregion
}