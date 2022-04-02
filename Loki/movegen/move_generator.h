#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H


namespace loki::movegen {

	class move_generator {
	public:
		using move_list_t = move_list<MAX_POSITION_MOVES>;
	private:
		using attack_table_t	= std::array<bitboard_t, 64>;

		position::position_t		m_position;
		magics::slider_generator_t	m_slider_generator;
		move_list_t					m_moves;
		static attack_table_t		knight_attacks;
		static attack_table_t		king_attacks;
	
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

		static void init_knight_attacks() noexcept;
		static void init_king_attacks() noexcept;

		template<SIDE _S, CASTLING_RIGHTS _Cr>
		inline bool can_castle() const {
			constexpr SQUARE relative_e1				= _S == WHITE ? E1 : E8;
			constexpr SQUARE relative_f1				= _S == WHITE ? F1 : F8;
			constexpr SQUARE relative_g1				= _S == WHITE ? G1 : G8;
			constexpr SQUARE relative_d1				= _S == WHITE ? D1 : D8;
			constexpr SQUARE relative_c1				= _S == WHITE ? C1 : C8;
			constexpr bitboard_t key_queenside_squares	= bitmasks::rank_masks[_S == WHITE ? RANK_1 : RANK_8] & (bitmasks::file_masks[FILE_G] | bitmasks::file_masks[FILE_F]);
			constexpr bitboard_t key_kingside_squares	= bitmasks::rank_masks[_S == WHITE ? RANK_1 : RANK_8] 
															& (bitmasks::file_masks[FILE_C] | bitmasks::file_masks[FILE_D] | bitmasks::file_masks[FILE_B]);

			if (_Cr == WKCA || _Cr == BKCA) {
				return m_position->m_state_info->castling_rights.operator()<_Cr>() &&
					((m_position->m_all_pieces[WHITE] | m_position->m_all_pieces[BLACK]) & key_kingside_squares) == 0 &&
					!m_position->square_attacked<!_S>(relative_e1) &&
					!m_position->square_attacked<!_S>(relative_f1) &&
					!m_position->square_attacked<!_S>(relative_g1);
			}
			else {
				return m_position->m_state_info->castling_rights.operator()<_Cr>() &&
					((m_position->m_all_pieces[WHITE] | m_position->m_all_pieces[BLACK]) & key_queenside_squares) == 0 &&
					!m_position->square_attacked<!_S>(relative_e1) &&
					!m_position->square_attacked<!_S>(relative_d1) &&
					!m_position->square_attacked<!_S>(relative_c1);
			}
		}
	};

}


#endif