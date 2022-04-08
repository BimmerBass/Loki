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
	/// Make a move on the board.
	/// </summary>
	/// <param name="move"></param>
	/// <returns></returns>
	bool position::make_move(move_t move) noexcept {
		return false;
	}

	/// <summary>
	/// Take back the latest move.
	/// </summary>
	/// <returns></returns>
	void position::undo_move() noexcept {

	}

	/// <summary>
	/// Check if a square is attacked by side _Si
	/// </summary>
	/// <param name="sq"></param>
	/// <returns></returns>
	template<SIDE _Si>
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
	template<> bool position::square_attacked<WHITE>(SQUARE) const noexcept;
	template<> bool position::square_attacked<BLACK>(SQUARE) const noexcept;
#pragma endregion
}