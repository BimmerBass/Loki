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
#include "position_history.hpp"
#include "movegen/magics/magic_index.hpp"
#include "movegen/move_list.hpp"
#include "movegen/i_move_generator.hpp"
#include <defs.hpp>

namespace loki::position
{
	class search_position;
	using search_position_t = std::shared_ptr<search_position>;

	/// <summary>
	/// Create a new instance of a search_position.
	/// </summary>
	/// <param name="state">The initial game state to load. Note that a new unique_ptr will be created from this.</param>
	/// <param name="bishop_index">A pointer to the bishop magic index</param>
	/// <param name="rook_index">A pointer to the rook magic index</param>
	/// <returns>A search_position wrapped in a std::shared_ptr</returns>
	search_position_t make(
		game_state_t state,
		movegen::magics::magic_index_t bishop_index,
		movegen::magics::magic_index_t rook_index);

	/// <summary>
	/// Wrapper for game_state, which is optimized for quick move generation during search.
	/// </summary>
	class search_position final
	{
		CHILD_EXCEPTION(position_error, loki_exception);
		friend class position_proxy;
		friend search_position_t make(
			game_state_t state,
			movegen::magics::magic_index_t bishop_index,
			movegen::magics::magic_index_t rook_index);
	private:
		std::unique_ptr<game_state> m_state;
		bitboard_t m_piecebbs[NUM_SIDES][NUM_PIECES];
		bitboard_t m_all_pieces[NUM_SIDES];
		e_square m_king_squares[NUM_SIDES];

		position_history<constants::MAX_GAME_MOVES> m_history;

		std::unique_ptr<movegen::i_move_generator> m_move_generator;
	private:
		search_position(std::unique_ptr<game_state> state, std::unique_ptr<movegen::i_move_generator> move_generator);

	public:
		/// <summary>
		/// Perform a move on the position.
		/// </summary>
		/// <param name="move">The move to perform.</param>
		/// <returns>True if the move was legal, false otherwise.</returns>
		bool make_move(const movegen::move& move);

		/// <summary>
		/// Undo the last move.
		/// </summary>
		/// <exception cref="position_error">If no previous moves has been made.</exception>
		void undo_last_move();

		/// <summary>
		/// Determine if the side to move is in check.
		/// </summary>
		/// <returns>a bool indicating if we're in check</returns>
		bool in_check() const;

		/// <summary>
		/// Generate all pseudo-legal moves for the current side.
		/// </summary>
		/// <param name="ml">A pointer to the move list object</param>
		/// <returns>The number of moves generated.</returns>
		size_t generate_moves(movegen::move_list* ml) const;

		/// <summary>
		/// Extract the game_state instance
		/// Note that after this function has been called, no other operations should be performed on this search_position object.
		/// </summary>
		/// <returns>A std::unique_ptr<game_state> rvalue reference</returns>
		std::unique_ptr<game_state>&& state() { return std::move(m_state); }
	};
}
