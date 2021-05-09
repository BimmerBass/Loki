#include "thread.h"



/*

Helper function to clear a searchinfo instance

*/

void SearchInfo_t::clear() {
	starttime = 0;
	stoptime = 0;

	depth = MAXDEPTH;
	seldepth = 0;
	depthset = MAXDEPTH;

	timeset = false;
	movestogo = 0;
	infinite = false;

	nodes = 0;

	quit = false;
	stopped = false;

	fh = 0;
	fhf = 0;
}



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
	// If the move is already the first killer, don't add it since it'll just result in duplicate killers
	if (stats.killers[ply][0] != move) {
		stats.killers[ply][1] = stats.killers[ply][0];
		stats.killers[ply][0] = move;
	}
}


									/* N,  B,   R,   Q */
constexpr int promotion_values[4] = { 300, 350, 500, 900 };

void SearchThread_t::score_moves(MoveList* ml) {
	SIDE Them = (pos->side_to_move == WHITE) ? BLACK : WHITE;
	
	for (int i = 0; i < ml->size(); i++) {
		// If it is a quiet move, i.e. not a capture, promotion or en-passant, we'll score it with killers and history heuristic
		if (pos->piece_list[Them][TOSQ((*ml)[i]->move)] == NO_TYPE && SPECIAL((*ml)[i]->move) != PROMOTION && SPECIAL((*ml)[i]->move) != ENPASSANT) {
	
			// Killers (~79 elo)
			if ((*ml)[i]->move == stats.killers[pos->ply][0]) {
				(*ml)[i]->score = first_killer;
			}
			else if ((*ml)[i]->move == stats.killers[pos->ply][1]) {
				(*ml)[i]->score = second_killer;
			}
	
			// Countermoves (~ -10 elo ???)
			//else if (pos->ply > 0 && (*ml)[i]->move == stats.counterMoves[FROMSQ(stats.moves_path[pos->ply - 1])][TOSQ(stats.moves_path[pos->ply - 1])]) {
			//	(*ml)[i]->score = countermove_bonus;
			//}
	
			// History (~243 elo)
			else {
				(*ml)[i]->score = stats.history[pos->side_to_move][FROMSQ((*ml)[i]->move)][TOSQ((*ml)[i]->move)];
			}
		}
	
		// It is a tactical move. We'll score this the highest
		else {
			(*ml)[i]->score = 1000000; // Make sure tactical moves are searched first.
		
			// Captures get scored with MVV/LVA. (~75 elo)
			//if (pos->piece_list[Them][TOSQ((*ml)[i]->move)] != NO_TYPE) {
			//	(*ml)[i]->score += MvvLva[pos->piece_list[pos->side_to_move][FROMSQ((*ml)[i]->move)]][pos->piece_list[Them][TOSQ((*ml)[i]->move)]];
			//}
			
			// MVV/LVA and SEE scoring of captures
			if (pos->piece_list[Them][TOSQ((*ml)[i]->move)] != NO_TYPE) {

				// Score with MVV/LVA if it is a LxH
				if (pos->piece_on(TOSQ((*ml)[i]->move), Them) > pos->piece_on(FROMSQ((*ml)[i]->move), pos->side_to_move)) {
					(*ml)[i]->score += MvvLva[pos->piece_list[pos->side_to_move][FROMSQ((*ml)[i]->move)]][pos->piece_list[Them][TOSQ((*ml)[i]->move)]];
				}
				// Score with SEE if the move seems bad.
				else {
					int score = pos->see((*ml)[i]->move);
				
					if (score >= 0) {
						(*ml)[i]->score += score;
					}
					else {
						(*ml)[i]->score *= -1;
						(*ml)[i]->score += score;
					}
				}
			}

			// Promotion and en-passant scoring (~11 elo)
			if (SPECIAL((*ml)[i]->move) == PROMOTION) {
				// If the promotion piece is a queen or we're capturing a piece, we'll score this just below the hash move
				if (PROMTO((*ml)[i]->move) + 1 == QUEEN) {
					(*ml)[i]->score = hash_move_sort - 100;
				}
				else { // Will be scored as loosing a pawn and gaining the promotion piece.
					(*ml)[i]->score += MvvLva[PAWN][PROMTO((*ml)[i]->move) + 1];
				}
			}
			
			// En-passants will be scored simply as PxP
			if (SPECIAL((*ml)[i]->move) == ENPASSANT) {
				(*ml)[i]->score += MvvLva[PAWN][PAWN];
			}
		
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
	
	// Step 1. Set the new killer moves
	setKillers(pos->ply, best_move);


	// Step 2. Update countermove heuristic
	//if (pos->ply > 0 && stats.moves_path[pos->ply - 1] != MOVE_NULL) { // We need to be at least one ply deep, otherwise we'd index negative array values.
	//	stats.counterMoves[FROMSQ(stats.moves_path[pos->ply - 1])][TOSQ(stats.moves_path[pos->ply - 1])] = best_move;
	//}


	// Step 3. Update history
	int history_bonus = std::min(depth * depth, 400);

	stats.history[pos->side_to_move][FROMSQ(best_move)][TOSQ(best_move)] += history_bonus;

	
	// Decrease history value for all other quiet moves, since they didn't fail high
	unsigned int move = NOMOVE;
	for (int mn = 0; mn < ml->size(); mn++) {
		move = (*ml)[mn]->move;
		if (pos->piece_list[(pos->side_to_move == WHITE) ? BLACK : WHITE][TOSQ(move)] == NO_TYPE && SPECIAL(move) != PROMOTION
			&& SPECIAL(move) != ENPASSANT) { // No piece is captured, not a promotion and not an en-passant

			stats.history[pos->side_to_move][FROMSQ(move)][TOSQ(move)] = std::max(-countermove_bonus, stats.history[pos->side_to_move][FROMSQ(move)][TOSQ(move)] - history_bonus);
		}
	}

	// Handle history table overflows
	if (stats.history[pos->side_to_move][FROMSQ(best_move)][TOSQ(best_move)] >= countermove_bonus) {
	
		for (int i = 0; i < 64; i++) {
	
			for (int j = 0; j < 64; j++) {
	
				stats.history[0][i][j] /= 2;
				stats.history[1][i][j] /= 2;
			}
		}
	}

}


void SearchThread_t::clear_move_heuristics() {

	for (int i = 0; i < 64; i++) {
	
		for (int j = 0; j < 64; j++) {
			stats.counterMoves[i][j] = 0;
	
			stats.history[0][i][j] = 0;
			stats.history[1][i][j] = 0;
		}
	
	}

	for (int d = 0; d < MAXDEPTH; d++) {
		stats.moves_path[d] = 0;
		stats.static_eval[d] = 0;

		stats.killers[d][0] = 0;
		stats.killers[d][1] = 0;
	}
}




void ThreadPool_t::init_threads(GameState_t* pos, SearchInfo_t* info) {

	for (int i = 0; i < threadNum; i++) {
		*threads[i].pos = *pos;
		*threads[i].info = *info;
		threads[i].thread_id = i;
	}

}