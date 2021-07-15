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



// The MoveStager class is the one responsible for keeping track of which moves to search.
class MoveStager {
public:
	MoveStager(GameState_t* _pos, const MoveStats_t* stats, unsigned int ttMove); // For main search
	MoveStager(const GameState_t* _pos); // For quiescence search.
	
	bool next_move(Move_t& move, bool skip_quiets = true);

private:
	MoveList ml;
	int stage = TT_STAGE;
	int current_move = 0;

	template<MoveType T>
	void score();

	GameState_t* pos = nullptr;
	MoveStats_t* stats = nullptr;

	unsigned int tt_move = NOMOVE;

	void pi_sort();
};


// Note: The below method is placed in the header since it is templated.
template<MoveType T>
void MoveStager::score() {
	SIDE Them = (pos->side_to_move == WHITE) ? BLACK : WHITE;

	if constexpr (T == QUIET) {
		// Step 1. Clear the movelist and generate all quiet moves.
		ml.reset();
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
			ml[i]->score = 0;

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