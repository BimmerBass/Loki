/*
	Loki, a UCI-compliant chess playing software
	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

	Loki is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Loki is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once
#ifndef POSIITON_H
#define POSITION_H

#include "bitboard.h"
#include "move.h"

#if !defined(_MSC_VER)
#include <cstring> // To use strcpy with GCC
#endif


// Class for saving all info that has been lost when making a move.
class SavedInfo_t {
public:
	int move = NOMOVE;
	int piece_captured = NO_TYPE;
	int piece_moved = NO_TYPE;

	int castleRights = 0;
	int fifty_moves = 0;
	int enPasSq = 0;

	uint64_t posKey = 0;
};


class GameState_t {
public:
	// Indexed by pieceBBS[pieceType][Color]
	Bitboard pieceBBS[6][2] = { {0} };

	// Indexed by piece_list[color][sq] to get a piecetype at that square.
	int piece_list[2][64] = { {0} };
	int piece_on(int sq, SIDE side) const;

	SIDE side_to_move = WHITE;

	// Indexed by all_pieces[Color] to get all pieces' positions for white and all for black respectively.
	Bitboard all_pieces[2] = { 0 };

	// King squares. Indexed by king_squares[side]
	int king_squares[2] = { 0 };

	int enPasSq = NO_SQ;

	int castleRights = 0;
	
	template<C_RIGHTS cr>
	bool can_castle();

	volatile int ply = 0;
	int fiftyMove = 0;



	// The zobrist hash of the position.
	volatile Bitboard posKey = 0;
	void generate_poskey();


	// For making moves on the board.
	bool make_move(Move_t* move);
	void undo_move();

	// For null move pruning
	int make_nullmove();
	void undo_nullmove(int oldEnPas);

	// Returns the value of the best possible capture on the board.
	int best_capture_possible() const;

	// For seeing if a square is attacked by one of the sides.
	bool square_attacked(int square, SIDE side) const;

	// Returns true if the side to move is in check
	bool in_check() const;

	// Returns a bitboard with all the pieces pinned to the king of side S
	template<SIDE S>
	Bitboard pinned_pieces() const;

	// Returns true if there are no sliding pieces for the side to move. Used for null move pruning
	bool safe_nullmove() const;


	// Returns true if we're in the late endgame
	bool is_endgame() const;

	// Returns true if both sides have pieces on the board.
	bool non_pawn_material() const;
	
	// Returns true if we are repeating moves or have reached the fifty-move rule limit.
	bool is_draw() const;

	/*
	SEE functions - the SEE algorithm itself will be implemented later
	*/
	// Returns a bitboard with all attackers to a given square (from both sides)
	Bitboard attackers_to(int sq, Bitboard occupied) const;
	Bitboard attackSlider(Bitboard occupied, int to_sq, SIDE side) const;

	int see(unsigned int move) const;


	/*
	Debugging functions
	*/
	bool lists_match();

	void mirror_board();

	// Will return false if the king-positions doesn't match up or other weird things is happening with the board. Used for debugging
	bool is_ok();

	/*
	UI
	*/

	// Clear the GameState_t
	void clearPos();

	// UI related functions
	void parseFen(const std::string FEN_STR);
	void displayBoardState();


	/*
	Constructors
	*/

	// Copy constructor
	GameState_t(const GameState_t& pos);

	// Default constructor. Is just zero since everything is initialized already.
	GameState_t();

	~GameState_t() {
	}

private:
	// Returns true if the position has been had before.
	bool is_repetition() const;

	// Returns true if the material situation on the board is such that none of the sides can possibly checkmate the other.
	bool insufficient_material() const;

	// Array for all SavedInfo_t after each move. Declared on heap because it might take too much stack when having multiple GameState_t for multithreading.
	SavedInfo_t history[MAXGAMEMOVES] = {  };
	int history_ply = 0; // Amount of SaveInfo_t in history.	
};




/*

Pinned pieces --> Gets all pieces of color S pinned to the king.
NOTE: This function is in the header due to the template.
*/


template<SIDE S>
Bitboard GameState_t::pinned_pieces() const {
	constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;
	int king_square = king_squares[S];

	Bitboard pinned = 0;

	// We need to generate the queen attacks (as if the board was cleared for all pieces) from the king square. We do this with our magic bitboards
	Bitboard king_rays = Magics::attacks_bb<QUEEN>(king_square, 0);
	Bitboard king_attacks = Magics::attacks_bb<QUEEN>(king_square, all_pieces[WHITE] | all_pieces[BLACK]);

	// Firstly, we'll check for bishops or queens on the diagonals
	Bitboard queen_bishop_attackers = (pieceBBS[BISHOP][Them] | pieceBBS[QUEEN][Them]) &
		(king_rays & (BBS::diagonalMasks[7 + (king_square / 8) - (king_square % 8)] | BBS::antidiagonalMasks[(king_square / 8) + (king_square % 8)]));

	int sq = NO_SQ;
	Bitboard attacks = 0;
	Bitboard blockers = 0;
	while (queen_bishop_attackers) {
		sq = PopBit(&queen_bishop_attackers); // Get the position of the attacking piece
		attacks = Magics::attacks_bb<BISHOP>(sq, all_pieces[WHITE] | all_pieces[BLACK]);

		blockers = (attacks & king_attacks);
		blockers &= (BBS::diagonalMasks[7 + (king_square / 8) - (king_square % 8)] | BBS::antidiagonalMasks[(king_square / 8) + (king_square % 8)]);
		blockers &= all_pieces[S];

		if (blockers) {
			assert(countBits(blockers) == 1);

			pinned |= (uint64_t(1) << bitScanForward(blockers));
		}
	}

	// Now we'll get the attacks on files/ranks from rooks and queens
	Bitboard queen_rook_attackers = (pieceBBS[ROOK][Them] | pieceBBS[QUEEN][Them]) &
		(king_rays & (BBS::FileMasks8[king_square % 8] | BBS::RankMasks8[king_square / 8]));

	while (queen_rook_attackers) {
		sq = PopBit(&queen_rook_attackers);
		attacks = Magics::attacks_bb<ROOK>(sq, all_pieces[WHITE] | all_pieces[BLACK]);

		blockers = (attacks & king_attacks) & (BBS::FileMasks8[king_square % 8] | BBS::RankMasks8[king_square / 8]);
		blockers &= all_pieces[S];

		if (blockers) {
			assert(countBits(blockers) == 1);

			pinned |= (uint64_t(1) << bitScanForward(blockers));
		}
	}

	return pinned;
}



#endif