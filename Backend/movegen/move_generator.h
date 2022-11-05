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
#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H


namespace loki::movegen
{

	class move_generator
	{
		EXCEPTION_CLASS(e_moveGenerator, e_lokiError);
	private:
		using attack_table_t = std::array<bitboard_t, 64>;

		position::position_t					m_position;

		magics::slider_generator_t				m_slider_generator;
		attack_table_t							knight_attacks;
		attack_table_t							king_attacks;
		std::array<move_list_t, MAX_GAME_MOVES>	m_movelists;
		move_list_t*							m_moves;
	public:
		// No default constructor since a reference to the position object is required.
		move_generator() = delete;
		move_generator(position::position_t pos, magics::slider_generator_t slider_generator) noexcept;

		// No copying, only moving allowed.
		move_generator(const move_generator&) = delete;
		move_generator& operator=(const move_generator&) = delete;

		move_generator(move_generator&& _src) noexcept;
		move_generator& operator=(move_generator&& _src) noexcept;

		template<MOVE_TYPE _Ty, SIDE _Si = SIDE_NB>
		const move_list_t& generate();

		template<SIDE _Si, PIECE _Pce>
		bitboard_t attackers_to(SQUARE sq) const noexcept;

		template<SIDE _Si>
		bitboard_t all_attackers_to(SQUARE sq) const noexcept;
	private:
		template<SIDE _S, MOVE_TYPE _Ty>
		void get_pawn_moves();
		template<SIDE _S, MOVE_TYPE _Ty>
		void get_knight_moves();
		template<SIDE _S, MOVE_TYPE _Ty>
		void get_bishop_moves();
		template<SIDE _S, MOVE_TYPE _Ty>
		void get_rook_moves();
		template<SIDE _S, MOVE_TYPE _Ty>
		void get_queen_moves();
		template<SIDE _S, MOVE_TYPE _Ty>
		void get_king_moves();

		// initialization methods.
		void init_knight_attacks() noexcept;
		void init_king_attacks() noexcept;

		template<SIDE _S, CASTLING_RIGHTS _Cr>
		inline bool can_castle() const
		{
			constexpr SQUARE relative_e1 = _S == WHITE ? E1 : E8;
			constexpr SQUARE relative_f1 = _S == WHITE ? F1 : F8;
			constexpr SQUARE relative_g1 = _S == WHITE ? G1 : G8;
			constexpr SQUARE relative_d1 = _S == WHITE ? D1 : D8;
			constexpr SQUARE relative_c1 = _S == WHITE ? C1 : C8;
			constexpr bitboard_t key_queenside_squares = bitmasks::rank_masks[_S == WHITE ? RANK_1 : RANK_8]
				& (bitmasks::file_masks[FILE_C] | bitmasks::file_masks[FILE_D] | bitmasks::file_masks[FILE_B]);
			constexpr bitboard_t key_kingside_squares = bitmasks::rank_masks[_S == WHITE ? RANK_1 : RANK_8] & (bitmasks::file_masks[FILE_G] | bitmasks::file_masks[FILE_F]);

			if (_Cr == WKCA || _Cr == BKCA)
			{
				return m_position->m_state_info->castling_rights.operator() < _Cr > () &&
					((m_position->m_all_pieces[WHITE] | m_position->m_all_pieces[BLACK]) & key_kingside_squares) == 0 &&
					!m_position->square_attacked<!_S>(relative_e1) &&
					!m_position->square_attacked<!_S>(relative_f1) &&
					!m_position->square_attacked<!_S>(relative_g1);
			}
			else
			{
				return m_position->m_state_info->castling_rights.operator() < _Cr > () &&
					((m_position->m_all_pieces[WHITE] | m_position->m_all_pieces[BLACK]) & key_queenside_squares) == 0 &&
					!m_position->square_attacked<!_S>(relative_e1) &&
					!m_position->square_attacked<!_S>(relative_d1) &&
					!m_position->square_attacked<!_S>(relative_c1);
			}
		}
	};
}


#endif