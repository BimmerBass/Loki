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
	int best_capture_possible();


	// For seeing if a square is attacked by one of the sides.
	bool square_attacked(int square, SIDE side);

	// Returns true if the side to move is in check
	bool in_check();

	// Returns true if there are no pieces for the side to move. Used for null move pruning
	bool safe_nullmove();


	// Returns true if we're in the late endgame
	bool is_endgame();

	// Returns true if the position has been had before.
	bool is_repetition();


	/*
	SEE functions - the SEE algorithm itself will be implemented later
	*/
	// Returns true if piece, pce on square, sq is pinned to the king
	bool is_pinned(int sq, SIDE s);


	/*
	Debugging functions
	*/
	bool lists_match();

	void mirror_board();


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
	// Array for all SavedInfo_t after each move. Declared on heap because it might take too much stack when having multiple GameState_t for multithreading.
	SavedInfo_t history[MAXGAMEMOVES] = {  };
	int history_ply = 0; // Amount of SaveInfo_t in history.

	// Used in static exchange evaluation
	int least_valuable_attacker(int square, SIDE side);

	
};




#endif