#pragma once

#include "move_generator.hpp"
#include "position/position_proxy.hpp"


#include "position/square.hpp"
#include "position/castle_rights.hpp"
#include "magics/magic_index.hpp"
#include "util/exception.hpp"
#include "move_list.hpp"
#include "defs.hpp"

namespace loki::movegen
{
	template<move_type MT, piece PT>
	inline size_t move_generator::generate(const proxy_t* pos, ml_t* ml) const
	{
		return pos->game_state()->side_to_move == WHITE ?
			generate<WHITE, MT, PT>(pos, ml) : generate<BLACK, MT, PT>(pos, ml);
	}


	template<side S, move_type MT, piece PT>  requires (PT <= NUM_PIECES) && (S < NUM_SIDES) && (MT <= PROMOTION)
	size_t move_generator::generate(const proxy_t* pos, ml_t* ml) const
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

	template<side S, move_type MT>
	void move_generator::generate_all(const proxy_t* pos, ml_t* ml) const
	{
		generate_pawn<S, MT>(pos, ml);
		generate_knight<S, MT>(pos, ml);
		generate_bishop<S, MT>(pos, ml);
		generate_rook<S, MT>(pos, ml);
		generate_queen<S, MT>(pos, ml);
		generate_king<S, MT>(pos, ml);
	}

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

#pragma region generate_ methods
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
			bitboard_t left_attacks = shift<up, left>(pawns)& opp_pieces;
			bitboard_t right_attacks = shift<up, right>(pawns)& opp_pieces;
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
	void move_generator::generate_queen(const proxy_t* pos, ml_t* ml) const
	{
		bitboard_t queenbb = pos->piece_bb<S, QUEEN>();
		if (queenbb == 0)
			return;
		bitboard_t occupancy =
			pos->all_pieces<S>() |
			pos->all_pieces<!S>();

		while (queenbb != 0)
		{
			auto from_sq = pop_lsb(queenbb).value();
			bitboard_t all_attacks =
				m_rook_index->attacks(from_sq, occupancy) | m_bishop_index->attacks(from_sq, occupancy);
			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<side S, move_type MT>
	void move_generator::generate_king(const proxy_t* pos, ml_t* ml) const
	{
		assert(pos->king_square<S>() == scan_lsb(pos->piece_bb<S, KING>()).value());
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
#pragma endregion

#pragma region attackers_to instantiations

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

}