#ifndef MOVEGEN_H
#define MOVEGEN_H
#include "position.h"


namespace moveGen {

	template<MoveType T>
	void generate(GameState_t* pos, MoveList* move_list);

	// Checks if the move exists for the current position
	bool moveExists(GameState_t* pos, unsigned int move);
}


inline void printMoveList(MoveList* ml) {
	for (int i = 0; i < ml->size(); i++) {
		std::cout << "Move " << (i + 1) << ": " << printMove((*ml)[i]->move) << " ( Score: " << (*ml)[i]->score << ", Encoded: " << (*ml)[i]->move << " )" << std::endl;
	}
}





#endif