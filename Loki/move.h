#ifndef MOVE_H
#define MOVE_H

#include "defs.h"

#include <algorithm>
#include <string>

/* The movetypes are:
	CAPTURES: All captures, promotions and en-passants (promotions are usually as good as captures, so they'll get sorted recordingly)
	QUIET: All moves that do not capture a piece or promote.
	CASTLE: All castling moves.
	ALL: All pseudo-legal moves in the position.
	EVASIONS: If we're in check, we'll only generate the legal check evasion moves

*/
enum MoveType :int { CAPTURES = 0, QUIET = 1, CASTLE = 2, ALL = 3};

/*
With these commands we can get information about a Move_t.move.
*/
enum SpecialMoves :int { PROMOTION = 0, ENPASSANT = 1, CASTLING = 2, NOT_SPECIAL = 3 };
#define FROMSQ(m) (((m) >> (4)) & (63))
#define TOSQ(m) ((m) >> (10))
#define PROMTO(m) (((m) >> (2)) & (3))
#define SPECIAL(m) ((m) & (3))

struct Move_t {
	unsigned int move = NOMOVE;
	int score = 0;
};

// The moveList class is inspired a lot by Stockfish
class MoveList {
public:
	~MoveList() {
		delete[] moveList;
	}

	MoveList() {
		moveList = new Move_t[MAXPOSITIONMOVES];
		last = moveList;
	}

	Move_t* begin() { return moveList; }
	Move_t* end() { return last; }

	size_t size() { return last - moveList; }

	Move_t* operator[](int);

	void add_move(int toSq, int fromSq, int promPce, int spc);

	Move_t at(int index);
	void replace(int index, Move_t newMove);

	void reset() {
		last = moveList;
	}
private:
	Move_t* moveList = nullptr;

	// Last element in the moveList. Used to get the size.
	Move_t* last = nullptr; 
};

const std::string FILES[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };
inline std::string index_to_uci(int sq) {
	int rank = sq / 8 + 1; std::string file = FILES[sq % 8];
	return file + std::to_string(rank);
}

extern std::string printMove(int move);
extern int parseMove(std::string userInput, MoveList* ml);


#endif