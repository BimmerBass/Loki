#include "thread.h"



/*

Helper functions for SearchThread move-picking

*/

void SearchThread_t::generate_moves(MoveList* moves, bool qsearch) {
	// Generate the moves
	if (qsearch) {
		moveGen::generate<CAPTURES>(pos, moves);
	}
	else {
		moveGen::generate<ALL>(pos, moves);
	}

	score_moves(moves);

}


void SearchThread_t::setKillers(int ply, int move) {
	killers[ply][1] = killers[ply][0];
	killers[ply][0] = move;
}


									/* N,  B,   R,   Q */
constexpr int promotion_values[4] = { 300, 350, 500, 900 };

void SearchThread_t::score_moves(MoveList* ml) {
	SIDE Them = (pos->side_to_move == WHITE) ? BLACK : WHITE;
	
	for (int i = 0; i < ml->size(); i++) {
		// If it is a quiet move, i.e. not a capture, promotion or en-passant, we'll score it with killers and history heuristic
		if (pos->piece_list[Them][TOSQ((*ml)[i]->move)] == NO_TYPE && SPECIAL((*ml)[i]->move) != PROMOTION && SPECIAL((*ml)[i]->move) != ENPASSANT) {
	
			// Killers (~79 elo)
			if ((*ml)[i]->move == killers[pos->ply][0]) {
				(*ml)[i]->score = first_killer;
			}
			else if ((*ml)[i]->move == killers[pos->ply][1]) {
				(*ml)[i]->score = second_killer;
			}
	
			// Countermoves
			//else if (pos->ply > 0 && (*ml)[i]->move == counterMoves[FROMSQ(moves_path[pos->ply - 1])][TOSQ(moves_path[pos->ply - 1])]) {
			//	(*ml)[i]->score = countermove_bonus;
			//}
	
			// History (~243 elo)
			else {
				(*ml)[i]->score = history[pos->side_to_move][FROMSQ((*ml)[i]->move)][TOSQ((*ml)[i]->move)];
			}
		}
	
		// It is a tactical move. We'll score this the highest
		else {
			(*ml)[i]->score = 10000000; // Make sure tactical moves are searched first.
		
			// Captures get scored with MVV/LVA. (~75 elo)
			if (pos->piece_list[Them][TOSQ((*ml)[i]->move)] != NO_TYPE) {
				(*ml)[i]->score += MvvLva[pos->piece_list[pos->side_to_move][FROMSQ((*ml)[i]->move)]][pos->piece_list[Them][TOSQ((*ml)[i]->move)]];
			}
			
			// Promotions will also be scored highly.
			//if (SPECIAL((*ml)[i]->move) == PROMOTION) {
			//	// If the promotion piece is a queen or we're capturing a piece, we'll score this just below the hash move
			//	if (PROMTO((*ml)[i]->move) + 1 == QUEEN) {
			//		(*ml)[i]->score = hash_move_sort - 100;
			//	}
			//	else { // Will be scored as loosing a pawn and gaining the promotion piece.
			//		(*ml)[i]->score += MvvLva[PAWN][PROMTO((*ml)[i]->move) + 1];
			//	}
			//}
			//
			//// En-passants will be scored simply as PxP
			//if (SPECIAL((*ml)[i]->move) == ENPASSANT) {
			//	(*ml)[i]->score += MvvLva[PAWN][PAWN];
			//}
		
		}
	
	}
}


void SearchThread_t::pickNextMove(int index, MoveList* ml) {
	int best_index = index;
	int best_score = -hash_move_sort;

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



void SearchThread_t::update_move_heuristics(int best_move, int depth, MoveList* ml) {
	// Set the new killer moves
	setKillers(pos->ply, best_move);

	// Update the countermove heuristic
	//if (pos->ply > 0) { // Prevent overflow
	//	counterMoves[FROMSQ(moves_path[pos->ply - 1])][TOSQ(moves_path[pos->ply - 1])] = move;
	//}


	/*
	Update history
	*/
	int history_bonus = std::min(depth * depth, 400);

	history[pos->side_to_move][FROMSQ(best_move)][TOSQ(best_move)] += history_bonus;

	
	// Decrease history value for all other quiet moves, since they didn't fail high
	unsigned int move = NOMOVE;
	for (int mn = 0; mn < ml->size(); mn++) {
		move = (*ml)[mn]->move;
		if (pos->piece_list[(pos->side_to_move == WHITE) ? BLACK : WHITE][TOSQ(move)] == NO_TYPE && SPECIAL(move) != PROMOTION
			&& SPECIAL(move) != ENPASSANT) { // No piece is captured, not a promotion and not an en-passant

			history[pos->side_to_move][FROMSQ(move)][TOSQ(move)] = std::max(-countermove_bonus, history[pos->side_to_move][FROMSQ(move)][TOSQ(move)] - history_bonus);
		}
	}

	// Handle history table overflows
	if (history[pos->side_to_move][FROMSQ(best_move)][TOSQ(best_move)] >= countermove_bonus) {
	
		for (int i = 0; i < 64; i++) {
	
			for (int j = 0; j < 64; j++) {
	
				history[0][i][j] /= 2;
				history[1][i][j] /= 2;
			}
		}
	}
}


void SearchThread_t::clear_move_heuristics() {

	for (int i = 0; i < 64; i++) {
	
		for (int j = 0; j < 64; j++) {
			//counterMoves[i][j] = 0;
	
			history[0][i][j] = 0;
			history[1][i][j] = 0;
		}
	
	}

	for (int d = 0; d < MAXDEPTH; d++) {
		//moves_path[d] = 0;
		//static_eval[d] = 0;

		killers[d][0] = 0;
		killers[d][1] = 0;
	}
}




void ThreadPool_t::init_threads(GameState_t* pos, SearchInfo_t* info) {

	for (int i = 0; i < threadNum; i++) {
		*threads[i].pos = *pos;
		*threads[i].info = *info;
		threads[i].thread_id = i;
	}

}