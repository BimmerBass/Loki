#pragma once
#include "defs.hpp"
#include "position/game_state.hpp"

namespace loki::position
{
	class position_proxy final
	{
	private:
		const search_position* m_pos;
	public:
		position_proxy(const search_position* pos) : m_pos{ pos }
		{}

		template<side S, piece P> bitboard_t piece_bb() const;
		template<side S> bitboard_t all_pieces() const;
		template<side S> bitboard_t king_square() const;
		const game_state* game_state() const;
	};
}