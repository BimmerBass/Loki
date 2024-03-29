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
#include "search.h"


static long long reductions = 0;
static long long re_searches = 0;

ThreadPool_t* Search::threads = nullptr;
std::vector<std::thread> Search::threads_running;
std::atomic<bool> Search::isStop(true);


// The checkup function sees if we need to stop the search
void check_stopped_search(SearchThread_t* ss) {
	// Only the 0'th (main) thread should check if the UCI has sent us commands to quit.
	if (ss->thread_id == 0) {
		if (ss->info->timeset && getTimeMs() >= ss->info->stoptime) {
			ss->info->stopped = true;
		}
		ReadInput(ss->info->stopped, ss->info->quit);

		// We've been told to stop, so let's tell the other threads.
		if (ss->info->stopped) {
			Search::isStop = true;
		}
	}
	// All the other threads will check if the time has expired or the main thread has set the isStop flag.
	else {
		if (ss->info->timeset && getTimeMs() >= ss->info->stoptime) {
			ss->info->stopped = true;
		}

		// See if the main thread has told us to stop searching.
		if (Search::isStop.load(std::memory_order_relaxed)) {
			ss->info->stopped = true;
		}
	}
}



void ChangePV(int move, SearchPv* parent, SearchPv* child) {
	parent->length = child->length + 1;
	parent->pv[0] = move;

	// Here we copy the PV that the child node found into the parent while keeping the parent's first move as "move"
	//std::copy(std::begin(child->pv), std::begin(child->pv) + child->length, std::begin(parent->pv) + 1);
	std::copy(child->pv.begin(), child->pv.begin() + child->length, parent->pv.begin() + 1);

	//if (child->length >= MAXDEPTH) {
	//	child->length = MAXDEPTH - 1;
	//}
	//
	//for (int i = 0; i < child->length; i++) {
	//	parent->pv[i + 1] = child->pv[i];
	//}
}


/*

Search parameter formulas

*/

// Here the idea is to increase the null move reduction depending on the lead in evaluation: lead = eval - beta.
int nullmove_reduction(int depth, int lead) {
	// Make sure not to go out of range. This shouldn't happen too often.
	if (lead >= 2000) {
		lead = 1999;
	}
	
	return NM_Reductions[depth][lead];
}


int late_move_reduction(int d, int c) {
	return Reductions[std::min(d, MAXDEPTH - 1)][std::min(c, MAXPOSITIONMOVES - 1)];
}


void lmr_conditions(const SearchThread_t* ss, bool improving, bool capture, bool is_pv, bool gives_check, bool promotes, const Move_t& move, int& R) {
	// Step 1. Initialize some variables.
	int fromSq = FROMSQ(move.move);
	int toSq = TOSQ(move.move);


	// Step 2. Increase reduction if we're not improving.
	R += !improving;

	// Step 3. Increase reduction for moves with bad history.
	// Note: The reason for using the "other side" is because the move already has been played.
	if (ss->stats.history[(ss->pos->side_to_move == WHITE) ? BLACK : WHITE][fromSq][toSq] < 0) {
		R += 1;
	}

	// Step 4. If the move is a capture, increase the reduction if SEE(move) < 0, otherwise decrease it.
	if (capture) {
		if (move.score < 0) { R += 1; }
		else { R -= 2; }
	}

	// Step 5. Decrease if we're in a PV node.
	R -= 2 * is_pv;

	// Step 6. Decrease reduction if the move gives check or promotes a pawn.
	R -= gives_check;
	R -= promotes;

}



int late_move_pruning(int depth, bool improving) {
	return LMP_Limit[std::min(depth, MAXDEPTH)];
}


int futility_margin(int depth, bool improving) {
	return depth * 110 + ((improving) ? 75 : 0);
}

// Determines if a pawn moved from fromSq is a passed one
bool is_passed(int fromSq, GameState_t* pos) {
	assert(pos->piece_list[pos->side_to_move][fromSq] == PAWN);

	return ((BBS::EvalBitMasks::passed_pawn_masks[pos->side_to_move][fromSq] & pos->pieceBBS[PAWN][(pos->side_to_move == WHITE) ? BLACK : WHITE]) == 0) ? true : false;
}

// The razoring margin should rise with depth, and on top of that, we do not want to prune too aggresively if our eval is improving
int razoring_margin(int depth, bool i) {
	return (2 * pawn_value.mg + (depth - 1) * (pawn_value.mg / 2)) + ((i == true) ? 100 : 0);
}




namespace Search {

	// This is the main search function, which initializes all the threads.
	void runSearch(GameState_t* pos, SearchInfo_t* info, int num_threads) {
		if (num_threads < 1) {
			num_threads = 1;
		}

		// Increment transposition table age
		tt->increment_age();

		threads_running.clear();

		ThreadPool_t tp(num_threads);
		threads = &tp;
		threads->init_threads(pos, info);

		// The "accumulated" depth gets incremented for all threads that have an odd thread_id and are not the 0'th
		int acc_depth = info->depth;
		if (num_threads > 1) {

			for (int t = 0; t < threads->count(); t++){
				// Lazy SMP depth variation
				if (t != 0 && t % 2 != 0) {
					acc_depth++;
					(threads->at(t))->info->depth = acc_depth;
				}

				threads_running.push_back(std::thread(searchPosition, threads->at(t)));
			}

			// Wait for all threads to finish
			for (int t = 0; t < threads->count(); t++) {
				threads_running[t].join();
			}
		}
		else {
			searchPosition(threads->at(0));
		}

		
		// If we've been told to quit, it is important to copy this to the info, so we can break out from the UCI loop
		if ((threads->at(0))->info->quit == true) {
			info->quit = true;
		}

		// We save the node-count of the main thread to be used by the benchmarking method
		info->nodes = (threads->at(0))->info->nodes;

		isStop = true;
		threads = nullptr;
	}


	void searchPosition(SearchThread_t* ss) {
		// Clear ss before searching
		clearForSearch(ss);

		// Here we get an estimate of the value of the position. Used for creating the aspiration windows
		SearchPv pvLine;
		int score = alphabeta(ss, 1, -INF, INF, true, &pvLine);
		int best_move = NOMOVE;

		// These are just some parameters to print for UCI
		long long nodes = 0;
		long long time_to_depth = 0;

		double branching_factor = 0.0;

		long long nps = 0;
		double move_ordering = 0;

		double reduction_failed = 0.0;

		long long fh;
		long long fhf;

		// Iterative deepening
		for (int currDepth = 1; currDepth <= ss->info->depth; currDepth++) {
			pvLine.clear();
			ss->info->seldepth = 0; // Clear seldepth

			// Search the position. Use the previous score to center the aspiration windows.
			score = aspiration_search(ss, currDepth, score, &pvLine);

			// If we've been asked to stop, break out of the loop. We don't want the new PV from the lates alphabeta call because the tree hasn't been fully
			// searched, so we'll take the next best, aka last iteration's result.
			if (ss->info->stopped == true || isStop.load() == true) {

				// If this is the first iteration, we need to get the PV move. Otherwise we'd return NOMOVE which is illegal.
				if (currDepth == 1) {
					best_move = pvLine.pv[0];
				}
				
				assert(best_move != NOMOVE);

				// If we're the main thread, we'll tell the other threads to stop searching
				if (ss->thread_id == 0) { isStop = true; }
				break;
			}

			
			nodes = getNodes();
			
			time_to_depth = getTimeMs() - ss->info->starttime;

			branching_factor = std::pow(nodes, 1 / double(currDepth));

			nps = nodes / ((time_to_depth < 1 ? 1 : time_to_depth) / 1000.0); // We need to make sure we don't divide by zero.
			
			// Get the best move from the pvLine stack
			best_move = pvLine.pv[0];

			// Only the "main" thread can print to console
			if (ss->thread_id == 0) {
				std::cout << "info ";

				if (abs(score) > MATE) {
					std::cout << "score mate " << to_mate(score);
				}
				else {
					std::cout << "score cp " << to_cp(score);
				}

				std::cout << " depth " << currDepth
					<< " seldepth " << ss->info->seldepth
					<< " nodes " << nodes
					<< " nps " << nps
					<< " time " << time_to_depth;
				
				std::cout << " pv ";

				// We need to only display the PV containing the mate, if abs(score) > MATE.
				// Otherwise we'd get weird lines from previous PV's Loki has found before seeing the mate.
				for (int n = 0; n < pvLine.length; n++) {
					assert(pvLine.pv[n] != NOMOVE);
					std::cout << printMove(pvLine.pv[n]) << " ";
				}
				std::cout << "\n";

				// Print out the move ordering for debugging
				fh = getFailHigh();
				fhf = getFailHighFirst();
				if (fh != 0 && fhf != 0) {
					move_ordering = (double(fhf) / double(fh)) * 100.0;
				}
				else {
					move_ordering = 0.0;
				}

				if (re_searches == 0) {
					reduction_failed = 0.0;
				}
				else {
					reduction_failed = (reductions == 0) ? 0 : (double(re_searches) / double(reductions)) * 100.0;
				}

				//std::cout << "Move Ordering: " << move_ordering << "%" << std::endl;
				//std::cout << "Branching factor: " << branching_factor << std::endl;
				//std::cout << "LMR re-search: " << reduction_failed << "%" << std::endl;
				//std::cout << "LMR reductions:" << reductions << std::endl;
			}
		} // Iterative deepening end


		if (ss->thread_id == 0) {

			std::cout << "bestmove " << printMove(best_move) << std::endl;
			
			// If the search stopped because the max depth has been reached, we need to stop all other threads.
			isStop = true;
		}
	} // searchPosition




	void clearForSearch(SearchThread_t* ss) {
		ss->pos->ply = 0;

		ss->info->stopped = false;
		ss->info->nodes = 0;

		ss->info->seldepth = 0;

		ss->info->fh = 0;
		ss->info->fhf = 0;

		reductions = 0;
		re_searches = 0;

		for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				ss->stats.history[0][i][j] = 0;
				ss->stats.history[1][i][j] = 0;
				
				ss->stats.counterMoves[i][j] = 0;
			}
		}
		
		for (int d = 0; d < MAXDEPTH; d++) {
			ss->stats.killers[d][0] = NOMOVE;
			ss->stats.killers[d][1] = NOMOVE;
		
			ss->stats.static_eval[d] = 0;
			//ss->moves_path[d] = 0;
		}

	}





	/*
	====================================================================================
	--------------------------------- SEARCH FUNCTIONS ---------------------------------
	====================================================================================
	*/

	// Calls search_root with aspiration windows (~10 elo)
	int aspiration_search(SearchThread_t* ss, int depth, int estimate, SearchPv* line) {
		// Step 1. Initialize some variables
		int score = -INF;

		int alpha_aspirated = -INF;
		int beta_aspirated = INF;

		int delta = aspiration_window;

		// Step 2. Determine whether aspiration windows should be used. Shallow searches are for example so fast that it doesn't make sense.
		// And if we're in a position with mate scores, we need to search with a full window.
		if (depth >= aspiration_depth && abs(estimate) < MATE / 2) {
			alpha_aspirated = std::max(-INF, estimate - delta);
			beta_aspirated = std::min(INF, estimate + delta);
		}

		// Step 3. Search the position with the determined windows.
		while (true) {

			// Step 3A. Search.
			score = search_root(ss, depth, alpha_aspirated, beta_aspirated, line);

			// Step 3A.1. If we've been told to stop, return.
			if (ss->info->stopped) { return 0; }

			// Step 3B. Handle fail-low (score is below alpha)
			if (score <= alpha_aspirated) {
				alpha_aspirated = std::max(-INF, alpha_aspirated - delta);

				continue;
			}

			// Step 3C. Handle fail-high (score is above beta)
			else if (score >= beta_aspirated) {
				beta_aspirated = std::min(INF, beta_aspirated + delta);

				continue;
			}

			// Step 3D. If the score is within the bounds, we can return.
			else {
				break;
			}

			// Step 3E. Increase the aspiration widening
			delta += delta / 2;
		}

		return score;
	}




	// Root alpha beta
	int search_root(SearchThread_t* ss, int depth, int alpha, int beta, SearchPv* pvLine) {
		assert(depth > 0);

		ss->info->nodes++;

		pvLine->clear();
		SearchPv line;
		
		int score = -INF;
		int best_score = -INF;
		int best_move = NOMOVE;
		int new_depth = depth;

		bool raised_alpha = false;
		int legal = 0;


		// Step 1. In-check extensions.
		bool in_check = ss->pos->in_check();
		if (in_check) {
			new_depth++;
		}


		// Step 2. Probe transposition table --> If there is a move from previous iterations, we'll assume the best move from that as the best move now, and
		//	order that first.
		bool ttHit = false;
		EntryData_t* entry = tt->probe_tt(ss->pos->posKey, ttHit);
		unsigned int pvMove = (ttHit) ? entry->get_move() : NOMOVE;


		if (ss->pos->ply >= ss->info->seldepth) {
			ss->info->seldepth = ss->pos->ply;
		}


		// Step 3. Static evaluation
		if (in_check) {
			ss->stats.static_eval[ss->pos->ply] = VALUE_NONE;
		}
		ss->stats.static_eval[ss->pos->ply] = ss->eval->score(ss->pos);


		// Step 4. Initialize a staged move generation object and loop through all moves.
		RootMoveStager stager(ss->pos, &ss->stats, pvMove);

		Move_t move;

		while (stager.next_move(move)) {
			line.clear();

			if (!ss->pos->make_move(&move)) {
				continue;
			}

			legal++;


			// Set the previous move such that we can use the countermove heuristic.
			ss->stats.moves_path[ss->pos->ply] = move.move;


			// Step 5. Principal Variation search. We search all moves with the full window until one raises alpha. Afterwards we'll search with a null window
			// If this is the first legal move
			if (legal == 1) {
				score = -alphabeta(ss, new_depth - 1, -beta, -alpha, true, &line);
			}
			else {
				score = -alphabeta(ss, new_depth - 1, -(alpha + 1), -alpha, true, &line);

				if (score > alpha && score < beta) {
					score = -alphabeta(ss, new_depth - 1, -beta, -alpha, true, &line);
				}
			}

			ss->pos->undo_move();

			if (ss->info->stopped) { return 0; }


			if (score >= beta) { // Fail high
				if (legal == 1) {
					ss->info->fhf++;
				}
				ss->info->fh++;

				tt->store_entry(ss->pos, move.move, beta, depth, ttFlag::BETA);


				return beta;
			}

			if (score > best_score) {
				best_score = score;
				best_move = move.move;

				if (score > alpha) {
					alpha = score;

					raised_alpha = true;

					ChangePV(best_move, pvLine, &line);
				}
			}
		}

		if (legal == 0) {
			if (ss->pos->in_check()) {
				return -INF + ss->pos->ply;
			}
			else {
				return 0;
			}
		}
		
		// If we improved alpha, we're in a PV-node
		if (raised_alpha) {
			assert(best_move == pvLine->pv[0]);
		
			tt->store_entry(ss->pos, best_move, alpha, depth, ttFlag::EXACT);
		}
		else {
			tt->store_entry(ss->pos, best_move, alpha, depth, ttFlag::ALPHA);
		}
	
		return alpha;
	}




	int alphabeta(SearchThread_t* ss, int depth, int alpha, int beta, bool can_null, SearchPv* pvLine) {
		assert(beta > alpha);

		SIDE Us = ss->pos->side_to_move;
		SIDE Them = (Us == WHITE) ? BLACK : WHITE;

		// If we return due to pruning, none of the moves in pvLine should be used by parent.
		pvLine->length = 0;


		// Update seldepth in case we've reached the highest ply so far
		if (ss->pos->ply >= ss->info->seldepth) {
			ss->info->seldepth = ss->pos->ply;
		}


		// Step 1. Dive into quiescence search (~382 elo)
		if (depth <= 0) {
			return quiescence(ss, alpha, beta);
		}

		// Update nodes
		ss->info->nodes++;

		// Check to see if we've been told to abort the search.
		if ((ss->info->nodes & 2047) == 0) {
			check_stopped_search(ss);
		}

		if (ss->info->stopped) {
			return 0;
		}


		// Step 2. Initialization of variables.

		bool root_node = (ss->pos->ply == 0);
		volatile bool is_pv = (beta - alpha == 1) ? false : true; // We are in a PV-node if we aren't in a null window.

		int score = -INF;
		int best_score = -INF;
		int best_move = NOMOVE;

		int old_alpha = alpha;

		int reduction = 0; // Only used in moves_loop

		int new_depth;

		// Create a new PV
		SearchPv line;

		// Idea from stockfish: Are we improving our static evaluations over plies? This can be used for pruning decisions.
		bool improving = false;

		// Flag to trigger futility pruning in moves_loop
		bool futility_pruning = false;

		// Flag to trigger late move pruning of quiet moves.
		bool do_lmp = false;
		
		// Determine if we're in check or not.
		volatile bool in_check = ss->pos->in_check();


		// Step 3. Draw checking and mate-distance pruning
		if (!root_node) {

			// Step 3A. See if the position is drawn by the fifty-move rule or repetition of moves.
			if (ss->pos->is_draw()) {
				return 0;
			}

			// Step 3B. Protect the data structures from overflow if the depth becomes too high
			if (ss->pos->ply >= MAXDEPTH) {
				return ss->eval->score(ss->pos);
			}

			// Step 3C. Mate distance pruning (~0 elo). If there has already been found a forced mate, don't search irrelevant nodes.
			alpha = std::max(-INF + ss->pos->ply, alpha);
			beta = std::min(INF - ss->pos->ply + 1, beta);

			if (alpha >= beta) {
				return alpha;
			}

		}

		// Step 4. Transposition table probing (~30 elo - too little?). This is done before quiescence since it is quite fast, and if we can get a cutoff before
		// going into quiescence, we'll of course use that. Probing before quiescence search contributed with ~17 elo.
		bool ttHit = false;
		EntryData_t* entry = tt->probe_tt(ss->pos->posKey, ttHit);
		
		int ttScore = (ttHit) ? value_from_tt(entry->get_score(), ss->pos->ply) : -INF;
		unsigned int ttMove = (ttHit) ? entry->get_move() : NOMOVE;
		int ttDepth = (ttHit) ? entry->get_depth() : 0;
		int tt_flag = (ttHit) ? entry->get_flag() : ttFlag::NO_FLAG;
		
		// If we're not in a PV-node (beta - alpha == 1), we can do a cutoff if the transposition table returned a valid depth.
		if (ttHit
			&& !is_pv
			&& ttDepth >= depth) {
		
			if (tt_flag == BETA && ttScore >= beta) {
				return beta;
			}
		
			else if (tt_flag == ALPHA && ttScore <= alpha) {
				return alpha;
			}

			else if (tt_flag == EXACT) {
				return ttScore;
			}
		}



		// Step 5. Static evaluation.
		if (in_check) {
			// If we're in check, we'll go directly to the moves since we don't want this branch pruned away.
			ss->stats.static_eval[ss->pos->ply] = VALUE_NONE;
			improving = false;
		
			goto moves_loop;
		}
		
		ss->stats.static_eval[ss->pos->ply] = ss->eval->score(ss->pos);
		//improving = (ss->pos->ply >= 2) ?
		//	(ss->stats.static_eval[ss->pos->ply] >= ss->stats.static_eval[ss->pos->ply - 2] || ss->stats.static_eval[ss->pos->ply - 2] == VALUE_NONE) :
		//	false;
		improving = (ss->pos->ply >= 2) ? (ss->stats.static_eval[ss->pos->ply] > ss->stats.static_eval[ss->pos->ply - 2]) : false;


		// Step 6. Null move pruning (~136 elo). FIXME: Improve safe_nullmove and nullmove_reduction, and set moves_path to MOVE_NULL so no unintentional pruning happens.
		if (can_null && !in_check && !is_pv
			&& depth > 2 && 
			ss->stats.static_eval[ss->pos->ply] >= beta &&
			ss->pos->safe_nullmove()) {
		
			//int R = nullmove_reduction(depth, ss->static_eval[ss->pos->ply] - beta);
		
			int R = 2;
		
			if (depth > 6) {
				R = 3;
		
				// If side to move has two or more pieces, we can extend R since zugzwang chances are _very_ slim.
				//if (countBits(ss->pos->all_pieces[ss->pos->side_to_move] ^ ss->pos->pieceBBS[PAWN][ss->pos->side_to_move] ^ ss->pos->pieceBBS[KING][ss->pos->side_to_move]) >= 2) {
				//	R++;
				//}
			}
			
			int old_evaluation = ss->stats.static_eval[ss->pos->ply];
			int old_enpassant = ss->pos->make_nullmove();
			
			// We want to use another eval here than the one already calculated since the former is inaccurate when the side to move gets switched
			ss->stats.static_eval[ss->pos->ply] = ss->eval->score(ss->pos);
		
			// When we do a nullmove, we can't rely on the countermove heuristic, so we'll have to set the move to indicate NMP usage
			ss->stats.moves_path[ss->pos->ply] = MOVE_NULL;
		
			score = -alphabeta(ss, depth - R - 1, -beta, 1 - beta, false, &line);
			
			// Clear the pv
			line.clear();
		
			// Insert the real evaluation again in case we don't get a cutoff.
			ss->stats.static_eval[ss->pos->ply] = old_evaluation;
			ss->pos->undo_nullmove(old_enpassant);
		
			if (score >= beta && abs(score) < MATE) {
				// Verified null move pruning hasn't been tested properly yet, so it is left out until there is time for tests with longer tc's
				// Step 6A. Verified Null Move Pruning. For high depths, we will want to do a verification search with a null window centered around beta to be sure
				//if (depth >= 8) {
				//	// This time, the score is not inside a "make/undo" move, so we shouldn't make it negative or switch the bounds
				//	score = alphabeta(ss, depth - R - 1, beta - 1, beta, false, &line);
				//
				//	if (score >= beta && abs(score < MATE)) { // If we're still above beta, it is safe to say, that our null move is good enough
				//		return beta;
				//	}
				//}
				//
				//else {
				//	return beta;
				//}
				return beta;
			}
		}
		
		
		
		// Step 7. Enhanced futility pruning (~105 elo). If our position seems so bad that it can't possibly raise alpha, we can set a futility_pruning flag
		//		and skip tactically boring moves from the search
		if (depth < 7 && !in_check && !is_pv
			&& abs(alpha) < MATE && abs(beta) < MATE
			&& ss->stats.static_eval[ss->pos->ply] + futility_margin(depth, improving) <= alpha) {
		
			futility_pruning = true;
		}
		
		
		// Step 8. Reverse futility pruning (~30 elo). If our static evaluation beats beta by the futility margin, we can most likely just return beta.
		if (depth < 7 && !in_check && !is_pv
			&& abs(alpha) < MATE && abs(beta) < MATE) {
		
			int margin = 175 * depth - ((improving) ? 75 : 0);
			
			if (ss->stats.static_eval[ss->pos->ply] - margin >= beta) {
				return beta;
			}
		}
		
		
		// Step 9. Razoring (~36 elo)
		if (use_razoring && depth <= razoring_depth && !is_pv &&
			ss->stats.static_eval[ss->pos->ply] + razoring_margin(depth, improving) <= alpha
			&& !in_check && abs(beta) < MATE && abs(alpha) < MATE && ss->pos->non_pawn_material()) {

			if (depth == 1) {
				return quiescence(ss, alpha, beta);
			}
		
			int razor_window = alpha - razoring_margin(depth, improving);
		
			score = quiescence(ss, razor_window, razor_window + 1);
		
			// If we couldn't raise the score over alpha - margin, this node is very likely to be an ALL-node
			if (score <= razor_window) {
		
				return alpha;
			}
		}


		moves_loop:

		// Initialize a movestager object.
		MoveStager stager(ss->pos, &ss->stats, (ttHit) ? ttMove : NOMOVE, in_check);

		// Step 10. Internal Iterative Deepening (IID) (~21 elo): If the transposition table didn't return a move, we'll search the position to a shallower
		//		depth in the hopes of finding the PV.
		// This will only be done if we are in a PV-node and at a high depth. At low depths, the search is very fast anyways.
		// We also won't do IID when in check because these usually only have a few moves, which would make the best one pretty fast to find.
		//if (!ttHit && !in_check && depth >= iid_depth) {
		//	assert(iid_depth >= iid_reduction);
		//	assert(ttMove == NOMOVE);
		//
		//	// Step 11A. Do a reduced depth search.
		//	//new_depth = depth - iid_reduction;
		//	new_depth = depth - (depth / 4) - 1;
		//
		//	score = alphabeta(ss, new_depth, alpha, beta, true, &line);
		//
		//	// Now we'll set the ttHit and ttMove if we found a good move.
		//	//if (line.pv[0] != NOMOVE) {
		//	//	ttHit = true;
		//	//	ttMove = line.pv[0];
		//	//}
		//
		//	// Step 11B. Probe the transposition table to see if we have found a (probably) best move.
		//	entry = tt->probe_tt(ss->pos->posKey, ttHit);
		//
		//	int ttScore = (ttHit) ? value_from_tt(entry->score, ss->pos->ply) : -INF;
		//	unsigned int ttMove = (ttHit) ? entry->move : NOMOVE;
		//	int ttDepth = (ttHit) ? entry->depth : 0;
		//	int tt_flag = (ttHit) ? entry->flag : ttFlag::NO_FLAG;
		//
		//	line.clear();
		//}

		Move_t current_move;
		int move = NOMOVE;
		int legal = 0;
		int moves_searched = 0;

		while(stager.next_move(current_move)) {
			move = current_move.move;
			
			// Most of the below will first be used when adding proper LMR and LMP, and thus they're commented out.
			bool capture = (ss->pos->piece_list[Them][TOSQ(move)] != NO_TYPE) ? true : false;

			int extensions = 0;


			// Step 11. Check extensions. Singular extensions will also be added here in the future.
			
			if (in_check) { // In check extensions (~22 elo)
				extensions++;
			}

			// Make the move.
			if (!ss->pos->make_move(&current_move)) {
				continue;
			}
			
			// Increment legal when we've made a move. This is used so as to not prune potential checkmates or stalemates.
			legal++;

			bool gives_check = ss->pos->in_check(); // FIXME: Add function to determine if a move gives check before making it.
			bool is_tactical = capture || gives_check || in_check || SPECIAL(move) == PROMOTION || SPECIAL(move) == ENPASSANT;

			// Set the move we're searching for use in the countermove heuristic.
			ss->stats.moves_path[ss->pos->ply] = move;


			// Step 12. If we are allowed to use futility pruning, and this move is not tactically significant, prune it.
			//			We just need to make sure that at least one legal move has been searched since we'd risk getting false mate scores else.
			if (futility_pruning && 
				(!is_tactical || (depth <= 1 && current_move.score < 0)) // If we're at a pre-frontier node, we'll also prune moves that are deemed to be bad.
				&& legal > 0) {
				ss->pos->undo_move();
				continue;
			}

			// Step 13. Late move pruning. If we have searched a number of moves (dependent on depth - rises exponentially) and haven't gotten a beta cutoff
			//			chances are that we won't get one with the quiets. Therefore, if they meet certain criteria, we skip them.

			if (do_lmp && !is_tactical) { // do_lmp is only set if we're not in a pv-node or root node, so we don't need to check this here.
				ss->pos->undo_move();
				continue;
			}
			else if (!is_tactical && !is_pv && !root_node && best_score > -MATE // We need to have raised alpha at least once.
				&& moves_searched > late_move_pruning(depth, improving)) {
				do_lmp = true;

				ss->pos->undo_move();
				continue;
			}


			// Step 14. Principal variation search: Always search the first move at full depth, with a full window.
			
			new_depth = depth - 1 + extensions;

			if (moves_searched == 0) {
				score = -alphabeta(ss, new_depth, -beta, -alpha, true, &line);
			}
			else {
				// Step 14A. Late move reductions (~107 elo). If we haven't raised alpha yet, we're probably in an ALL-node,
				//	so we'll reduce the search depth and do a full-depth re-search if the score is (surprisingly) above alpha
				if (moves_searched >= lmr_limit && depth >= lmr_depth && !in_check && !root_node) {
					// Step 14A.1. Initialize the base reduction from a pre-calculated table.
					int R = late_move_reduction(depth, moves_searched);

					// Step 14A.2. Increase/Decrease the reduction based on different conditions.
					lmr_conditions(ss, improving, capture, is_pv, gives_check, SPECIAL(move) == PROMOTION, current_move, R);

					// Step 14A.3. We don't want to go directly into qsearch or search to a higher depth than d - 1.
					int d = std::clamp(depth - 1 - R, 1, depth - 1);

					// Step 14A.4. Now search the move in a null-window centered around alpha.
					score = -alphabeta(ss, d, -(alpha + 1), -alpha, true, &line);
				}
				else {	/* Hack to enter normal search in case LMR isn't applicable */
					score = alpha + 1;
				}

				// Step 14B. If we couldn't do LMR, or the reduced search returned a value above alpha, do a normal search.
				if (score > alpha) {
					score = -alphabeta(ss, new_depth, -(alpha + 1), -alpha, true, &line);

					if (score > alpha && score < beta) { // If we raised alpha and stayed inside the bounds, re-search with a full window.
						score = -alphabeta(ss, new_depth, -beta, -alpha, true, &line);
					}
				}

			}


			// Undo the move and increment the moves_searched counter.
			ss->pos->undo_move();
			moves_searched++;

			if (ss->info->stopped) { return 0; }

			if (score >= beta) {
				if (moves_searched == 1) {
					ss->info->fhf++;
				}
				ss->info->fh++;

				// Step 14C. If a beta cutoff was achieved, update the quit move ordering heuristics 
				if (!capture && SPECIAL(move) != PROMOTION && SPECIAL(move) != ENPASSANT) {
					ss->update_move_heuristics(move, depth, stager.get_moves());
				}
				
				
				tt->store_entry(ss->pos, move, beta, depth, ttFlag::BETA);

				return beta;
			}


			if (score > best_score) {
				best_score = score;
				best_move = move;

				if (score > alpha) {
					alpha = score;

					// Change PV
					ChangePV(best_move, pvLine, &line);
				}
			}
		}

		// Step 15. Checkmate/Stalemate detection.
		if (legal <= 0) {
			if (ss->pos->in_check()) {
				return -INF + ss->pos->ply;
			}
			else {
				return 0;
			}
		}

		
		if (alpha > old_alpha) {
			tt->store_entry(ss->pos, best_move, alpha, depth, ttFlag::EXACT);

		}
		else{
			tt->store_entry(ss->pos, best_move, alpha, depth, ttFlag::ALPHA);
		}


		return alpha;
	}


	const int delta_piece_value[5] = { 100, 310, 350, 560, 1000 };

	int quiescence(SearchThread_t* ss, int alpha, int beta) {
		assert(beta > alpha);
		
		ss->info->nodes++;

		if ((ss->info->nodes & 2047) == 0) {
			check_stopped_search(ss);
		}

		if (ss->info->stopped) {
			return 0;
		}

		// Step 1. Check for a draw, and return immediately
		if (ss->pos->is_draw() && ss->pos->ply) {
			return 0;
		}

		if (ss->pos->ply >= MAXDEPTH) {
			return ss->eval->score(ss->pos);
		}

		// Step 2. Static evaluation and possible cutoff if this beats beta.
		int stand_pat = ss->eval->score(ss->pos);

		assert(stand_pat > -MATE && stand_pat < MATE);

		if (stand_pat >= beta) {
			return beta;
		}

		if (alpha < stand_pat) {
			alpha = stand_pat;
		}



		int score = -INF;
		
		// If we're in check, we'll generate all moves, since we would otherwise give false mate scores if no captures were available to evade the check
		// (~20 elo)
		bool in_check = ss->pos->in_check();



		// Step 3. Delta pruning (~10 elo). If our position is so bad that not even the best capture possible would be enough to raise alpha, we'll assume that it is an all-node.
		//if (stand_pat + std::max(delta_margin, ss->pos->best_capture_possible()) <= alpha && !in_check) {
		//	return alpha;
		//}


		// Step 4. Generation of moves
		MoveStager stager(ss->pos);

		int legal = 0;
		int move = NOMOVE;
		Move_t current_move;

		while(stager.next_move(current_move, true)) {
			
			move = current_move.move;
			int piece_captured = ss->pos->piece_list[(ss->pos->side_to_move == WHITE) ? BLACK : WHITE][TOSQ(move)];

			// Step 5. SEE pruning (~56 elo). If the move is a capture and SEE(move) < 0 (we know this if move->score < 0 for captures), just prune it.
			if (piece_captured != NO_TYPE && current_move.score < 0) {
				continue;
			}
			
			// Step 6. Futility pruning (~30 elo). If the value of the piece captured, plus some margin (~200cp) is still not enough to raise alpha, we won't bother searching it.
			// We'll just have to make sure, that there has been tested at least one legal move, so we don't miss a checkmate
			//if (SPECIAL(move) != PROMOTION && SPECIAL(move) != ENPASSANT && piece_captured != NO_TYPE &&
			//	stand_pat + delta_piece_value[piece_captured] + delta_margin <= alpha
			//	&& !ss->pos->is_endgame()) {
			//	continue;
			//}


			if (!ss->pos->make_move(&current_move)) {
				continue;
			}
			legal++;


			score = -quiescence(ss, -beta, -alpha);

			ss->pos->undo_move();

			if (ss->info->stopped) {
				return 0;
			}

			if (score >= beta) {
				if (legal == 1) {
					ss->info->fhf++;
				}
				ss->info->fh++;


				return beta;
			}

			if (score > alpha) {
				alpha = score;
			}
		}


		return alpha;
	}




	namespace Debug {

		int MDTF(SearchThread_t* ss, int estimate, int depth, SearchPv* pvLine) {
			int g = estimate;

			int lower = -INF;
			int upper = INF;

			while (lower < upper) {
				int beta = std::max(g, lower + 1);

				g = search_root(ss, depth, beta - 1, beta, pvLine);

				if (g < beta) {
					pvLine->length = 0;
					upper = g;
				}
				else {
					lower = g;
				}
			}

			return g;
		}

	}



} // namespace Search



SearchInfo_t::SearchInfo_t(const SearchInfo_t& s) {
	starttime = s.starttime;
	stoptime = s.stoptime;
	
	depth = s.depth;
	depthset = s.depthset;

	timeset = s.timeset;
	movestogo = s.movestogo;

	infinite = s.infinite;

	nodes = s.nodes;
	
	quit = s.quit;
	stopped = s.stopped;

	fh = s.fh;
	fhf = s.fhf;
}



/*
====================================================================================
--------------------------------- HELPER FUNCTIONS ---------------------------------
====================================================================================
*/



long long getNodes() {
	long long n = 0;
	for (int i = 0; i < Search::threads->count(); i++) {
		n += (Search::threads->at(i))->info->nodes;
	}
	return n;
}

long long getFailHigh() {
	long long n = 0;
	for (int i = 0; i < Search::threads->count(); i++) {
		n += (Search::threads->at(i))->info->fh;
	}
	return n;
}

long long getFailHighFirst() {
	long long n = 0;
	for (int i = 0; i < Search::threads->count(); i++) {
		n += (Search::threads->at(i))->info->fhf;
	}
	return n;
}


void uci_moveinfo(int move, int depth, int index) {
	std::string moveStr = printMove(move);

	std::cout << "info depth " << depth << " currmove " << moveStr << " currmovenumber " << index << std::endl;
}


int to_cp(int score) {
	return score * (100 / pawn_value.mg);
}

int to_mate(int score) {
	return (score > MATE) ? (INF - abs(score)) / 2 + ((abs(score) % 2 != 0) ? 1 : 0) :
		-(INF - abs(score)) / 2 + ((abs(score) % 2 != 0) ? 1 : 0);
}


/*
====================================================================================
---------------------------------- INITIALIZATION ----------------------------------
====================================================================================
*/
int MvvLva[6][6] = { {0} };

int Reductions[MAXDEPTH][MAXPOSITIONMOVES] = { {0} };
int LMP_Limit[MAXDEPTH] = { 0 };

int NM_Reductions[MAXDEPTH][2000] = { {0} };

void Search::INIT() {

	// Initialize MvvLva.
	int victim_scores[6] = { 100, 200, 300, 400, 500, 600 };

	for (int victim = KING; victim >= PAWN; victim--) {
		for (int attacker = KING; attacker >= PAWN; attacker--) {

			MvvLva[attacker][victim] = victim_scores[victim] + (6 - attacker);

		}
	}

	// Initialize table of late move reductions
	for (double d = 1; d < MAXDEPTH; d += 1.0) {

		for (double c = 1; c < MAXPOSITIONMOVES; c += 1.0) {

			//Reductions[(int)d][(int)c] = 1.75 + (int)std::round((std::log(3.0 * d) * std::log(3.0 * c)) / 5.5);
			Reductions[(int)d][(int)c] = (int)std::round((std::log(2.0 * d) * std::log(2.0 * c)) / 5.5);

			if (Reductions[(int)d][(int)c] < 1) {
				Reductions[(int)d][(int)c] = 1;
			}

		}
	}

	// Initialize table of move-count limits for late move pruning
	LMP_Limit[0] = 0; // We won't be using LMP for depth = 0 anyways..

	for (double d = 1.0; d < (double)MAXDEPTH; d += 1.0) {
		// The below formula has been fitted using exponential regression to the following values:
		/*
			(depth, move count limit)
			(1, 3)
			(2, 5)
			(3, 8)
			(4, 14)
			(5, 25)
		*/
		// It has a fit of R^2 = 0.9968
		// Note: Since the growth is exponential, we'll cut it at 256 (max moves in a position).
		LMP_Limit[(int)d] = static_cast<int>(std::min(uint64_t(std::round(1.73 * std::exp(0.53 * d))), (uint64_t)MAXPOSITIONMOVES));
	}

	// Initialize null move R-value table
	for (int d = 0; d < MAXDEPTH; d++) {

		for (int lead = 0; lead < 2000; lead++) {
			// This is set so as to not reduce by more than six plies under any circumstance
			//NM_Reductions[d][lead] = ((d > 6) ? 3 : 2) + std::max(0, std::min(3, (int)std::round(1.5 * std::log(std::pow(lead / 100, 2)))));
			NM_Reductions[d][lead] = (int)std::round(1.5 + 0.25 * double(d) + std::min(3.0, double(lead) / (2.0 * (double)pawn_value.mg)));
		}
	}
}
