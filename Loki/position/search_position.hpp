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
#include "movegen/move.hpp"
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
	public:
		CHILD_EXCEPTION(position_error, loki_exception);

		class position_proxy final
		{
		private:
			const search_position* m_pos;
		public:
			position_proxy(const search_position* pos) : m_pos{ pos }
			{
			}

			bitboard_t piece_bb(side s, piece p) const noexcept { return m_pos->m_piecebbs[s][p]; }
			bitboard_t all_pieces(side s) const noexcept { return m_pos->m_all_pieces[s]; }
			bitboard_t all_pieces() const noexcept { return m_pos->m_all_pieces[WHITE] | m_pos->m_all_pieces[BLACK]; }
			e_square king_square(side s) const noexcept { return m_pos->m_king_squares[s]; }
			const position::game_state* game_state() const noexcept { return m_pos->m_state.get(); }
		};

	private:
		friend search_position_t make(
			game_state_t state,
			movegen::magics::magic_index_t bishop_index,
			movegen::magics::magic_index_t rook_index);
		
		std::unique_ptr<game_state> m_state;
		bitboard_t m_piecebbs[NUM_SIDES][NUM_PIECES];
		bitboard_t m_all_pieces[NUM_SIDES];
		e_square m_king_squares[NUM_SIDES];

		position_history<constants::MAX_GAME_MOVES> m_history;
		ply_t m_ply;

		std::unique_ptr<movegen::i_move_generator<position_proxy>> m_move_generator;
	private:
		search_position(std::unique_ptr<game_state> state, std::unique_ptr<movegen::i_move_generator<position_proxy>> move_generator);

	public:
		search_position(const search_position&) = delete;
		search_position& operator=(const search_position&) = delete;
		search_position(search_position&&) = default;
		search_position& operator=(search_position&&) = default;
		
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
		/// Determine whether the current position is drawn.
		/// </summary>
		bool is_draw() const noexcept;

		/// <summary>
		/// Determine whether neither side has sufficient material to mate.
		/// TODO: Implement material draw detection.
		/// </summary>
		bool is_material_draw() const noexcept;

		/// <summary>
		/// Determine whether the current position has occurred before.
		/// TODO: Implement after position hashing is available.
		/// </summary>
		bool is_repetition() const noexcept;

		/// <summary>
		/// Generate pseudo-legal moves for the current side.
		/// </summary>
		/// <param name="ml">A pointer to the move list object</param>
		/// <returns>The number of moves generated.</returns>
		template<movegen::move_type MT = movegen::ALL>
		[[maybe_unused]] size_t generate_moves(movegen::move_list* ml) const;

		/// <summary>
		/// Generate all legal moves for the current side. This is expensive and shouldn't be used in search.
		/// </summary>
		/// <param name="ml">A pointer to the move list object</param>
		/// <returns>The number of moves generated.</returns>
		[[maybe_unused]] size_t generate_all_legals(movegen::move_list* ml);

		/// <summary>
		/// Get the side to move.
		/// </summary>
		/// <returns>The side to move...</returns>
		inline side side_to_move() const noexcept
		{
			return m_state->side_to_move;
		}

		/// <summary>
		/// Represent the position as a FEN.
		/// Note that this is *very* inefficient and should never be executed along a critical path!
		/// </summary>
		/// <returns>A std::unique_ptr<game_state> rvalue reference</returns>
		std::string to_fen() const { return game_state::to_fen(std::make_shared<game_state>(*m_state)); }

		/// <summary>
		/// Create a position view to read data from the object.
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] 
		position_proxy make_view() const& noexcept
		{
			return position_proxy(this);
		}
		position_proxy make_view() const&& = delete;

		/// <summary>
		/// Find out if a move has been made on this object by checking for emptiness on the history stack.
		/// </summary>
		/// <returns>true if a move has been made, false if not.</returns>
		inline bool has_made_move() const noexcept { return m_history.size() > 0; }

		/// <summary>
		/// Get the current ply relative to the root of the search.
		/// </summary>
		/// <returns>A mutable reference to the current search ply.</returns>
		[[nodiscard]] inline ply_t& ply() noexcept { return m_ply; }

		/// <summary>
		/// Get the current ply relative to the root of the search.
		/// </summary>
		/// <returns>The current search ply.</returns>
		[[nodiscard]] inline ply_t ply() const noexcept { return m_ply; }

		/// <summary>
		/// Clone the current object for use in a single search_worker.
		/// Note: This is expensive, so do not use during search.
		/// </summary>
		/// <returns>a unique_ptr to a copy of the current object.</returns>
		std::unique_ptr<search_position> clone() const;
	};

	extern template size_t search_position::generate_moves<movegen::ALL>(movegen::move_list*) const;
	extern template size_t search_position::generate_moves<movegen::ACTIVE>(movegen::move_list*) const;
	extern template size_t search_position::generate_moves<movegen::QUIET>(movegen::move_list*) const;
}
