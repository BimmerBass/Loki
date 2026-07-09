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

	template<position_view pos_t>
	class move_generator final : public i_move_generator<pos_t>
	{
		CHILD_EXCEPTION(movegen_exception, loki_exception);
	private:
		using atk_table = std::array<position::bitboard_t, position::NUM_SQUARES>;
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
		move_generator& operator=(const move_generator&) = delete;

		move_generator(move_generator&&) noexcept = delete;
		move_generator& operator=(const move_generator&&) noexcept = delete;

		size_t generate_internal(const pos_t* pos, ml_t* ml, side s, move_type mt, piece pt) const;
		position::bitboard_t attackers_to_internal(const pos_t* pos, position::e_square sq, side s, piece pt) const;

		inline magics::magic_index_t bishop_index() const noexcept override { return m_bishop_index; }
		inline magics::magic_index_t rook_index() const noexcept override { return m_rook_index; }
	private:
		void init_knight_table();
		void init_king_table();

		
		template<side S, move_type MT> void generate_pawn(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_knight(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_bishop(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_rook(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_queen(const pos_t* pos, ml_t* ml) const;
		template<side S, move_type MT> void generate_king(const pos_t* pos, ml_t* ml) const;

		
		template<side S, position::castling_direction D> 
		bool castle_is_safe(const pos_t* pos) const;
		template<side S> bool square_attacked(const pos_t* pos, position::e_square sq) const;

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




	using namespace loki::position;

	template<position_view pos_t>
	void move_generator<pos_t>::init_king_table()
	{
		for (auto sq = A1; sq <= H8; sq++)
		{
			auto sqBB = 1ULL << sq;
			m_king_attacks[sq] =
				shift<UP>(sqBB) | shift<DOWN>(sqBB) | shift<LEFT>(sqBB) | shift<RIGHT>(sqBB) |
				shift<UP, LEFT>(sqBB) | shift<UP, RIGHT>(sqBB) | shift<DOWN, LEFT>(sqBB) | shift<DOWN, RIGHT>(sqBB);
		}
	}

	template<position_view pos_t>
	void move_generator<pos_t>::init_knight_table()
	{
		for (auto sq = A1; sq <= H8; sq++)
		{
			auto file = square(sq).file();
			auto rank = square(sq).rank();
			auto sqBB = 1ULL << sq;
			m_knight_attacks[sq] = 0ULL;

			m_knight_attacks[sq] =
				((shift_combine<UP, UP, LEFT>(sqBB) | shift_combine<UP, UP, RIGHT>(sqBB) |
					shift_combine<DOWN, DOWN, LEFT>(sqBB) | shift_combine<DOWN, DOWN, RIGHT>(sqBB)) & ~FILE_MASKS[file]) |
				((shift_combine<LEFT, LEFT, UP>(sqBB) | shift_combine<LEFT, LEFT, DOWN>(sqBB) |
					shift_combine<RIGHT, RIGHT, UP>(sqBB) | shift_combine<RIGHT, RIGHT, DOWN>(sqBB)) & ~RANK_MASKS[rank]);
		}
	}

	template<position_view pos_t>
	size_t move_generator<pos_t>::generate_internal(const pos_t* pos, ml_t* ml, side s, move_type mt, piece pt) const
	{
		if (pos == nullptr || ml == nullptr)
			throw_msg<movegen_exception>("invalid arguments to generate(): either pos ({}) or ml ({}) is nullptr.", (intptr_t)pos, (intptr_t)ml);

		ml->clear();

		switch (mt)
		{
		case QUIET: (s == WHITE) ? generate_all<WHITE, QUIET>(pos, ml, pt) : generate_all<BLACK, QUIET>(pos, ml, pt); break;
		case ACTIVE: (s == WHITE) ? generate_all<WHITE, ACTIVE>(pos, ml, pt) : generate_all<BLACK, ACTIVE>(pos, ml, pt); break;
		case ALL: (s == WHITE) ? generate_all<WHITE, ALL>(pos, ml, pt) : generate_all<BLACK, ALL>(pos, ml, pt); break;
		}

		return ml->size();
	}

	template<position_view pos_t>
	bitboard_t move_generator<pos_t>::attackers_to_internal(const pos_t* pos, position::e_square sq, side s, piece pt) const
	{
		assert(pt >= PAWN && pt <= NUM_PIECES);
		if (pt == NUM_PIECES)
		{
			return
				pawn_attackers(pos, sq, s) |
				knight_attackers(pos, sq, s) |
				bishop_attackers(pos, sq, s) |
				rook_attackers(pos, sq, s) |
				queen_attackers(pos, sq, s) |
				king_attackers(pos, sq, s);
		}

		switch (pt)
		{
		case PAWN: return pawn_attackers(pos, sq, s);
		case KNIGHT: return knight_attackers(pos, sq, s);
		case BISHOP: return bishop_attackers(pos, sq, s);
		case ROOK: return rook_attackers(pos, sq, s);
		case QUEEN: return queen_attackers(pos, sq, s);
		case KING: return king_attackers(pos, sq, s);
		default:
			throw_msg<movegen_exception>("Cannot generate attackers for piece type {}", (int)pt);
		}
	}

	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::add_basic_attacks(
		position::bitboard_t all_attacks,
		position::e_square from_sq,
		const pos_t* pos,
		ml_t* ml) const
	{
		auto valid_attacks = 0ULL;

		if constexpr (MT == ACTIVE || MT == ALL)
			valid_attacks |= all_attacks & pos->all_pieces(!S);
		if constexpr (MT == QUIET || MT == ALL)
			valid_attacks |= all_attacks & ~pos->all_pieces(!S);

		while (valid_attacks != 0)
		{
			auto to_sq = position::pop_lsb_nonzero(valid_attacks);
			ml->push_back(move(from_sq, to_sq));
		}
	}


#pragma region generate implementations
	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::generate_all(const pos_t* pos, ml_t* ml, piece pt) const
	{
		if (pt == PAWN || pt == NUM_PIECES)
			generate_pawn<S, MT>(pos, ml);
		if (pt == KNIGHT || pt == NUM_PIECES)
			generate_knight<S, MT>(pos, ml);
		if (pt == BISHOP || pt == NUM_PIECES)
			generate_bishop<S, MT>(pos, ml);
		if (pt == ROOK || pt == NUM_PIECES)
			generate_rook<S, MT>(pos, ml);
		if (pt == QUEEN || pt == NUM_PIECES)
			generate_queen<S, MT>(pos, ml);
		if (pt == KING || pt == NUM_PIECES)
			generate_king<S, MT>(pos, ml);
	}

	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::generate_pawn(const pos_t* pos, ml_t* ml) const
	{
		constexpr e_rank last_rank = S == WHITE ? RANK_8 : RANK_1;
		constexpr e_rank third_rank = S == WHITE ? RANK_3 : RANK_6;
		constexpr e_direction up = S == WHITE ? UP : DOWN;
		constexpr e_direction down = S == WHITE ? DOWN : UP;
		constexpr e_direction left = S == WHITE ? LEFT : RIGHT;
		constexpr e_direction right = S == WHITE ? RIGHT : LEFT;
		constexpr int left_attack_origin = S == WHITE ? 7 : -7;
		constexpr int right_attack_origin = S == WHITE ? 9 : -9;

		bitboard_t pawns = pos->piece_bb(S, PAWN);
		bitboard_t occ = pos->all_pieces();
		bitboard_t opp_pieces = pos->all_pieces(!S);

		if (pawns == 0)
			return;

		if constexpr (MT == QUIET || MT == ALL) // one-up, two-up
		{
			bitboard_t one_up = shift<up>(pawns) & ~(occ | RANK_MASKS[last_rank]);
			bitboard_t two_up = shift<up>(one_up & RANK_MASKS[third_rank]) & ~occ;

			while (one_up)
			{
				auto inx = pop_lsb_nonzero(one_up);
				ml->push_back(move(S == WHITE ? inx - 8 : inx + 8, inx));
			}
			while (two_up)
			{
				auto inx = pop_lsb_nonzero(two_up);
				ml->push_back(move(S == WHITE ? inx - 16 : inx + 16, inx));
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
				square inx = pop_lsb_nonzero(left_attacks);

				if (inx.rank() == last_rank) // promotion capture.
					add_promotions(static_cast<e_square>(inx.value() - left_attack_origin), inx.value(), ml);
				else
					ml->push_back(move(inx.value() - left_attack_origin, inx.value()));
			}
			while (right_attacks)
			{
				square inx = pop_lsb_nonzero(right_attacks);

				if (inx.rank() == last_rank) // promotion capture.
					add_promotions(static_cast<e_square>(inx.value() - right_attack_origin), inx.value(), ml);
				else
					ml->push_back(move(inx.value() - right_attack_origin, inx.value()));
			}

			// Promotions
			while (promotions)
			{
				e_square inx = (e_square)pop_lsb_nonzero(promotions);
				add_promotions(static_cast<e_square>(S == WHITE ? inx - 8 : inx + 8), inx, ml);
			}

			// En-passant
			auto en_passant_sq = pos->game_state()->en_passant_sq.value();
			if (en_passant_sq != NO_SQ)
			{
				bitboard_t ep_board = 1ULL << en_passant_sq;

				if (shift<down, left>(ep_board) & pawns) // we use right attack origin since from the attacker's perspective it is a right-attack
					ml->push_back(move(en_passant_sq - right_attack_origin, en_passant_sq, ENPASSANT, KNIGHT));
				if (shift<down, right>(ep_board) & pawns)
					ml->push_back(move(en_passant_sq - left_attack_origin, en_passant_sq, ENPASSANT, KNIGHT));
			}
		}
	}

	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::generate_knight(const pos_t* pos, ml_t* ml) const
	{
		bitboard_t knightbb = pos->piece_bb(S, KNIGHT);
		if (knightbb == 0)
			return;

		while (knightbb != 0)
		{
			auto from_sq = static_cast<e_square>(pop_lsb_nonzero(knightbb));
			bitboard_t all_attacks = m_knight_attacks[from_sq] & ~pos->all_pieces(S);
			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::generate_bishop(const pos_t* pos, ml_t* ml) const
	{
		bitboard_t bishopbb = pos->piece_bb(S, BISHOP);
		if (bishopbb == 0)
			return;

		bitboard_t occupancy = pos->all_pieces();

		while (bishopbb != 0)
		{
			auto from_sq = static_cast<e_square>(pop_lsb_nonzero(bishopbb));
			bitboard_t all_attacks = m_bishop_index->attacks(from_sq, occupancy);
			all_attacks &= ~pos->all_pieces(S);

			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::generate_rook(const pos_t* pos, ml_t* ml) const
	{
		bitboard_t rookbb = pos->piece_bb(S, ROOK);
		if (rookbb == 0)
			return;

		bitboard_t occupancy = pos->all_pieces();

		while (rookbb != 0)
		{
			auto from_sq = static_cast<e_square>(pop_lsb_nonzero(rookbb));
			bitboard_t all_attacks = m_rook_index->attacks(from_sq, occupancy);
			all_attacks &= ~pos->all_pieces(S);

			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::generate_queen(const pos_t* pos, ml_t* ml) const
	{
		bitboard_t queenbb = pos->piece_bb(S, QUEEN);
		if (queenbb == 0)
			return;
		bitboard_t occupancy = pos->all_pieces();

		while (queenbb != 0)
		{
			auto from_sq = static_cast<e_square>(pop_lsb_nonzero(queenbb));
			bitboard_t all_attacks =
				m_rook_index->attacks(from_sq, occupancy) | m_bishop_index->attacks(from_sq, occupancy);
			all_attacks &= ~pos->all_pieces(S);

			add_basic_attacks<S, MT>(all_attacks, from_sq, pos, ml);
		}
	}

	template<position_view pos_t>
	template<side S, move_type MT>
	void move_generator<pos_t>::generate_king(const pos_t* pos, ml_t* ml) const
	{
		assert(pos->king_square(S) == scan_lsb(pos->piece_bb(S, KING)).value());
		constexpr auto first_rank = S == WHITE ? RANK_1 : RANK_8;

		auto king_sq = pos->king_square(S);
		bitboard_t attacks = m_king_attacks[king_sq] & ~pos->all_pieces(S);
		bitboard_t occupancy = pos->all_pieces();
		bitboard_t valid_attacks = 0;

		if constexpr (MT == ACTIVE || MT == ALL)
			valid_attacks |= attacks & pos->all_pieces(!S);
		if constexpr (MT == QUIET || MT == ALL)
		{
			valid_attacks |= attacks & ~pos->all_pieces(!S);

			// castling
			if (pos->game_state()->castling_rights.can_castle<S, KINGSIDE>() && castle_is_safe<S, KINGSIDE>(pos))
			{
				auto f_sq = square(first_rank, FILE_F);
				auto g_sq = square(first_rank, FILE_G);

				if (!is_one_at(occupancy, f_sq.value()) &&
					!is_one_at(occupancy, g_sq.value()))
				{
					ml->push_back(move(king_sq, S == WHITE ? G1 : G8, CASTLING, KNIGHT));
				}
			}
			if (pos->game_state()->castling_rights.can_castle<S, QUEENSIDE>() && castle_is_safe<S, QUEENSIDE>(pos))
			{
				auto b_sq = square(first_rank, FILE_B);
				auto c_sq = square(first_rank, FILE_C);
				auto d_sq = square(first_rank, FILE_D);
				if (!is_one_at(occupancy, b_sq.value()) &&
					!is_one_at(occupancy, c_sq.value()) &&
					!is_one_at(occupancy, d_sq.value()))
				{
					ml->push_back(move(king_sq, S == WHITE ? C1 : C8, CASTLING, KNIGHT));
				}
			}
		}
		while (valid_attacks != 0)
		{
			auto to_sq = pop_lsb_nonzero(valid_attacks);
			ml->push_back(move(king_sq, to_sq));
		}
	}

	template<position_view pos_t>
	template<side S, position::castling_direction D>
	bool move_generator<pos_t>::castle_is_safe(const pos_t* pos) const
	{
		auto king_sq = pos->king_square(S);

		if (square_attacked<!S>(pos, king_sq))
			return false;

		if constexpr (D == KINGSIDE)
		{
			auto f_sq = square(S == WHITE ? RANK_1 : RANK_8, FILE_F);
			auto g_sq = square(S == WHITE ? RANK_1 : RANK_8, FILE_G);
			if (square_attacked<!S>(pos, f_sq.value()) || square_attacked<!S>(pos, g_sq.value()))
				return false;
		}
		else if constexpr (D == QUEENSIDE)
		{
			auto c_sq = square(S == WHITE ? RANK_1 : RANK_8, FILE_C);
			auto d_sq = square(S == WHITE ? RANK_1 : RANK_8, FILE_D);
			if (square_attacked<!S>(pos, c_sq.value()) || square_attacked<!S>(pos, d_sq.value()))
				return false;
		}
		return true;
	}

	template<position_view pos_t>
	template<side S> 
	bool move_generator<pos_t>::square_attacked(const pos_t* pos, position::e_square sq) const
	{
		if (pawn_attackers(pos, sq, S) != 0)
			return true;
		if (knight_attackers(pos, sq, S) != 0)
			return true;
		if (bishop_attackers(pos, sq, S) != 0)
			return true;
		if (rook_attackers(pos, sq, S) != 0)
			return true;
		if (queen_attackers(pos, sq, S) != 0)
			return true;
		if (king_attackers(pos, sq, S) != 0)
			return true;

		return false;

	}


#pragma endregion

#pragma region attackers implementations
	template<position_view pos_t>
	bitboard_t move_generator<pos_t>::pawn_attackers(const pos_t* pos, e_square sq, side s) const
	{
		bitboard_t sq_bb = set_one_at(0ULL, sq);
		bitboard_t attackers = pos->piece_bb(s, PAWN);
		bitboard_t attacks = s == BLACK ?
			shift<UP, LEFT>(sq_bb) | shift<UP, RIGHT>(sq_bb) :
			shift<DOWN, LEFT>(sq_bb) | shift<DOWN, RIGHT>(sq_bb);
		return attacks & attackers;
	}

	template<position_view pos_t>
	bitboard_t move_generator<pos_t>::knight_attackers(const pos_t* pos, e_square sq, side s) const
	{
		auto knights = pos->piece_bb(s, KNIGHT);
		return m_knight_attacks[sq] & knights;
	}

	template<position_view pos_t>
	bitboard_t move_generator<pos_t>::bishop_attackers(const pos_t* pos, e_square sq, side s) const
	{
		auto bishops = pos->piece_bb(s, BISHOP);
		return m_bishop_index->attacks(sq, pos->all_pieces()) & bishops;
	}

	template<position_view pos_t>
	bitboard_t move_generator<pos_t>::rook_attackers(const pos_t* pos, e_square sq, side s) const
	{
		auto rooks = pos->piece_bb(s, ROOK);
		return m_rook_index->attacks(sq, pos->all_pieces()) & rooks;
	}

	template<position_view pos_t>
	bitboard_t move_generator<pos_t>::queen_attackers(const pos_t* pos, e_square sq, side s) const
	{
		auto queens = pos->piece_bb(s, QUEEN);
		auto occ = pos->all_pieces();
		auto attacks = m_rook_index->attacks(sq, occ) | m_bishop_index->attacks(sq, occ);
		return attacks & queens;
	}

	template<position_view pos_t>
	bitboard_t move_generator<pos_t>::king_attackers(const pos_t* pos, e_square sq, side s) const
	{
		auto king = set_one_at(0ULL, pos->king_square(s));
		return m_king_attacks[sq] & king;
	}
#pragma endregion
}