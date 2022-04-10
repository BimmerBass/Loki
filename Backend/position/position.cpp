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

namespace loki::position {



	/// <summary>
	/// Create a position object. Since we need to set the m_self pointer, we need to have a factory method to do this.
	/// Move generator is also created here
	/// </summary>
	/// <param name="internal_state"></param>
	/// <param name="magic_index"></param>
	/// <returns></returns>
	position_t position::create_position(game_state_t internal_state, movegen::magics::slider_generator_t magic_index) {
		auto pos = position_t(new position(internal_state, magic_index));
		pos->m_self = pos;
		pos->m_generator = std::make_unique<movegen::move_generator>(pos, magic_index);

		return pos;
	}

	position::position(game_state_t internal_state, movegen::magics::slider_generator_t magics_index) {
		m_move_history = std::make_unique<movegen::move_stack<MAX_GAME_MOVES>>();
		m_state_info = internal_state;
		reload();
	}

	/// <summary>
	/// Generate moves of the given type and side.
	/// </summary>
	/// <returns></returns>
	template<movegen::MOVE_TYPE _Ty>
	const movegen::move_list_t& position::generate_moves() {
		return m_state_info->side_to_move == WHITE ?
			m_generator->generate<_Ty, WHITE>() :
			m_generator->generate<_Ty, BLACK>();
	}

	/// <summary>
	/// Make a move on the board.
	/// </summary>
	/// <param name="move"></param>
	/// <returns>Whether or not the move is legal (if not, the move is unmade)</returns>
	bool position::make_move(move_t move) {
		auto me					= m_state_info->side_to_move;
		auto them				= !me;
		size_t origin			= movegen::from_sq(move);
		size_t destination		= movegen::to_sq(move);
		auto special_props		= movegen::special(move);
		auto promotion_piece	= static_cast<PIECE>(static_cast<int64_t>(movegen::promotion_piece(move)) + 1);
		auto piece_moved		= m_piece_list[m_state_info->side_to_move][origin];
		auto piece_captured		= m_piece_list[them][destination];

		// debug assertations.
		assert(origin != destination);
		assert(origin >= A1 && origin <= H8);
		assert(destination >= A1 && destination <= H8);
		assert(special_props >= movegen::PROMOTION && special_props <= movegen::NOT_SPECIAL);
		assert(promotion_piece >= KNIGHT && promotion_piece <= QUEEN);
		assert(piece_moved >= PAWN && piece_moved < PIECE_NB);
		assert(piece_captured >= PAWN && piece_captured <= PIECE_NB);


		if (piece_captured == KING) {
			return false;
		}

		// irreversible data.
		m_move_history->insert(move, 
			std::make_tuple(
				piece_captured,
				piece_moved, 
				m_state_info->castling_rights.get(), 
				m_state_info->fifty_move_counter, 
				m_state_info->en_passant_square));

		auto add_to_destination = [&]() {
			m_piece_list[me][destination] = piece_moved;
			m_state_info->piece_placements[me][piece_moved] = set_bit(m_state_info->piece_placements[me][piece_moved], destination);

			if (piece_moved == KING)
				m_king_squares[me] = static_cast<SQUARE>(destination);
		};

		// handle moved piece
		m_state_info->piece_placements[me][piece_moved] = toggle_bit(m_state_info->piece_placements[me][piece_moved], origin);
		m_piece_list[me][origin] = PIECE_NB;

		size_t en_pas_cap;
		switch (special_props) {
		case movegen::PROMOTION: /* Add promotion piece instead of moved piece. */
			m_piece_list[me][destination] = promotion_piece;
			m_state_info->piece_placements[me][promotion_piece] = set_bit(m_state_info->piece_placements[me][promotion_piece], destination);
			break;
		
		case movegen::CASTLE: /* Move the rook. */
			perform_rook_castle(origin, destination);
			add_to_destination();
			break;
		
		case movegen::ENPASSANT: /* Remove captured pawn and fall-through */
			assert(piece_moved == PAWN);
			en_pas_cap = (me == WHITE) ? m_state_info->en_passant_square - 8 : m_state_info->en_passant_square + 8;
			m_piece_list[them][en_pas_cap] = PIECE_NB;
			m_state_info->piece_placements[them][PAWN] = toggle_bit(m_state_info->piece_placements[them][PAWN], en_pas_cap);
			[[fallthrough]];
		
		case movegen::NOT_SPECIAL: /* Add moved piece to destination. */
			add_to_destination();
			break;
		}

		// handle captured piece.
		if (piece_captured != PIECE_NB) {
			m_piece_list[them][destination] = PIECE_NB;
			m_state_info->piece_placements[them][piece_captured] = toggle_bit(m_state_info->piece_placements[them][piece_captured], destination);
		}

		// handle new castling rights.
		if (piece_moved == KING) {
			m_state_info->castling_rights -= me == WHITE ? WKCA : BKCA;
			m_state_info->castling_rights -= me == WHITE ? WQCA : BQCA;
		}
		// If a piece has been moved to or from a corner square, the castling rights for that corner is automatically removed.
		if (origin == A1 || destination == A1)
			m_state_info->castling_rights -= WQCA;
		if (origin == H1 || destination == H1)
			m_state_info->castling_rights -= WKCA;
		if (origin == A8 || destination == A8)
			m_state_info->castling_rights -= BQCA;
		if (origin == H8 || destination == H8)
			m_state_info->castling_rights -= BKCA;

		// handle new en-passant square.
		m_state_info->en_passant_square = NO_SQ;

		if (piece_moved == PAWN && 
			((me == WHITE && destination == origin + 16) || (me == BLACK && destination == origin - 16)))
			m_state_info->en_passant_square = static_cast<SQUARE>(me == WHITE ? destination - 8 : destination + 8);

		// misc data updates.
		m_ply++;
		m_state_info->fifty_move_counter++;
		m_state_info->full_move_counter += me == BLACK ? 1 : 0;
		update_occupancies();

		// if we're in check, undo the move since it was illegal
		if (in_check()) {
			m_state_info->side_to_move = !me; // will be toggled by undo_move
			undo_move();
			return false;
		}
		m_state_info->side_to_move = !me;
		return true;
	}

	/// <summary>
	/// Take back the latest move.
	/// </summary>
	/// <returns></returns>
	void position::undo_move() {
		const auto& move_info	= m_move_history->pop();
		size_t origin			= movegen::from_sq(move_info.first);
		size_t destination		= movegen::to_sq(move_info.first);
		auto special_props		= movegen::special(move_info.first);
		auto promotion_piece	= static_cast<PIECE>(static_cast<int64_t>(movegen::promotion_piece(move_info.first)) + 1);
		auto lost_info			= move_info.second;
		
		// toggle side to move.
		m_state_info->side_to_move	= !m_state_info->side_to_move;
		auto me						= m_state_info->side_to_move;
		auto them					= !me;

		// handle piece moved
		if (special_props != movegen::PROMOTION) {
			m_piece_list[me][destination] = PIECE_NB;
			m_state_info->piece_placements[me][lost_info.piece_moved] = toggle_bit(m_state_info->piece_placements[me][lost_info.piece_moved], destination);
		}
		else {
			m_piece_list[me][destination] = PIECE_NB;
			m_state_info->piece_placements[me][promotion_piece] = toggle_bit(m_state_info->piece_placements[me][promotion_piece], destination);
		}

		m_piece_list[me][origin] = lost_info.piece_moved;
		m_state_info->piece_placements[me][lost_info.piece_moved] = set_bit(m_state_info->piece_placements[me][lost_info.piece_moved], origin);

		if (lost_info.piece_moved == KING) {
			m_king_squares[me] = static_cast<SQUARE>(origin);
		}

		// handle special moves other than promotion.
		size_t pawn_sq;
		switch (special_props) {
		case movegen::ENPASSANT:	/* Place back the pawn captured. */
			pawn_sq = me == WHITE ? lost_info.en_passant_square - 8 : lost_info.en_passant_square + 8;
			m_piece_list[them][pawn_sq] = PAWN;
			m_state_info->piece_placements[them][PAWN] = set_bit(m_state_info->piece_placements[them][PAWN], pawn_sq);
			break;
		case movegen::CASTLE:	/* Move back the rook. */
			perform_rook_castle(origin, destination, true);
			break;
		default:
			break;
		}

		// handle captured piece.
		if (lost_info.piece_captured != PIECE_NB) {
			m_piece_list[them][destination] = lost_info.piece_captured;
			m_state_info->piece_placements[them][lost_info.piece_captured] = set_bit(m_state_info->piece_placements[them][lost_info.piece_captured], destination);
		}

		// handle lost information.
		update_occupancies();
		m_ply--;
		m_state_info->castling_rights.load(move_info.second.castling_rights);
		m_state_info->fifty_move_counter = move_info.second.fifty_moves_count;
		m_state_info->full_move_counter -= me == BLACK ? 1 : 0;
		m_state_info->en_passant_square = move_info.second.en_passant_square;
	}

	/// <summary>
	/// Move the rook during a castling move
	/// </summary>
	/// <param name="orig"></param>
	/// <param name="dest"></param>
	void position::perform_rook_castle(size_t orig, size_t dest, bool undo) noexcept {
		auto stm = m_state_info->side_to_move;
		bool is_kingside = dest > orig;
		auto castling_side = is_kingside ? (stm == WHITE ? WKCA : BKCA) : (stm == WHITE ? WQCA : BQCA);
		SQUARE rook_orig, rook_dest;

		switch (castling_side) {
		case WKCA:
			rook_orig = H1;
			rook_dest = F1;
			break;
		case WQCA:
			rook_orig = A1;
			rook_dest = D1;
			break;
		case BKCA:
			rook_orig = H8;
			rook_dest = F8;
			break;
		case BQCA:
			rook_orig = A8;
			rook_dest = D8;
			break;
		}

		if (!undo) {
			m_piece_list[stm][rook_orig] = PIECE_NB;
			m_state_info->piece_placements[stm][ROOK] ^= bitboard_t(1) << rook_orig;

			m_piece_list[stm][rook_dest] = ROOK;
			m_state_info->piece_placements[stm][ROOK] |= bitboard_t(1) << rook_dest;
		}
		else {
			m_piece_list[stm][rook_orig] = ROOK;
			m_state_info->piece_placements[stm][ROOK] |= bitboard_t(1) << rook_orig;

			m_piece_list[stm][rook_dest] = PIECE_NB;
			m_state_info->piece_placements[stm][ROOK] ^= bitboard_t(1) << rook_dest;
		}
	}

	/// <summary>
	/// Update the occupancy bitboards.
	/// </summary>
	void position::update_occupancies() {
		m_all_pieces[WHITE] = (
			m_state_info->piece_placements[WHITE][PAWN] |
			m_state_info->piece_placements[WHITE][KNIGHT] |
			m_state_info->piece_placements[WHITE][BISHOP] |
			m_state_info->piece_placements[WHITE][ROOK] |
			m_state_info->piece_placements[WHITE][QUEEN] |
			m_state_info->piece_placements[WHITE][KING]);
		m_all_pieces[BLACK] = (
			m_state_info->piece_placements[BLACK][PAWN] |
			m_state_info->piece_placements[BLACK][KNIGHT] |
			m_state_info->piece_placements[BLACK][BISHOP] |
			m_state_info->piece_placements[BLACK][ROOK] |
			m_state_info->piece_placements[BLACK][QUEEN] |
			m_state_info->piece_placements[BLACK][KING]);
	}

	/// <summary>
	/// Return whether or not we're in check.
	/// </summary>
	/// <returns></returns>
	bool position::in_check() const noexcept {
		return m_state_info->side_to_move == WHITE ?
			square_attacked<BLACK>(m_king_squares[WHITE]) :
			square_attacked<WHITE>(m_king_squares[BLACK]);
	}

	/// <summary>
	/// Check if a square is attacked by side _Si
	/// </summary>
	/// <param name="sq"></param>
	/// <returns></returns>
	template<SIDE _Si> requires (_Si == WHITE || _Si == BLACK)
	bool position::square_attacked(SQUARE sq) const noexcept {
		// Note: We go from king to pawns because they are a little (negligible) more expensive to look up.
		if (m_generator->attackers_to<_Si, KING>(sq) != 0) {
			return true;
		}
		if (m_generator->attackers_to<_Si, QUEEN>(sq) != 0) {
			return true;
		}
		if (m_generator->attackers_to<_Si, ROOK>(sq) != 0) {
			return true;
		}
		if (m_generator->attackers_to<_Si, BISHOP>(sq) != 0) {
			return true;
		}
		if (m_generator->attackers_to<_Si, KNIGHT>(sq) != 0) {
			return true;
		}
		if (m_generator->attackers_to<_Si, PAWN>(sq) != 0) {
			return true;
		}
		return false;
	}
	
	/// <summary>
	/// Reload our data from m_state_info
	/// </summary>
	void position::reload() {
		// Reset all data.
		std::fill(std::begin(m_piece_list[WHITE]), std::end(m_piece_list[WHITE]), PIECE_NB);
		std::fill(std::begin(m_piece_list[BLACK]), std::end(m_piece_list[BLACK]), PIECE_NB);
		m_all_pieces[WHITE] = 0;
		m_all_pieces[BLACK] = 0;
		m_king_squares[WHITE] = NO_SQ;
		m_king_squares[BLACK] = NO_SQ;
		m_ply = 0;
		m_move_history->clear();

		for (size_t pce = PAWN; pce <= KING; pce++) {
			size_t inx;
			bitboard_t whites = m_state_info->piece_placements[WHITE][pce];
			bitboard_t blacks = m_state_info->piece_placements[BLACK][pce];

			m_all_pieces[WHITE] |= whites;
			m_all_pieces[BLACK] |= blacks;

			while (whites) {
				inx = pop_bit(whites);
				m_piece_list[WHITE][inx] = static_cast<PIECE>(pce);

				if (pce == KING) {
					m_king_squares[WHITE] = static_cast<SQUARE>(inx);
				}
			}
			while (blacks) {
				inx = pop_bit(blacks);
				m_piece_list[BLACK][inx] = static_cast<PIECE>(pce);

				if (pce == KING) {
					m_king_squares[BLACK] = static_cast<SQUARE>(inx);
				}
			}
		}
	}

	/// <summary>
	/// Load a FEN.
	/// </summary>
	/// <param name="fen"></param>
	void position::operator<<(const std::string& fen) {
		// Load our game_state
		*m_state_info << fen;
		reload();
	}

	/// <summary>
	/// Generate a FEN.
	/// </summary>
	/// <param name="fen"></param>
	void position::operator>>(std::string& fen) const {
		*m_state_info >> fen;
	}

	/// <summary>
	/// Print the position.
	/// </summary>
	/// <param name="os"></param>
	/// <param name="pos"></param>
	/// <returns></returns>
	std::ostream& operator<<(std::ostream& os, const position& pos) {
		os << (*pos.m_state_info);
		return os;
	}
	std::ostream& operator<<(std::ostream& os, const position_t& pos) {
		os << (*pos->m_state_info);
		return os;
	}

#pragma region Explicit instantiations
	template bool position::square_attacked<WHITE>(SQUARE) const noexcept;
	template bool position::square_attacked<BLACK>(SQUARE) const noexcept;

	template const movegen::move_list_t& position::generate_moves<movegen::ACTIVES>();
	template const movegen::move_list_t& position::generate_moves<movegen::QUIET>();
	template const movegen::move_list_t& position::generate_moves<movegen::ALL>();
#pragma endregion
}