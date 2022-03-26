#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H


namespace loki::movegen {

	class move_generator {
	private:
		using attack_table_t	= std::array<bitboard_t, 64>;
		using move_list_t		= move_list<MAX_POSITION_MOVES>;

		position::position_t		m_position;
		magics::slider_generator_t	m_slider_generator;
		move_list_t					m_moves;
		static attack_table_t		knight_attacks;
		static attack_table_t		king_attacks;
	
	public:
		// No default constructor since a reference to the position object is required.
		move_generator() = delete;

		// No copying, only moving allowed.
		move_generator(const move_generator&) = delete;
		move_generator& operator=(const move_generator&) = delete;

		move_generator(move_generator&& _src);
		move_generator&& operator=(move_generator&& _src);

		move_generator(position::position_t pos, magics::slider_generator_t slider_generator) noexcept;

		template<move_type _Ty>
		move_list_t generate();
	private:
		
		static void init_knight_attacks() noexcept;
		static void init_king_attacks() noexcept;
	};

}


#endif