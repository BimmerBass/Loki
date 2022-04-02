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
		SQUARE en_passant_square;
		castle_rights castling_rights;
	};

}

#endif