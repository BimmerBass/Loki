#pragma once
#include "defs.hpp"
#include "position/game_state.hpp"

namespace loki::position
{
	class i_position_view
	{
	public:
		virtual bitboard_t piece_bb(side s, piece p) const = 0;
		virtual bitboard_t all_pieces(side s) const = 0;
		virtual bitboard_t all_pieces() const = 0;
		virtual e_square king_square(side s) const = 0;
		virtual const game_state* game_state() const = 0;

		virtual ~i_position_view() {}
	};
}