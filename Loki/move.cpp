#include "move.h"


Move_t* MoveList::operator[](int index) {
	if (index >= size() || index < 0) {
		return nullptr;
	}
	return (moveList + index);
}


/*
MOVE REPRESENTATION IS INSPIRED BY STOCKFISH
A move can be stored in a 16 bit unsigned integer.
bit 0..5 -> destination square (0 to 63)
bit 6..11 -> origin square (0 to 63)
bit 12..13 -> promotion piece ((0) knight = 00, (1) bishop = 01, (2) rook = 10, (3) queen = 11)
bit 14..15 -> special move flag ((0) promotion = 00, (1) en-passant = 01, (2) castling = 10, (3) neither = 11)
NOTE: The en-passant flag is only set if the move is a pawn that can do an en-passant capture.
*/
void MoveList::add_move(int toSq, int fromSq, int promPce, int spc) {
	moveList[size()].move = ((toSq << 10) | (fromSq << 4) | (promPce << 2) | spc);
	moveList[size()].score = 0;

	last = &moveList[size() + 1];
}


std::string printMove(int move) {
	std::string moveStr = "";

	int fromSq = FROMSQ(move);
	int toSq = TOSQ(move);

	int spc = SPECIAL(move);
	int promotion_piece = PROMTO(move);

	assert(fromSq >= A1 && fromSq <= H8);
	assert(toSq >= A1 && fromSq <= H8);

	moveStr += index_to_uci(fromSq) + index_to_uci(toSq);

	if (spc == PROMOTION) {
		switch (promotion_piece) {
		case 0:
			moveStr += "n"; break;
		case 1:
			moveStr += "b"; break;
		case 2:
			moveStr += "r"; break;
		case 3:
			moveStr += "q"; break;
		default:
			break;
		}
	}
	return moveStr;
}


int parseMove(std::string userInput, MoveList* ml) {
	if (userInput.length() > 5) { return NOMOVE; }
	if (userInput[0] < 'a' || userInput[0] > 'h') { return NOMOVE; }
	if (userInput[1] < '1' || userInput[1] > '8') { return NOMOVE; }
	if (userInput[2] < 'a' || userInput[2] > 'h') { return NOMOVE; }
	if (userInput[3] < '1' || userInput[3] > '8') { return NOMOVE; }

	int from = ((userInput[0] - 'a') + (userInput[1] - '1') * 8);
	int to = ((userInput[2] - 'a') + (userInput[3] - '1') * 8);

	assert(from >= 0 && from <= 63);
	assert(to >= 0 && to <= 63);

	int move = 0;
	int promPce = NO_TYPE;
	int spcFlag = 3;


	for (int moveNum = 0; moveNum < ml->size(); moveNum++) {
		move = (*ml)[moveNum]->move;

		if (FROMSQ(move) == from && TOSQ(move) == to) {
			spcFlag = SPECIAL(move);
			if (spcFlag == PROMOTION) { // promotion
				promPce = PROMTO(move);
				if (toupper(userInput[4]) == 'N' && promPce == 0) {
					return move;
				}
				else if (toupper(userInput[4]) == 'B' && promPce == 1) {
					return move;
				}
				else if (toupper(userInput[4]) == 'R' && promPce == 2) {
					return move;
				}
				else if (toupper(userInput[4]) == 'Q' && promPce == 3) {
					return move;
				}
				continue;
			}
			return move;
		}

	}
	return NOMOVE;
}


Move_t MoveList::at(int index) {
	assert(moveList + index < last);

	return moveList[index];
}

void MoveList::replace(int index, Move_t newMove) {
	assert(moveList + index < last);

	moveList[index] = newMove;
}

/// <summary>
/// Checks if a move is in the movelist.
/// </summary>
/// <param name="move">: The move to search for.</param>
/// <returns>True if the move exists in the list and false if not.</returns>
bool MoveList::contains(unsigned int move) {
	if (move == NOMOVE) { return false; }

	for (int i = 0; i < size(); i++) {
		if (moveList[i].move == move) {
			return true;
		}
	}

	return false;
}