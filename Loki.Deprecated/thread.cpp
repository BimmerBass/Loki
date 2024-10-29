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
#include "thread.h"



/*

Clear a search info object.

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

Set a new killer move. This will push back the previously best killer move.

*/
void SearchThread_t::setKillers(int ply, int move) {
	// If the move is already the first killer, don't add it since it'll just result in duplicate killers
	if (stats.killers[ply][0] != move) {
		stats.killers[ply][1] = stats.killers[ply][0];
		stats.killers[ply][0] = move;
	}
}




/*

Update the move ordering heuristics. This function is called when a beta cutoff occurs.

*/
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



/*

Clear all the move ordering heuristics.

*/
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



/*

Initialize all the SearchThread_t objects.

*/
void ThreadPool_t::init_threads(GameState_t* pos, SearchInfo_t* info) {

	for (int i = 0; i < threadNum; i++) {
		*threads[i].pos = *pos;
		*threads[i].info = *info;
		threads[i].thread_id = i;
	}

}