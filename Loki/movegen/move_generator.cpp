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


	/*
#pragma region attackers_to instantiations
	template<> position::bitboard_t move_generator::attackers_to<PAWN>(const proxy_t*, position::e_square, side) const;
	template<> position::bitboard_t move_generator::attackers_to<KNIGHT>(const proxy_t*, position::e_square, side) const;
	template<> position::bitboard_t move_generator::attackers_to<BISHOP>(const proxy_t*, position::e_square, side) const;
	template<> position::bitboard_t move_generator::attackers_to<ROOK>(const proxy_t*, position::e_square, side) const;
	template<> position::bitboard_t move_generator::attackers_to<QUEEN>(const proxy_t*, position::e_square, side) const;
	template<> position::bitboard_t move_generator::attackers_to<KING>(const proxy_t*, position::e_square, side) const;
	template<> position::bitboard_t move_generator::attackers_to<NUM_PIECES>(const proxy_t*, position::e_square, side) const;


	template<>
	position::bitboard_t move_generator::attackers_to<PAWN>(const proxy_t* pos, position::e_square sq, side s) const
	{
		position::bitboard_t sq_bb = set_one_at(0ULL, sq);
		position::bitboard_t attackers = s == WHITE ?
			pos->piece_bb<WHITE, PAWN>() : pos->piece_bb<BLACK, PAWN>();
		position::bitboard_t attacks = s == WHITE ?
			position::shift<position::UP, position::LEFT>(sq_bb) | position::shift<position::UP, position::RIGHT>(sq_bb) :
			position::shift<position::DOWN, position::LEFT>(sq_bb) | position::shift<position::DOWN, position::RIGHT>(sq_bb);
		return attacks & attackers;
	}

	template<>
	position::bitboard_t move_generator::attackers_to<KNIGHT>(const proxy_t* pos, position::e_square sq, side s) const
	{
		auto knights = s == WHITE ? pos->piece_bb<WHITE, KNIGHT>() : pos->piece_bb<BLACK, KNIGHT>();
		return m_knight_attacks[sq] & knights;
	}

	template<>
	position::bitboard_t move_generator::attackers_to<BISHOP>(const proxy_t* pos, position::e_square sq, side s) const
	{
		auto bishops = s == WHITE ? pos->piece_bb<WHITE, BISHOP>() : pos->piece_bb<BLACK, BISHOP>();
		auto occ = pos->all_pieces<WHITE>() | pos->all_pieces<BLACK>();
		return m_bishop_index->attacks(sq, occ) & bishops;
	}

	template<>
	position::bitboard_t move_generator::attackers_to<ROOK>(const proxy_t* pos, position::e_square sq, side s) const
	{
		auto rooks = s == WHITE ? pos->piece_bb<WHITE, ROOK>() : pos->piece_bb<BLACK, ROOK>();
		auto occ = pos->all_pieces<WHITE>() | pos->all_pieces<BLACK>();
		return m_rook_index->attacks(sq, occ) & rooks;
	}

	template<>
	position::bitboard_t move_generator::attackers_to<QUEEN>(const proxy_t* pos, position::e_square sq, side s) const
	{
		auto queens = s == WHITE ? pos->piece_bb<WHITE, QUEEN>() : pos->piece_bb<BLACK, QUEEN>();
		auto occ = pos->all_pieces<WHITE>() | pos->all_pieces<BLACK>();
		auto attacks = m_rook_index->attacks(sq, occ) | m_bishop_index->attacks(sq, occ);
		return attacks & queens;
	}

	template<>
	position::bitboard_t move_generator::attackers_to<KING>(const proxy_t* pos, position::e_square sq, side s) const
	{
		auto king = s == WHITE ?
			position::set_one_at(0ULL, pos->king_square<WHITE>()) :
			position::set_one_at(0ULL, pos->king_square<BLACK>());
		return m_king_attacks[sq] & king;
	}


	template<>
	position::bitboard_t move_generator::attackers_to<NUM_PIECES>(const proxy_t* pos, position::e_square sq, side s) const
	{
		return
			attackers_to<PAWN>(pos, sq, s) |
			attackers_to<KNIGHT>(pos, sq, s) |
			attackers_to<BISHOP>(pos, sq, s) |
			attackers_to<ROOK>(pos, sq, s) |
			attackers_to<QUEEN>(pos, sq, s) |
			attackers_to<KING>(pos, sq, s);
	}
#pragma endregion
	
#pragma region misc. implementation
	
	template<> void move_generator::add_basic_attacks<WHITE, ACTIVE>(bitboard_t, e_square, const proxy_t*, ml_t*) const;
	template<> void move_generator::add_basic_attacks<WHITE, QUIET>(bitboard_t, e_square, const proxy_t*, ml_t*) const;
	template<> void move_generator::add_basic_attacks<WHITE, ALL>(bitboard_t, e_square, const proxy_t*, ml_t*) const;
	template<> void move_generator::add_basic_attacks<BLACK, ACTIVE>(bitboard_t, e_square, const proxy_t*, ml_t*) const;
	template<> void move_generator::add_basic_attacks<BLACK, QUIET>(bitboard_t, e_square, const proxy_t*, ml_t*) const;
	template<> void move_generator::add_basic_attacks<BLACK, ALL>(bitboard_t, e_square, const proxy_t*, ml_t*) const;

	template<side S, move_type MT>
	void move_generator::add_basic_attacks(
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
	
#pragma endregion

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
*/
}