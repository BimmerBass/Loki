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