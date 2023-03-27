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
#ifndef GAMESTATE_H
#define GAMESTATE_H

namespace loki::position
{

	/// <summary>
	/// game_state holds the most basic data to describe a chess position.
	/// </summary>
	struct game_state
	{
		EXCEPTION_CLASS(e_gameState, e_lokiError);

		// Piece positions.
		bitboard_t piece_placements[eSide::SIDE_NB][ePiece::PIECE_NB] = { {0} };
		eSide side_to_move = WHITE;

		size_t fifty_move_counter = 0;
		size_t full_move_counter = 0;
		eSquare en_passant_square = NO_SQ;
		castle_rights castling_rights = 0;

		/// <summary>
		/// Parse a FEN position.
		/// </summary>
		/// <param name="fen"></param>
		void operator<<(const std::string& fen);

		/// <summary>
		/// Generate a FEN string from the current position.
		/// </summary>
		/// <param name="fen"></param>
		void operator>>(std::string& fen) const;

		/// <summary>
		/// Print the position to a given stream.
		/// </summary>
		friend std::ostream& operator<<(std::ostream& os, const game_state& gs);

		game_state() = default;
		game_state(const game_state& _Other);
	};

}

#endif