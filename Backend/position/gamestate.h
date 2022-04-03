#ifndef GAMESTATE_H
#define GAMESTATE_H

namespace loki::position {

	/// <summary>
	/// game_state holds the most basic data to describe a chess position.
	/// </summary>
	struct game_state {
		// Piece positions.
		bitboard_t piece_placements[SIDE::SIDE_NB][PIECE::PIECE_NB];
		SIDE side_to_move;

		size_t fifty_move_counter;
		size_t full_move_counter;
		SQUARE en_passant_square;
		castle_rights castling_rights;

		/// <summary>
		/// Parse a FEN position.
		/// </summary>
		/// <param name="fen"></param>
		void operator<<(const std::string& fen);

		/// <summary>
		/// Generate a FEN string from the current position.
		/// </summary>
		/// <param name="fen"></param>
		void operator>>(std::string& fen);

		/// <summary>
		/// Print the position to a given stream.
		/// </summary>
		void print_position(std::ostream& os);
	};

}

#endif