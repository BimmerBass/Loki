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
#include "util/exception.hpp"
#include "game_state.hpp"

namespace loki::position
{
	class position_proxy;

	class search_position
	{
		CHILD_EXCEPTION(position_error, loki_exception);
		friend class position_proxy;
	private:
		std::unique_ptr<game_state> m_state;
		bitboard_t m_piecebbs[NUM_SIDES][NUM_PIECES];
		bitboard_t m_all_pieces[NUM_SIDES];
		e_square m_king_squares[NUM_SIDES];


	};

	/// <summary>
	/// Provides simple getters and setters without cluttering search_position's interface.
	/// </summary>
	class position_proxy final
	{
	private:
		const search_position* m_ptr;
	public:

		position_proxy(const search_position* ptr)
			: m_ptr(ptr)
		{}

		template<side S, piece P>
		inline bitboard_t piece_bb() const { return m_ptr->m_piecebbs[S][P]; }
		template<side S>
		inline bitboard_t all_pieces() const { return m_ptr->m_all_pieces[S]; }
		template<side S>
		inline bitboard_t king_square() const { return m_ptr->m_king_squares[S]; }
		inline const game_state* game_state() const { return m_ptr->m_state.get(); }
	};
}
