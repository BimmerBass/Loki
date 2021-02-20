#include "thread.h"



/*

Helper functions for SearchThread move-picking

*/

MoveList* SearchThread_t::generate_moves(bool qsearch) {
	MoveList* ml = (qsearch == false) ? moveGen::generate<ALL>(pos) : moveGen::generate<CAPTURES>(pos);

	score_moves(ml);

	return ml;
}


void SearchThread_t::setKillers(int ply, int move) {
	killers[ply][1] = killers[ply][0];
	killers[ply][0] = move;
}


void SearchThread_t::score_moves(MoveList* ml) {
	SIDE Them = (pos->side_to_move == WHITE) ? BLACK : WHITE;
	
	for (int i = 0; i < ml->size(); i++) {
		// If it is a quiet move, i.e. not a capture, promotion or en-passant, we'll score it with killers and history heuristic
		if (pos->piece_list[Them][TOSQ((*ml)[i]->move)] == NO_TYPE && SPECIAL((*ml)[i]->move) != PROMOTION && SPECIAL((*ml)[i]->move) != ENPASSANT) {

			// Killers
			if ((*ml)[i]->move == killers[pos->ply][0]) {
				(*ml)[i]->score = first_killer;
			}
			else if ((*ml)[i]->move == killers[pos->ply][1]) {
				(*ml)[i]->score = second_killer;
			}

			// Countermoves
			else if (pos->ply > 0 && (*ml)[i]->move == counterMoves[FROMSQ(moves_path[pos->ply - 1])][TOSQ(moves_path[pos->ply - 1])]) {
				(*ml)[i]->score = countermove_bonus;
			}

			// History
			else {
				(*ml)[i]->score = history[pos->side_to_move][FROMSQ((*ml)[i]->move)][TOSQ((*ml)[i]->move)];
			}

		}

		// It is a tactical move. We'll score this the highest
		else {
			(*ml)[i]->score = capture_bonus;

			// Captures get scored with MVV/LVA.
			if (pos->piece_list[Them][TOSQ((*ml)[i]->move)] != NO_TYPE) {
				(*ml)[i]->score += MvvLva[pos->piece_list[pos->side_to_move][FROMSQ((*ml)[i]->move)]][pos->piece_list[Them][TOSQ((*ml)[i]->move)]];
			}
		
		}

	}
}


void SearchThread_t::pickNextMove(int index, MoveList* ml) {
	int best_index = index;
	int best_score = 0;

	for (int m = index; m < ml->size(); m++) {

		// If the score for (*ml)[m] is higher than the maximum score we've found, this is the new best move.
		if ((*ml)[m]->score > best_score) {
			best_index = m;

			best_score = (*ml)[m]->score;
		}
	}
	// Copy the move at index and at best_index
	Move_t temp_move = ml->at(index);
	Move_t best_move = ml->at(best_index);

	// Insert best_move at index and place temp_move (the previous move at index) at best_index.
	ml->replace(index, best_move);
	ml->replace(best_index, temp_move);
}




void ThreadPool_t::init_threads(GameState_t* pos, SearchInfo_t* info) {

	for (int i = 0; i < threadNum; i++) {
		*threads[i].pos = *pos;
		*threads[i].info = *info;
		threads[i].thread_id = i;
	}

}