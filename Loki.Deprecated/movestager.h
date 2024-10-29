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
#ifndef MOVESTAGER_H
#define MOVESTAGER_H
#include "thread.h"

// This is all the stages we generate the moves in. In the future, it would be nice to split the captures.
enum STAGE_T :int {
	TT_STAGE = 0,
	CAPTURE_SCORE_STAGE = 1,
	CAPTURE_STAGE = 2,
	QUIET_SCORE_STAGE = 3,
	QUIET_STAGE = 4,
	NO_STAGE = 5
};


// Function for determining whether a move is pseudo-legal or not
bool is_pseudo_legal(GameState_t* pos, unsigned int move, bool in_check);


// The MoveStager class is the one responsible for keeping track of which moves to search.
class MoveStager {
public:
	MoveStager();
	MoveStager(GameState_t* _pos, MoveStats_t* _stats, unsigned int ttMove, bool in_check); // For main search
	MoveStager(GameState_t* _pos); // For quiescence search.
	
	bool next_move(Move_t& move, bool skip_quiets = false);

	MoveList* get_moves();
protected:
	MoveList ml;
	int stage = TT_STAGE;
	int current_move = 0;

	template<MoveType T>
	void score(bool raise_captures = false);

	GameState_t* pos = nullptr;
	MoveStats_t* stats = nullptr;

	unsigned int tt_move = NOMOVE;

	void pi_sort();
};



class RootMoveStager : public MoveStager {
public:
	RootMoveStager(GameState_t* _pos, MoveStats_t* _stats, unsigned int ttMove);

	bool next_move(Move_t& move);
};




/// <summary>
/// Method for generating and scoring all moves. The two templates are for captures and quiets separately.
/// </summary>
/// <param name="raise_captures">A flag used to give captures a higher score than quiets (+10M). Used when generating all moves at once.</param>
template<MoveType T>
void MoveStager::score(bool raise_captures) {
	SIDE Them = (pos->side_to_move == WHITE) ? BLACK : WHITE;

	if constexpr (T == QUIET) {
		// Step 1. Generate all quiet moves.
		moveGen::generate<QUIET>(pos, &ml);

		// Step 2. Loop through the moves while scoring them.
		for (int i = 0; i < ml.size(); i++) {
			ml[i]->score = 0;

			// Step 2A. Killers (~79 elo).
			if (ml[i]->move == stats->killers[pos->ply][0]) {
				ml[i]->score = first_killer;
			}
			else if (ml[i]->move == stats->killers[pos->ply][1]) {
				ml[i]->score = second_killer;
			}

			// Step 2B. History (~243 elo).
			else {
				ml[i]->score = stats->history[pos->side_to_move][FROMSQ(ml[i]->move)][TOSQ(ml[i]->move)];
			}
		}
	}
	else {
		// Step 1. Generate the moves. We don't need to reset the list since these are the first moves to be generated.
		moveGen::generate<CAPTURES>(pos, &ml);

		// Step 2. Loop through all the moves.
		for (int i = 0; i < ml.size(); i++) {
			
			if (raise_captures) {
				ml[i]->score = 10000000;
			}
			else {
				ml[i]->score = 0;
			}

			// Step 2A. If the capture is a LxH, score it with MvvLva
			if (pos->piece_list[Them][TOSQ(ml[i]->move)] > pos->piece_list[pos->side_to_move][FROMSQ(ml[i]->move)]) {
				ml[i]->score = MvvLva[pos->piece_list[pos->side_to_move][FROMSQ(ml[i]->move)]][pos->piece_list[Them][TOSQ(ml[i]->move)]];
			}
			// Step 2B. Otherwise, we need to run SEE
			else {
				int score = pos->see(ml[i]->move);

				if (score >= 0) {
					ml[i]->score += score;
				}
				else {
					ml[i]->score *= -1;
					ml[i]->score += score;
				}
			}
		}
	}
}


#endif