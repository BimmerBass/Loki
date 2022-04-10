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
#include "loki.pch.h"


namespace loki::movegen {

	move_generator::attack_table_t	move_generator::knight_attacks{ 0 };
	move_generator::attack_table_t	move_generator::king_attacks{ 0 };

	move_generator::move_generator(position::position_t pos, magics::slider_generator_t slider_generator) noexcept :
		m_position(pos),
		m_slider_generator(slider_generator),
		m_moves(nullptr) {

		static std::mutex mtx;
		static std::atomic_bool tables_initialized = false;
		{
			std::lock_guard<std::mutex> lock(mtx);
			if (!tables_initialized) {
				init_knight_attacks();
				init_king_attacks();

				tables_initialized = true;
			}
		}
	}

	/// <summary>
	/// Generate a list of pseudo-legal moves in the position.
	/// _Ty: The type of moves (ACTIVES: Generally moves that change the material situation, QUIETS: Moves that does not change material on the board, ALL: All pseudo-legal moves)
	/// _Si: The side to generate the moves for (WHITE: Generate white's pseudo-legal moves, BLACK: Generate black's pseudo-legal moves, SIDE_NB: Generate pseudo-legal moves for the side to move)
	/// </summary>
	/// <returns></returns>
	template<MOVE_TYPE _Ty, SIDE _Si>
	const move_list_t& move_generator::generate() {
		// Make the generator ready to generate moves.
		m_moves = &m_movelists[m_position->m_ply];
		m_moves->clear();

		// FIXME: Ugly implementation...
		if constexpr (_Si == SIDE_NB) {
			switch (m_position->m_state_info->side_to_move) {
			case WHITE:
				get_pawn_moves<WHITE, _Ty>();
				get_knight_moves<WHITE, _Ty>();
				get_bishop_moves<WHITE, _Ty>();
				get_rook_moves<WHITE, _Ty>();
				get_queen_moves<WHITE, _Ty>();
				get_king_moves<WHITE, _Ty>();
				break;
			case BLACK:
				get_pawn_moves<BLACK, _Ty>();
				get_knight_moves<BLACK, _Ty>();
				get_bishop_moves<BLACK, _Ty>();
				get_rook_moves<BLACK, _Ty>();
				get_queen_moves<BLACK, _Ty>();
				get_king_moves<BLACK, _Ty>();
				break;
			default:
				throw std::runtime_error("An illegal value for the side to generate moves for were passed.");
			}
		}
		else {
			get_pawn_moves<_Si, _Ty>();
			get_knight_moves<_Si, _Ty>();
			get_bishop_moves<_Si, _Ty>();
			get_rook_moves<_Si, _Ty>();
			get_queen_moves<_Si, _Ty>();
			get_king_moves<_Si, _Ty>();
		}

		return *m_moves;
	}

	/// <summary>
	/// Get attackers of some piece type to a given square from a certain side.
	/// </summary>
	/// <param name="sq"></param>
	/// <returns></returns>
	template<SIDE _Si, PIECE _Pce>
	bitboard_t move_generator::attackers_to(SQUARE sq) const noexcept {
		constexpr DIRECTION up_left		= (_Si == BLACK) ? NORTHWEST : SOUTHWEST;
		constexpr DIRECTION up_right	= (_Si == BLACK) ? NORTHEAST : SOUTHEAST;

		bitboard_t occupancy = m_position->m_all_pieces[WHITE] | m_position->m_all_pieces[BLACK];
		bitboard_t attacks = 0;

		bitboard_t sq_bb;
		switch (_Pce) {
		case PAWN: 
			sq_bb = bitboard_t(1) << sq;
			attacks = shift<up_left>(sq_bb) | shift<up_right>(sq_bb);
			break;
		case KNIGHT: attacks = knight_attacks[sq]; break;
		case BISHOP: attacks = m_slider_generator->bishop_attacks(sq, occupancy); break;
		case ROOK: attacks = m_slider_generator->rook_attacks(sq, occupancy); break;
		case QUEEN: attacks = m_slider_generator->queen_attacks(sq, occupancy); break;
		case KING: attacks = king_attacks[sq]; break;
		}

		return attacks & m_position->m_state_info->piece_placements[_Si][_Pce];
	}


	/// <summary>
	/// Get all attackers to a given square from a side.
	/// </summary>
	/// <param name="sq"></param>
	/// <returns></returns>
	template<SIDE _Si>
	bitboard_t move_generator::all_attackers_to(SQUARE sq) const noexcept {
		return (
			attackers_to<_Si, PAWN>(sq) |
			attackers_to<_Si, KNIGHT>(sq) |
			attackers_to<_Si, BISHOP>(sq) |
			attackers_to<_Si, ROOK>(sq) |
			attackers_to<_Si, QUEEN>(sq) |
			attackers_to<_Si, KING>(sq));
	}

#pragma region Piece specific move generation
	/// <summary>
	/// Generate pawn moves.
	/// TODO: Perhaps it would be a good idea to have a pre-calculated table of possible pawn moves..?
	/// </summary>
	template<SIDE _S, MOVE_TYPE _Ty>
	void move_generator::get_pawn_moves() {

		constexpr DIRECTION up				= (_S == WHITE) ? NORTH : SOUTH;
		constexpr DIRECTION up_left			= (_S == WHITE) ? NORTHWEST : SOUTHWEST;
		constexpr DIRECTION down_left		= (_S == WHITE) ? SOUTHWEST : NORTHWEST;
		constexpr DIRECTION up_right		= (_S == WHITE) ? NORTHEAST : SOUTHEAST;
		constexpr DIRECTION down_right		= (_S == WHITE) ? SOUTHEAST : NORTHEAST;
		constexpr bitboard_t relative_r3	= bitmasks::rank_masks[_S == WHITE ? RANK_3 : RANK_6];
		constexpr bitboard_t relative_r8	= bitmasks::rank_masks[_S == WHITE ? RANK_8 : RANK_1];
		constexpr RANK relative_top_rank	= _S == WHITE ? RANK_8 : RANK_1;
		constexpr int left_attack_origin	= (_S == WHITE) ? 7 : -9;
		constexpr int right_attack_origin	= (_S == WHITE) ? 9 : -7;		
		constexpr int left_ep_origin		= (_S == WHITE) ? 9 : -7;
		constexpr int right_ep_origin		= (_S == WHITE) ? 7 : -9;
		

		bitboard_t pawns			= m_position->m_state_info->piece_placements[_S][PAWN];
		bitboard_t opponent_pieces	= m_position->m_all_pieces[!_S];
		bitboard_t occupied			= m_position->m_all_pieces[WHITE] | m_position->m_all_pieces[BLACK];

		if (pawns == 0)
			return;

		size_t idx;
		if constexpr (_Ty == QUIET) {
			bitboard_t one_up = shift<up>(pawns) & ~(occupied | relative_r8); // No promotions here.
			bitboard_t two_up = shift<up>(one_up & relative_r3) & ~occupied;

			while (one_up) {
				idx = pop_bit(one_up);

				m_moves->add(_S == WHITE ? idx - 8 : idx + 8, idx, NOT_SPECIAL, 0, 0);
			}
			while (two_up) {
				idx = pop_bit(two_up);
				m_moves->add(_S == WHITE ? idx - 16 : idx + 16, idx, NOT_SPECIAL, 0, 0);
			}
		}
		else if constexpr (_Ty == ACTIVES) {
			// 1: Attacks.
			bitboard_t left_attacks		= shift<up_left>(pawns) & opponent_pieces;
			bitboard_t right_attacks	= shift<up_right>(pawns) & opponent_pieces;
			bitboard_t promotions		= shift<up>(pawns) & relative_r8 & ~occupied;

			while (left_attacks) {
				idx = pop_bit(left_attacks);

				if (rank(idx) == relative_top_rank) { // Promotion capture.
					m_moves->add(idx - left_attack_origin, idx, PROMOTION, KNIGHT - 1, 0);
					m_moves->add(idx - left_attack_origin, idx, PROMOTION, BISHOP - 1, 0);
					m_moves->add(idx - left_attack_origin, idx, PROMOTION, ROOK - 1, 0);
					m_moves->add(idx - left_attack_origin, idx, PROMOTION, QUEEN - 1, 0);
				}
				else { // Regular capture.
					m_moves->add(idx - left_attack_origin, idx, NOT_SPECIAL, 0, 0);
				}
			}
			while (right_attacks) {
				idx = pop_bit(right_attacks);

				if (rank(idx) == relative_top_rank) { // Promotion capture.
					m_moves->add(idx - right_attack_origin, idx, PROMOTION, KNIGHT - 1, 0);
					m_moves->add(idx - right_attack_origin, idx, PROMOTION, BISHOP - 1, 0);
					m_moves->add(idx - right_attack_origin, idx, PROMOTION, ROOK - 1, 0);
					m_moves->add(idx - right_attack_origin, idx, PROMOTION, QUEEN - 1, 0);
				}
				else { // Regular capture.
					m_moves->add(idx - right_attack_origin, idx, NOT_SPECIAL, 0, 0);
				}
			}

			// 2. Regular promotions.
			while (promotions) {
				idx = pop_bit(promotions);

				m_moves->add(_S == WHITE ? idx - 8 : idx + 8, idx, PROMOTION, KNIGHT - 1, 0);
				m_moves->add(_S == WHITE ? idx - 8 : idx + 8, idx, PROMOTION, BISHOP - 1, 0);
				m_moves->add(_S == WHITE ? idx - 8 : idx + 8, idx, PROMOTION, ROOK - 1, 0);
				m_moves->add(_S == WHITE ? idx - 8 : idx + 8, idx, PROMOTION, QUEEN - 1, 0);
			}

			// 3. En-passant.
			if (m_position->m_state_info->en_passant_square != NO_SQ) {
				auto ep_square		= m_position->m_state_info->en_passant_square;
				bitboard_t ep_board = bitboard_t(1) << ep_square;

				if ((shift<down_left>(ep_board) & pawns) != 0) {
					m_moves->add(ep_square - left_ep_origin, ep_square, ENPASSANT, 0, 0);
				}
				if ((shift<down_right>(ep_board) & pawns) != 0) {
					m_moves->add(ep_square - right_ep_origin, ep_square, ENPASSANT, 0, 0);
				}
			}
		}
		else {
			// Call this function for active moves as well as quiets.
			get_pawn_moves<_S, QUIET>();
			get_pawn_moves<_S, ACTIVES>();
		}
	}

	/// <summary>
	/// Generate knight moves.
	/// </summary>
	template<SIDE _S, MOVE_TYPE _Ty>
	void move_generator::get_knight_moves() {
		constexpr bitboard_t full_board = ~bitboard_t(0);
		bitboard_t friendly_pieces		= m_position->m_all_pieces[_S];
		bitboard_t opponent_pieces		= m_position->m_all_pieces[!_S];
		bitboard_t knight_board			= m_position->m_state_info->piece_placements[_S][KNIGHT];
		
		if (knight_board == 0)
			return;

		while (knight_board) {
			auto idx = pop_bit(knight_board);
			bitboard_t attacks = 
				knight_attacks[idx] & 
				(_Ty == ACTIVES ? opponent_pieces : full_board) & 
				(_Ty == QUIET ? ~opponent_pieces : full_board) & 
				~friendly_pieces;

			while (attacks) {
				auto to_sq = pop_bit(attacks);
				m_moves->add(idx, to_sq, NOT_SPECIAL, 0, 0);
			}
		}
	}

	template<SIDE _S, MOVE_TYPE _Ty>
	void move_generator::get_bishop_moves() {
		constexpr bitboard_t full_board = ~bitboard_t(0);
		bitboard_t friendly_pieces	= m_position->m_all_pieces[_S];
		bitboard_t opponent_pieces	= m_position->m_all_pieces[!_S];
		bitboard_t bishops			= m_position->m_state_info->piece_placements[_S][BISHOP];

		while (bishops) {
			auto idx = pop_bit(bishops);
			bitboard_t attacks =
				m_slider_generator->bishop_attacks(idx, friendly_pieces | opponent_pieces) &
				(_Ty == ACTIVES ? opponent_pieces : full_board) &
				(_Ty == QUIET ? ~opponent_pieces : full_board) &
				~friendly_pieces;
			
			while (attacks) {
				auto to_sq = pop_bit(attacks);
				m_moves->add(idx, to_sq, NOT_SPECIAL, 0, 0);
			}
		}
	}

	template<SIDE _S, MOVE_TYPE _Ty>
	void move_generator::get_rook_moves() {
		constexpr bitboard_t full_board = ~bitboard_t(0);
		bitboard_t friendly_pieces	= m_position->m_all_pieces[_S];
		bitboard_t opponent_pieces	= m_position->m_all_pieces[!_S];
		bitboard_t rooks			= m_position->m_state_info->piece_placements[_S][ROOK];

		while (rooks) {
			auto idx = pop_bit(rooks);
			bitboard_t attacks =
				m_slider_generator->rook_attacks(idx, friendly_pieces | opponent_pieces) &
				(_Ty == ACTIVES ? opponent_pieces : full_board) &
				(_Ty == QUIET ? ~opponent_pieces : full_board) &
				~friendly_pieces;

			while (attacks) {
				auto to_sq = pop_bit(attacks);
				m_moves->add(idx, to_sq, NOT_SPECIAL, 0, 0);
			}
		}
	}

	template<SIDE _S, MOVE_TYPE _Ty>
	void move_generator::get_queen_moves() {
		constexpr bitboard_t full_board = ~bitboard_t(0);
		bitboard_t friendly_pieces	= m_position->m_all_pieces[_S];
		bitboard_t opponent_pieces	= m_position->m_all_pieces[!_S];
		bitboard_t queens			= m_position->m_state_info->piece_placements[_S][QUEEN];

		while (queens) {
			auto idx = pop_bit(queens);
			bitboard_t attacks =
				m_slider_generator->queen_attacks(idx, friendly_pieces | opponent_pieces) &
				(_Ty == ACTIVES ? opponent_pieces : full_board) &
				(_Ty == QUIET ? ~opponent_pieces : full_board) &
				~friendly_pieces;

			while (attacks) {
				auto to_sq = pop_bit(attacks);
				m_moves->add(idx, to_sq, NOT_SPECIAL, 0, 0);
			}
		}
	}

	template<SIDE _S, MOVE_TYPE _Ty>
	void move_generator::get_king_moves() {
		constexpr SQUARE kingside_castling_dest		= _S == WHITE ? G1 : G8;
		constexpr SQUARE queenside_castling_dest	= _S == WHITE ? C1 : C8;
		SQUARE king_sq								= m_position->m_king_squares[_S];
		bitboard_t friendly_pieces					= m_position->m_all_pieces[_S];
		bitboard_t opponent_pieces					= m_position->m_all_pieces[!_S];

		if constexpr (_Ty == ACTIVES) {
			bitboard_t attacks = king_attacks[king_sq] & ~friendly_pieces & opponent_pieces;

			while (attacks) {
				auto idx = pop_bit(attacks);
				m_moves->add(king_sq, idx, NOT_SPECIAL, 0, 0);
			}
		}
		else if constexpr (_Ty == QUIET) {
			bitboard_t attacks = king_attacks[king_sq] & ~(friendly_pieces | opponent_pieces);

			while (attacks) {
				auto idx = pop_bit(attacks);
				m_moves->add(king_sq, idx, NOT_SPECIAL, 0, 0);
			}

			// FIXME: Should castling be considered a QUIET or an ACTIVE move?
			if (can_castle<_S, _S == WHITE ? WKCA : BKCA>()) {
				m_moves->add(king_sq, kingside_castling_dest, CASTLE, 0, 0);
			}
			if (can_castle<_S, _S == WHITE ? WQCA : BQCA>()) {
				m_moves->add(king_sq, queenside_castling_dest, CASTLE, 0, 0);
			}
		}
		else {
			get_king_moves<_S, QUIET>();
			get_king_moves<_S, ACTIVES>();
		}
	}
#pragma endregion

	/// <summary>
	/// Initialize the table of attacks for knights.
	/// </summary>
	/// <returns></returns>
	void move_generator::init_knight_attacks() noexcept {

		for (auto sq = 0; sq < 64; sq++) {
			bitboard_t current_attack_mask = 0;

			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_H]) << 17;
			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_A]) << 15;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_G] | bitmasks::file_masks[FILE_H])) << 10;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_G] | bitmasks::file_masks[FILE_H])) >> 6;
			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_H]) >> 15;
			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_A]) >> 17;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_A] | bitmasks::file_masks[FILE_B])) << 6;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_A] | bitmasks::file_masks[FILE_B])) >> 10;

			knight_attacks[sq] = current_attack_mask;
		}
	}

	/// <summary>
	/// Initialize the table of king attacks.
	/// </summary>
	/// <returns></returns>
	void move_generator::init_king_attacks() noexcept {
		for (auto sq = 0; sq < 64; sq++) {
			bitboard_t current_attack_mask = 0;
			bitboard_t sq_board = uint64_t(1) << sq;

			current_attack_mask |= (sq_board & ~bitmasks::rank_masks[RANK_8]) << 8;
			current_attack_mask |= (sq_board & ~bitmasks::rank_masks[RANK_1]) >> 8;
			current_attack_mask |= (sq_board & ~bitmasks::file_masks[FILE_H]) << 1;
			current_attack_mask |= (sq_board & ~bitmasks::file_masks[FILE_A]) >> 1;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_8] | bitmasks::file_masks[FILE_A])) << 7;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_8] | bitmasks::file_masks[FILE_H])) << 9;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_1] | bitmasks::file_masks[FILE_A])) >> 9;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_1] | bitmasks::file_masks[FILE_H])) >> 7;


			king_attacks[sq] = current_attack_mask;
		}
	}

	// Note: For the move-constructors, we don't need to check for table initialization since it is guaranteed to have been done before the move ctor was called.
	move_generator::move_generator(move_generator&& _src) noexcept
		:	m_position(std::move(_src.m_position)),
			m_slider_generator(std::move(_src.m_slider_generator)),
			m_movelists(_src.m_movelists),
			m_moves(m_movelists.data() + (_src.m_moves - _src.m_movelists.data())) {
	}

	move_generator& move_generator::operator=(move_generator&& _src) noexcept {
		if (this != &_src) {
			m_position = position::position_t(std::move(_src.m_position));
			m_slider_generator = magics::slider_generator_t(std::move(_src.m_slider_generator));

			// move_list is just a wrapped static array, so we can only copy it..
			m_movelists = _src.m_movelists;
			m_moves = m_movelists.data() + (_src.m_moves - _src.m_movelists.data());
		}
		return *this;
	}

#pragma region Explicit template instantiations
	template void move_generator::get_pawn_moves<WHITE, ACTIVES>();
	template void move_generator::get_pawn_moves<WHITE, QUIET>();
	template void move_generator::get_pawn_moves<WHITE, ALL>();
	template void move_generator::get_pawn_moves<BLACK, ACTIVES>();
	template void move_generator::get_pawn_moves<BLACK, QUIET>();
	template void move_generator::get_pawn_moves<BLACK, ALL>();

	template void move_generator::get_knight_moves<WHITE, ACTIVES>();
	template void move_generator::get_knight_moves<WHITE, QUIET>();
	template void move_generator::get_knight_moves<WHITE, ALL>();
	template void move_generator::get_knight_moves<BLACK, ACTIVES>();
	template void move_generator::get_knight_moves<BLACK, QUIET>();
	template void move_generator::get_knight_moves<BLACK, ALL>();

	template void move_generator::get_bishop_moves<WHITE, ACTIVES>();
	template void move_generator::get_bishop_moves<WHITE, QUIET>();
	template void move_generator::get_bishop_moves<WHITE, ALL>();
	template void move_generator::get_bishop_moves<BLACK, ACTIVES>();
	template void move_generator::get_bishop_moves<BLACK, QUIET>();
	template void move_generator::get_bishop_moves<BLACK, ALL>();

	template void move_generator::get_rook_moves<WHITE, ACTIVES>();
	template void move_generator::get_rook_moves<WHITE, QUIET>();
	template void move_generator::get_rook_moves<WHITE, ALL>();
	template void move_generator::get_rook_moves<BLACK, ACTIVES>();
	template void move_generator::get_rook_moves<BLACK, QUIET>();
	template void move_generator::get_rook_moves<BLACK, ALL>();

	template void move_generator::get_queen_moves<WHITE, ACTIVES>();
	template void move_generator::get_queen_moves<WHITE, QUIET>();
	template void move_generator::get_queen_moves<WHITE, ALL>();
	template void move_generator::get_queen_moves<BLACK, ACTIVES>();
	template void move_generator::get_queen_moves<BLACK, QUIET>();
	template void move_generator::get_queen_moves<BLACK, ALL>();

	template void move_generator::get_king_moves<WHITE, ACTIVES>();
	template void move_generator::get_king_moves<WHITE, QUIET>();
	template void move_generator::get_king_moves<WHITE, ALL>();
	template void move_generator::get_king_moves<BLACK, ACTIVES>();
	template void move_generator::get_king_moves<BLACK, QUIET>();
	template void move_generator::get_king_moves<BLACK, ALL>();

	template const move_list_t& move_generator::generate<ACTIVES, WHITE>();
	template const move_list_t& move_generator::generate<QUIET, WHITE>();
	template const move_list_t& move_generator::generate<ALL, WHITE>();
	template const move_list_t& move_generator::generate<ACTIVES, BLACK>();
	template const move_list_t& move_generator::generate<QUIET, BLACK>();
	template const move_list_t& move_generator::generate<ALL, BLACK>();
	template const move_list_t& move_generator::generate<ACTIVES, SIDE_NB>();
	template const move_list_t& move_generator::generate<QUIET, SIDE_NB>();
	template const move_list_t& move_generator::generate<ALL, SIDE_NB>();

	template bitboard_t move_generator::attackers_to<WHITE, PAWN>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<WHITE, KNIGHT>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<WHITE, BISHOP>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<WHITE, ROOK>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<WHITE, QUEEN>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<WHITE, KING>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<BLACK, PAWN>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<BLACK, KNIGHT>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<BLACK, BISHOP>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<BLACK, ROOK>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<BLACK, QUEEN>(SQUARE) const noexcept;
	template bitboard_t move_generator::attackers_to<BLACK, KING>(SQUARE) const noexcept;

	template bitboard_t move_generator::all_attackers_to<WHITE>(SQUARE) const noexcept;
	template bitboard_t move_generator::all_attackers_to<BLACK>(SQUARE) const noexcept;

#pragma endregion
}