#include "search.h"
#include <fstream>

static long long reductions = 0;
static long long re_searches = 0;

ThreadPool_t* Search::threads = nullptr;


// The checkup function sees if we need to stop the search
void CheckUp(SearchThread_t* ss) {
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

	parent->pv[0] = move;
	memcpy(parent->pv + 1, child->pv, child->length * sizeof(int));
	parent->length = child->length + 1;
}


inline int value_to_tt(int score, int ply) {
	return (score > MATE) ? score + ply : ((score < -MATE) ? score - ply : score);
}


/*

Search parameter formulas

*/

// Here the idea is to increase the null move reduction depending on the lead in evaluation: lead = eval - beta.
// The exact formula is R(d, l) = 0.5 * d + 1.5 * ln(l)
int nullmove_reduction(int depth, int lead) {
	
	//return std::round((depth / 2) + 1.5 * std::log(double(lead)));
	if (lead >= 100) { // For lead < 100 we'd get negative values for ln(lead)
		return ((depth > 6) ? 3 : 2) + std::round(1.5 * std::log(double(lead) / 100));
	}
	return (depth > 6) ? 3 : 2;
}


int late_move_reduction(int d, int c, int pv, int i) {
	if (d < 1) {
		return 1;
	}

	int R = Reductions[d][c];

	// Increase reduction if we're not improving
	R += (!i) ? 1 : 0;

	// Decrease reduction if we're in a PV node
	R /= (pv) ? 2 : 1;

	return R;
}


int late_move_pruning(int depth, bool improving) {
	return std::round((4.0 * std::exp(0.37 * double(depth))) * ((1.0 + ((improving) ? 1.0 : 0.0)) / 2.0));
	//return std::round((4.0 * std::exp(0.51 * double(depth))) * ((1.0 + ((improving) ? 1.0 : 0.0)) / 2.0));
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
	return (2 * Eval::pawnValMg + (depth - 1) * (Eval::pawnValMg / 2)) + ((i == true) ? 100 : 0);
}




namespace Search {

	// This is the main search function, which initializes all the threads.
	void runSearch(GameState_t* pos, SearchInfo_t* info, int num_threads) {
		if (num_threads < 1) {
			num_threads = 1;
		}

		isStop = false;
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


		threads = nullptr;
	}


	void searchPosition(SearchThread_t* ss) {
		// Clear ss before searching
		clearForSearch(ss);

		// Here we get an estimate of the value of the position. Used for creating the aspiration windows
		SearchPv guess_pv;
		int score = alphabeta(ss, 1, -INF, INF, true, &guess_pv);
		int best_move = guess_pv.pv[0];

		// These are just some parameters to print for UCI
		long long nodes = 0;

		double branching_factor = 0.0;

		long long nps = 0;
		long long move_ordering = 0;

		double reduction_failed = 0.0;

		long long fh;
		long long fhf;

		// Iterative deepening
		for (int currDepth = 1; currDepth <= ss->info->depth; currDepth++) {
			SearchPv pvLine;

			// Search the position. Use the previous score to center the aspiration windows.
			score = aspiration_search(ss, currDepth, score, &pvLine);

			// If we've been asked to stop, break out of the loop. We don't want the new PV from the lates alphabeta call because the tree hasn't been fully
			// searched, so we'll take the next best, aka last iteration's result.
			if (ss->info->stopped == true || isStop.load() == true) {

				// If this is the first iteration, we need to get the PV move. Otherwise we'd return NOMOVE which is illegal.
				if (currDepth == 1) {
					best_move = pvLine.pv[0];
				}
				

				// If we're the main thread, we'll tell the other threads to stop searching
				if (ss->thread_id == 0) { isStop = true; }
				break;
			}

			
			nodes = getNodes();

			branching_factor = std::pow(nodes, 1 / double(currDepth));

			nps = nodes / ((getTimeMs() - ss->info->starttime) / 1000.0);

			// Get the best move from the pvLine stack
			best_move = pvLine.pv[0];

			// Only the "main" thread can print to console
			if (ss->thread_id == 0) {
				std::cout << "info ";

				if (abs(score) > MATE) {
					std::cout << "score mate " << to_mate(score);

					// Here we find the right pvLine length for the mate.
					pvLine.length = (INF - abs(score));

				}
				else {
					std::cout << "score cp " << to_cp(score);
				}

				std::cout << " depth " << currDepth
					<< " nodes " << nodes
					<< " nps " << nps
					<< " time " << getTimeMs() - ss->info->starttime;
				
				std::cout << " pv ";

				// We need to only display the PV containing the mate, if abs(score) > MATE.
				// Otherwise we'd get weird lines from previous PV's Loki has found before seeing the mate.
				for (int n = 0; n < pvLine.length; n++) {
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
					reduction_failed = (double(re_searches) / double(reductions)) * 100.0;
				}

				std::cout << "Move Ordering: " << move_ordering << "%" << std::endl;
				std::cout << "Branching factor: " << branching_factor << std::endl;
				std::cout << "LMR re-search: " << reduction_failed << "%" << std::endl;
				std::cout << "LMR reductions:" << reductions << std::endl;
			}
		} // Iterative deepening end


		if (ss->thread_id == 0) {

			// We need to set the isStop flag if we're done searching
			isStop = true;

			std::cout << "bestmove " << printMove(best_move) << std::endl;
		}
	} // searchPosition




	void clearForSearch(SearchThread_t* ss) {
		ss->pos->ply = 0;

		ss->info->stopped = false;
		ss->info->nodes = 0;
		ss->info->fh = 0;
		ss->info->fhf = 0;

		reductions = 0;
		re_searches = 0;

		for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				ss->history[0][i][j] = 0;
				ss->history[1][i][j] = 0;

				ss->counterMoves[i][j] = 0;
			}
		}

		for (int d = 0; d < MAXDEPTH; d++) {
			ss->killers[d][0] = NOMOVE;
			ss->killers[d][1] = NOMOVE;

			ss->static_eval[d] = 0;
			ss->moves_path[d] = 0;
		}

	}





	/*
	====================================================================================
	--------------------------------- SEARCH FUNCTIONS ---------------------------------
	====================================================================================
	*/

	// Calls search_root with aspiration windows
	int aspiration_search(SearchThread_t* ss, int depth, int estimate, SearchPv* line) {
		int score = -INF;
		
		int alpha = -INF;
		int beta = INF;

		int delta = aspiration_window;

		// Only use narrow windows at high depths and if there are no mate scores
		if (depth > aspiration_depth && abs(estimate) < MATE) {
			alpha = std::max(alpha, estimate - delta);
			beta = std::min(beta, estimate + delta);
		}

	asp_loop:
		line->length = 0;
		score = search_root(ss, depth, alpha, beta, line);


		/*
		Disclaimer: The idea of widening the bounds and window sizes, especially changing beta in a fail low is taken from Stockfish.
				The values has been tweaked but I am not the creator of the method.
		*/

		if (score <= alpha) {
			beta = (alpha + beta) / 2; // We move beta to the average of the bounds since we could possibly move alpha by a large margin and end up searching
										// a huge window. 
			alpha = std::max(-INF, score - delta);

			// Increase the window size.
			delta += std::round(0.25 * double(aspiration_window));

			goto asp_loop;
		}

		if (score >= beta) {
			// Only increase the bound that failed.
			beta = std::min(INF, score + delta);

			// Increase the window size.
			delta += std::round(0.25 * double(aspiration_window));
			
			goto asp_loop;
		}

		return score;
	}




	// Root alpha beta
	int search_root(SearchThread_t* ss, int depth, int alpha, int beta, SearchPv* pvLine) {
		assert(depth > 0);

		SearchPv line;

		ss->info->nodes++;
		
		int score = -INF;
		int best_move = NOMOVE;

		bool raised_alpha = false;
		int legal = 0;

		MoveList* moves = ss->generate_moves();


		// Step 1. In-check extensions.
		bool in_check = ss->pos->in_check();
		if (in_check) {
			depth++;
		}

		
		// Steo 2. Static evaluation.
		if (in_check) {
			ss->static_eval[ss->pos->ply] = VALUE_NONE;
		}
		ss->static_eval[ss->pos->ply] = Eval::evaluate(ss->pos);


		// Step 3. Probe transposition table --> If there is a move from previous iterations, we'll assume the best move from that as the best move now, and
		//	order that first.
		bool ttHit = false;
		TT_Entry* entry = tt->probe_tt(ss->pos->posKey, ttHit);
		int pvMove = NOMOVE;

		if (ttHit) {
			pvMove = entry->data.move;

			// Loop through the move list and find the pvMove
			for (int m = 0; m < moves->size(); m++) {
				if ((*moves)[m]->move == pvMove) {
					(*moves)[m]->score = hash_move_sort;
					break;
				}
			}
		}


		// Now we'll loop through the move list.
		for (int m = 0; m < moves->size(); m++) {
			ss->pickNextMove(m, moves);
			
			int move = (*moves)[m]->move;
			

			if (!ss->pos->make_move((*moves)[m])) {
				continue;
			}
			
			legal++;

			
			// Print move, move number and such to the command line.
			if (ss->thread_id == 0) {
				uci_moveinfo(move, depth, legal);
			}

			int new_depth = depth - 1;

			// Set the previous move such that we can use the countermove heuristic.
			ss->moves_path[ss->pos->ply] = move;

			// Step 3. Principal Variation search. We search all moves with the full window until one raises alpha. Afterwards we'll search with a null window
			//		and only widen it if the null window search raises alpha, which is assumed unlikely.
			if (!raised_alpha) {
				score = -alphabeta(ss, new_depth, -beta, -alpha, true, &line);
			}
			else {
				score = -alphabeta(ss, new_depth, -alpha - 1, -alpha, true, &line);

				if (score > alpha) {
					score = -alphabeta(ss, new_depth, -beta, -alpha, true, &line);
				}
			}


			ss->pos->undo_move();

			if (score >= beta) {
				if (legal == 1) {
					ss->info->fhf++;
				}
				ss->info->fh++;

				tt->store_entry(ss->pos, move, score, depth, BETA);

				delete moves;

				return beta;
			}

			if (score > alpha) {
				best_move = move;
				alpha = score;
				raised_alpha = true;

				ChangePV(best_move, pvLine, &line);
			}

		}

		// Delete the movelist when we're done with it.
		delete moves;

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

			tt->store_entry(ss->pos, best_move, alpha, depth, EXACT);
		}
		else {
			tt->store_entry(ss->pos, best_move, alpha, depth, ALPHA);
		}
	
		return alpha;
	}




	int alphabeta(SearchThread_t* ss, int depth, int alpha, int beta, bool can_null, SearchPv* pvLine) {
		assert(beta > alpha);

		SIDE Us = ss->pos->side_to_move;
		SIDE Them = (Us == WHITE) ? BLACK : WHITE;

		SearchPv line;

		// Step 1. Transposition table probing. This is done before quiescence since it is quite fast, and if we can get a cutoff before going into quiescence,
		//		we'll of course use that. Probing before quiescence search contributed with ~17 elo.
		bool ttHit = false;
		TT_Entry* entry = tt->probe_tt(ss->pos->posKey, ttHit);

		int ttScore = (ttHit) ? entry->data.score : -INF;
		int ttMove = (ttHit) ? entry->data.move : NOMOVE;
		int ttDepth = (ttHit) ? entry->data.depth : 0;
		int ttFlag = (ttHit) ? entry->data.flag : NO_FLAG;

		if (ttScore > MATE && ttHit) {
			ttScore -= ss->pos->ply;
		}
		else if (ttScore < -MATE && ttHit) {
			ttScore += ss->pos->ply;
		}

		if (ttHit && ttDepth >= depth) {
			switch (ttFlag) {
			case ALPHA:
				if (ttScore <= alpha) {
					return alpha;
				}
				break;
			case BETA:
				if (ttScore >= beta) {
					return beta;
				}
				break;
			case EXACT:
				return ttScore;
			}
		}


		if (depth <= 0) {
			pvLine->length = 0;
			return quiescence(ss, alpha, beta);
		}
		ss->info->nodes++;


		if ((ss->info->nodes & 2047) == 0) {
			CheckUp(ss);
		}

		if (ss->info->stopped) {
			return 0;
		}


		// Step 2. Repetition checking. If this position has been reached before, it can be drawn
		if (ss->pos->is_repetition()) {
			return 0;
		}

		// Protect the data structures from overflow if the depth becomes too high
		if (ss->pos->ply >= MAXDEPTH) {
			return Eval::evaluate(ss->pos);
		}


		int score = -INF;
		bool raised_alpha = false;
		int best_move = NOMOVE;

		int reduction = 0; // Only used in moves_loop

		int new_depth;

		// We are in a PV-node if we aren't in a null window.
		// In a null window, beta = alpha + 1, which means that beta - alpha = 1, so if this isn't true, we're not in a null window.
		bool is_pv = (beta - alpha == 1) ? false : true;

		// Idea from stockfish: Are we improving our static evaluations over plies? This can be used for pruning decisions.
		bool improving = false;

		// Flag to trigger futility pruning in moves_loop
		bool futility_pruning = false;

		// Flag to set late move pruning.
		bool lmp = false;

		// Determine if we're in check or not.
		bool in_check = ss->pos->in_check();



		// Step 3. Mate distance pruning (~11 elo). If there has already been found a forced mate, don't search irrelevant nodes.
		alpha = std::max(-INF + ss->pos->ply, alpha);
		beta = std::min(INF - ss->pos->ply + 1, beta);
		
		if (alpha >= beta) {
			return alpha;
		}


		// Step 4. Static evaluation.
		if (in_check) {
			// If we're in check, we'll go directly to the moves since we don't want this branch pruned away.
			ss->static_eval[ss->pos->ply] = VALUE_NONE;
			improving = false;

			goto moves_loop;
		}
		
		ss->static_eval[ss->pos->ply] = Eval::evaluate(ss->pos);
		improving = (ss->pos->ply >= 2) ? (ss->static_eval[ss->pos->ply] >= ss->static_eval[ss->pos->ply - 2] || ss->static_eval[ss->pos->ply - 2] == VALUE_NONE) :
			false;

		assert(!in_check);


		// Step 5. Null move pruning (~62 elo). FIXME: Improve safe_nullmove
		if (can_null && !in_check && !is_pv
			&& depth > 2 && 
			ss->static_eval[ss->pos->ply] >= beta &&
			ss->pos->safe_nullmove()) {
		
			int R = nullmove_reduction(depth, ss->static_eval[ss->pos->ply] - beta);


			int old_enpassant = ss->pos->make_nullmove();
			
			assert(depth - reduction - 1 >= 0);
		
			score = -alphabeta(ss, depth - R - 1, -beta, 1 - beta, false, &line);
			line.length = 0;
		
			ss->pos->undo_nullmove(old_enpassant);
		
			if (score >= beta && abs(score) < MATE) {
				return beta;
			}
		}



		// Step 6. Enhanced futility pruning (~54 elo). If our position seems so bad that it can't possibly raise alpha, we can set a futility_pruning flag
		//		and skip tactically boring moves from the search
		if (depth < 7 && !in_check && !is_pv
			&& abs(alpha) < MATE && abs(beta) < MATE
			&& ss->static_eval[ss->pos->ply] + futility_margin(depth, improving) <= alpha) {
		
			futility_pruning = true;
		}


		// Step 7. Reverse futility pruning (~13 elo). If our static evaluation beats beta by the futility margin, we can most likely just return beta.
		if (depth < 7 && !in_check && !is_pv
			&& abs(alpha) < MATE && abs(beta) < MATE) {
		
			int margin = 110 * depth + ((improving) ? 75 : 0);
		
			if (ss->static_eval[ss->pos->ply] - margin >= beta) {
				return beta;
			}
		}


		// Step 8. Razoring (~31 elo)
		if (use_razoring && depth <= razoring_depth && !is_pv &&
			ss->static_eval[ss->pos->ply] + razoring_margin(depth, improving) <= alpha
			&& !in_check && abs(beta) < MATE) {
		
			int razoring_window = alpha - razoring_margin(depth, improving);
			score = quiescence(ss, razoring_window, razoring_window + 1); // Do a null window quiescence to prove we can't raise alpha over  alpha - margin
			//score = quiescence(ss, alpha, beta);
		
			if (score + razoring_margin(depth, improving) <= alpha) {// && depth - 1 <= 0) {
				// Even though we're in a fail-hard negamax framework, we'll return the score here since razoring is supposed to act as an early quiescence search.
				return score; 
			}
		}


		moves_loop:

		MoveList* moves = ss->generate_moves();


		// Step 9. Internal Iterative Deepening (IID) (~5 elo): If the transposition table didn't return a move, we'll search the position to a shallower
		//		depth in the hopes of finding the PV.
		// This will only be done if we are in a PV-node and at a high depth. At low depths, the search is very fast anyways.
		if (!ttHit && is_pv && depth > iid_depth) {
			assert(iid_depth >= iid_reduction);
			assert(ttMove == NOMOVE);
		
			// We'll reduce the depth.
			new_depth = depth - iid_reduction;
		
			SearchPv iid_pv;
		
			score = alphabeta(ss, new_depth, alpha, beta, true, &iid_pv);
		
			// Now we'll set the ttHit and ttMove if we found a good move.
			if (iid_pv.pv[0] != NOMOVE) {
				ttHit = true;
				ttMove = iid_pv.pv[0];
			}
		}


		// If the transposition table returned a move, this is probably the best, so we'll score it highest.
		if (ttHit && ttMove != NOMOVE) {
			for (int i = 0; i < moves->size(); i++) {
				if ((*moves)[i]->move == ttMove) {
					(*moves)[i]->score = hash_move_sort;
				}
				break;
			}
		}

		int move = NOMOVE;
		int legal = 0;
		for (int m = 0; m < moves->size(); m++) {
			ss->pickNextMove(m, moves);

			move = (*moves)[m]->move;
			bool capture = (ss->pos->piece_list[Them][TOSQ(move)] != NO_TYPE) ? true : false;

			reduction = 0;
			int extensions = 0;


			// Step 10. Various extensions.

			if (SPECIAL(move) == CASTLING) { // Castle extensions.
				extensions++;
			}

			if (in_check) { // In check extensions
				extensions++;
			}

			//if (ss->pos->piece_list[ss->pos->side_to_move][FROMSQ(move)] == PAWN && is_passed(FROMSQ(move), ss->pos)) { // Passed pawn extensions
			//	extensions++;
			//}


			if (!ss->pos->make_move((*moves)[m])) {
				continue;
			}
			
			bool gives_check = ss->pos->in_check();			



			// Step 11. Late move pruning (~21 elo). If the move is tactically boring and late, we can probably safely prune it.
			//			We set the lmp flag before incrementing legal, such that if late_move_pruning() returns 0, we are ensured to have searched a move.
			lmp = (extensions == 0) ? (legal > late_move_pruning(depth, improving)) : false;
			
			if (lmp && !in_check && !gives_check && !capture && !is_pv
				&& SPECIAL(move) != PROMOTION && SPECIAL(move) != ENPASSANT && SPECIAL(move) != CASTLING) {
				ss->pos->undo_move();
			
				continue;
			}


			// Step 12. If we are allowed to use futility pruning, and this move is not tactically significant, prune it.
			//			We just need to make sure that at least one legal move has been searched since we'd risk getting false mate scores else.
			if (futility_pruning && !capture && !gives_check && SPECIAL(move) != PROMOTION && SPECIAL(move) != ENPASSANT && legal > 0) {
				ss->pos->undo_move();
				continue;
			}
			
			new_depth = depth + extensions - 1;



			// Step 13. Late move reductions (~33 elo)
			if (!in_check && !gives_check && !capture && SPECIAL(move) != PROMOTION && SPECIAL(move) != ENPASSANT && extensions == 0
				&& legal > lmr_limit && score > -MATE) {
			
				reduction = late_move_reduction(depth, legal, is_pv, improving);
			
				new_depth = std::max(depth - reduction - 1, 0);
			
				reductions++;
			}


			// Increment the legal move counter.
			legal++;

			ss->moves_path[ss->pos->ply] = move;

			re_search:

			// Step 14. Principal Variation Search.
			if (!raised_alpha) {
				score = -alphabeta(ss, new_depth, -beta, -alpha, true, &line);
			}
			else {
				score = -alphabeta(ss, new_depth, -alpha - 1, -alpha, true, &line);

				if (score > alpha) {
					score = -alphabeta(ss, new_depth, -beta, -alpha, true, &line);
				}
			}


			// If we raise alpha on a reduced search, re-search the move at full depth.
			if (reduction > 0 && score > alpha) {
				reduction = 0;
				new_depth = depth - 1;

				re_searches++;

				goto re_search;
			}


			ss->pos->undo_move();


			if (score >= beta) {
				if (legal == 1) {
					ss->info->fhf++;
				}
				ss->info->fh++;

				// If it is not a capture, promotion or en-passant, we'll update our quiet move ordering heuristics.
				if (!capture && SPECIAL(move) != PROMOTION && SPECIAL(move) != ENPASSANT) {
					// Killer move heuristic update
					ss->setKillers(ss->pos->ply, move);

					// Countermove heuristic update
					ss->counterMoves[FROMSQ(ss->moves_path[ss->pos->ply - 1])][TOSQ(ss->moves_path[ss->pos->ply - 1])] = move;

					// History heuristic update
					ss->history[ss->pos->side_to_move][FROMSQ(move)][TOSQ(move)] += depth * depth;
					
					
					// If the history score becomes higher than the killer scores, we should scale down the whole table, since history moves
					//	are usually worse than killer moves.
					if (ss->history[ss->pos->side_to_move][FROMSQ(move)][TOSQ(move)] >= countermove_bonus) {
					
						for (int i = 0; i < 64; i++) {
							for (int j = 0; j < 64; j++) {
								ss->history[ss->pos->side_to_move][i][j] /= 2;
								ss->history[(ss->pos->side_to_move == WHITE) ? BLACK : WHITE][i][j] /= 2;
							}
						}
					}

				}
				

				tt->store_entry(ss->pos, move, score, depth, BETA);

				delete moves;

				return beta;
			}

			if (score > alpha) {
				alpha = score;
				raised_alpha = true;
				best_move = move;

				ChangePV(best_move, pvLine, &line);
			}
		}

		delete moves;

		if (legal <= 0) {
			if (ss->pos->in_check()) {
				return -INF + ss->pos->ply;
			}
			else {
				return 0;
			}
		}

		
		if (raised_alpha) {
			tt->store_entry(ss->pos, best_move, alpha, depth, EXACT);

			assert(best_move == pvLine->pv[0]);
		}
		else{
			tt->store_entry(ss->pos, best_move, alpha, depth, ALPHA);
		}


		return alpha;
	}


	const int delta_piece_value[5] = { 100, 310, 350, 560, 1000 };

	int quiescence(SearchThread_t* ss, int alpha, int beta) {
		assert(beta > alpha);

		ss->info->nodes++;

		if ((ss->info->nodes & 2047) == 0) {
			CheckUp(ss);
		}

		if (ss->info->stopped) {
			return 0;
		}


		if (ss->pos->is_repetition()) {
			return 0;
		}

		if (ss->pos->ply >= MAXDEPTH) {
			return Eval::evaluate(ss->pos);
		}

		// Step 1. Static evaluation and possible cutoff if this beats beta.
		int stand_pat = Eval::evaluate(ss->pos);

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



		// Step 2. Delta pruning (~19 elo). If our position is so bad that not even the best capture possible would be enough to raise alpha, we'll assume that it is an all-node.
		if (stand_pat + std::max(delta_margin, ss->pos->best_capture_possible()) <= alpha && !in_check) {
			return alpha;
		}


		
		MoveList* ml = nullptr;

		if (!in_check) {
			ml = ss->generate_moves(true);
		}
		else {
			ml = ss->generate_moves();
		}

		int legal = 0;
		int move = NOMOVE;
		for (int m = 0; m < ml->size(); m++) {
			ss->pickNextMove(m, ml);
			
			move = (*ml)[m]->move;
			int piece_captured = ss->pos->piece_list[(ss->pos->side_to_move == WHITE) ? BLACK : WHITE][TOSQ(move)];
			int piece_moved = ss->pos->piece_list[ss->pos->side_to_move][FROMSQ(move)];

			// Step 1. Futility pruning (~6 elo). If the value of the piece captured, plus some margin (~200cp) is still not enough to raise alpha, we won't bother searching it.
			// We'll just have to make sure, that there has been tested at least one legal move, so we don't miss a checkmate
			if (SPECIAL(move) != PROMOTION && SPECIAL(move) != ENPASSANT && piece_captured != NO_TYPE &&
				stand_pat + delta_piece_value[piece_captured] + delta_margin < alpha
				&& !ss->pos->is_endgame() && legal > 0 && !in_check) {
				continue;
			}



			if (!ss->pos->make_move((*ml)[m])) {
				continue;
			}
			legal++;


			score = -quiescence(ss, -beta, -alpha);

			ss->pos->undo_move();

			if (score >= beta) {
				if (legal == 1) {
					ss->info->fhf++;
				}
				ss->info->fh++;

				delete ml;

				return beta;
			}

			if (score > alpha) {
				alpha = score;
			}
		}

		delete ml;

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
	return score * (100 / Eval::pawnValMg);
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

void Search::INIT() {

	// Initialize MvvLva.
	int victim_scores[6] = { 100, 200, 300, 400, 500, 600 };

	for (int victim = KING; victim >= PAWN; victim--) {
		for (int attacker = KING; attacker >= PAWN; attacker--) {
			MvvLva[attacker][victim] = victim_scores[victim] + (6 - attacker);
		}
	}

	// Initialize table of late move reductions
	for (int d = 1; d < MAXDEPTH; d++) {

		for (int c = 1; c < MAXPOSITIONMOVES; c++) {
			//Reductions[d][c] = std::round(0.75 + (std::log(2.0 * double(d)) * std::log(2.0 * double(c))) / 2.75);
			Reductions[d][c] = std::round((std::log(2.0 * double(d)) * std::log(2.0 * double(c))) / 2.75);
		}

	}
}
