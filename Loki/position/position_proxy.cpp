#include "position_proxy.hpp"
#include "search_position.hpp"

namespace loki::position
{
	template<side S, piece P> bitboard_t position_proxy::piece_bb() const { return m_pos->m_piecebbs[S][P]; }
	template<side S> bitboard_t position_proxy::all_pieces() const { return m_pos->m_all_pieces[S]; }
	template<side S> bitboard_t position_proxy::king_square() const { return m_pos->m_king_squares[S]; }
	const game_state* position_proxy::game_state() const { return m_pos->m_state.get(); }
}