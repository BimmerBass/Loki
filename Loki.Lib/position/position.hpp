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
#ifndef POSITION_H
#define POSITION_H

namespace loki::position
{

	/// <summary>
	/// The position class is the central object describing the game state. It is the one responsible for changing a game_state object in a legal way while also optimizing the
	/// data-extraction from a position. This is for example done by indexing where all pieces are in both arrays and bitboards, and where the kings are.
	/// </summary>
	class position
	{
		friend class movegen::move_generator;
	private:
		movegen::move_stack_t<MAX_GAME_MOVES>	m_move_history;					/* The stack of moves that has led to this position. */
		movegen::move_generator_t				m_generator;					/* The object responsible for finding all pseudo-legal moves in the position. */
		game_state_t							m_state_info;					/* Our basic state-describing object. */
		ePiece									m_piece_list[SIDE_NB][SQ_NB];	/* An array to easier look up pieces on specific squares. */
		bitboard_t								m_all_pieces[SIDE_NB];			/* Two bitboards containing all pieces on the respective sides. */
		eSquare									m_king_squares[SIDE_NB];		/* The two squares where the kings are. */
		size_t									m_ply;							/* The ply-depth we're at from the base position. */
		weak_position_t							m_self;							/* Shared-ptr to this object. */
		hashkey_t								m_poskey;						/* Position (zobrist) hash key. */
		zobrist_t								m_hashing_generator;			/* Object to alter the position's hash. */
	public:
		/// <summary>
		/// Create a position object. A static method is needed because the m_self property to be set.
		/// </summary>
		/// <param name="generator"></param>
		/// <returns></returns>
		static position_t create_position(game_state_t&& internal_state, movegen::magics::slider_generator_t magic_index);

		/// <summary>
		/// Make a move.
		/// </summary>
		/// <param name="move"></param>
		/// <returns></returns>
		bool make_move(move_t move);

		/// <summary>
		/// Undo the previous move made.
		/// </summary>
		void undo_move();

		/// <summary>
		/// Generate moves of the given type, for the side to move.
		/// </summary>
		/// <returns></returns>
		template<movegen::MOVE_TYPE _Ty = movegen::ALL>
		const movegen::move_list_t& generate_moves();

		// Checks if a square is attacked by one of the sides.
		template<eSide _Si> requires (_Si == WHITE || _Si == BLACK)
			bool square_attacked(eSquare sq) const noexcept;
		bool in_check() const noexcept;

		/// <summary>
		/// Will check if the current position has already been reached.
		/// </summary>
		bool is_repetition() const;

		/// <summary>
		/// Determine how many pieces of type _Pi side _Si has on the board.
		/// </summary>
		template<eSide _Si, ePiece _Pi> requires (_Si == WHITE || _Si == BLACK) && (_Pi >= 0 && _Pi <= 4)
			inline size_t piece_count() const noexcept
		{
			return count_bits(m_state_info->piece_placements[_Si][_Pi]);
		}

		/// <summary>
		/// Determine the side to move.
		/// </summary>
		/// <returns></returns>
		inline eSide side_to_move() const noexcept
		{
			return m_state_info->side_to_move;
		}

		/// <summary>
		/// Get the ply we're at.
		/// </summary>
		inline size_t ply() const noexcept { return m_ply; }

		inline bool is_draw() const { return m_state_info->fifty_move_counter >= 100 || is_repetition(); }

		/// <summary>
		/// Load a FEN.
		/// </summary>
		/// <param name="fen"></param>
		void operator<<(const std::string& fen);

		/// <summary>
		/// Generate a FEN.
		/// </summary>
		/// <param name="fen"></param>
		void operator>>(std::string& fen) const;

		/// <summary>
		/// Print the position.
		/// </summary>
		/// <param name="os"></param>
		/// <param name="pos"></param>
		/// <returns></returns>
		friend std::ostream& operator<<(std::ostream& os, const position& pos);
		friend std::ostream& operator<<(std::ostream& os, const position_t& pos);
	private:
		position(game_state_t&& internal_state, movegen::magics::slider_generator_t magic_index);

		/// <summary>
		/// Move the rook during a castling move
		/// </summary>
		/// <param name="orig"></param>
		/// <param name="dest"></param>
		void perform_rook_castle(size_t orig, size_t dest, bool undo = false) noexcept;

		/// <summary>
		/// Re-load this object from our internal state.
		/// Note: This function is expensive and should only be used for loading a FEN.
		/// </summary>
		void reload();

		/// <summary>
		/// Update the occupancy bitboards.
		/// </summary>
		void update_occupancies();

		/// <summary>
		/// Generate a position hash-key from scratch.
		/// Note: This is expensive, and should only be used for newly loaded positions. In all other cases, incremental updates in make_move and undo_move are preferred.
		/// </summary>
		hashkey_t generate_poskey() const;
	};
}


#endif