#include "movestager.h"



/// <summary>
/// Alpha/Beta constructor for the MoveStager class.
/// </summary>
/// <param name="_pos">A position object that we'll store in order to generate the moves.</param>
/// <param name="stats">The previously generated stats on different kinds of (mostly quiet) moves</param>
/// <param name="ttMove">A move from the transposition table.</param>
MoveStager::MoveStager(GameState_t* _pos, MoveStats_t* _stats, unsigned int ttMove) {
	pos = _pos;
	stats = _stats;

	// If there is a move from the transposition table, set stage to tt stage, otherwise to capture stage.
	tt_move = ttMove;
	if (ttMove != NOMOVE) {
		stage = TT_STAGE;

		// Since we're using quite small hash keys, we need to check that we're not capturing friendly pieces or moving opponent ones with the tt move.
		if (pos->piece_list[pos->side_to_move][FROMSQ(tt_move)] == NO_TYPE || pos->piece_list[pos->side_to_move][TOSQ(tt_move)] != NO_TYPE
			|| pos->piece_list[(pos->side_to_move == WHITE) ? BLACK : WHITE][FROMSQ(tt_move)] != NO_TYPE) {
			stage = CAPTURE_SCORE_STAGE;
		}
	}
	else {
		stage = CAPTURE_SCORE_STAGE;
	}
}


/// <summary>
/// A constructor for use in quiescence search. The movestats are excluded since we wont be using them in quiescence where only captures are searched.
/// </summary>
/// <param name="_pos">A position object to use for generating moves.</param>
MoveStager::MoveStager(GameState_t* _pos) {
	pos = _pos;

	stage = CAPTURE_SCORE_STAGE;
}




/// <summary>
/// Responsible for determining the next move to search or, if out of moves, generate more.
/// </summary>
/// <param name="move">A reference to a move that the search function will use. This will be filled out in the method.</param>
/// <param name="skip_quiets">A flag to not return quiet moves.</param>
/// <returns>A boolean flag signalling if a move was found.</returns>
bool MoveStager::next_move(Move_t& move, bool skip_quiets) {

	// Add a goto label here since we will perhaps need to re-enter the switch statement with a new stage.
top:

	switch (stage) {

	case TT_STAGE:
		move.move = tt_move;
		move.score = hash_move_sort;

		stage++;
		return true;

	case CAPTURE_SCORE_STAGE:
		// Score and generate the captures.
		score<CAPTURES>();
		current_move = 0;

		// If there are no captures, go to the quiet score stage.
		if (ml.size() == 0) {
			stage = QUIET_SCORE_STAGE;
			goto top;
		}

		// Set the new flag and fallthrough.
		stage++;
		[[fallthrough]];

	case CAPTURE_STAGE:
		// Step 1. Find the best move and insert it.
		pi_sort();
		move.move = ml[current_move]->move;
		move.score = ml[current_move]->score;

		// Step 2. Now increment the current move and if it turns out we don't have any more moves, increment the stage too.
		current_move++;

		if (current_move >= ml.size()) {
			stage++;
		}

		// Step 3. If the move we found was the TT move, we don't want to search it. Find another one.
		if (move.move == tt_move) {
			goto top;
		}

		return true;

	case QUIET_SCORE_STAGE:
		// If we should skip the quiets, return false since there are no moves after these.
		if (skip_quiets) {
			return false;
		}
		
		// Score and generate the quiet moves.
		score<QUIET>();

		// If there are no quiets, return false.
		if (current_move >= ml.size()) { return false; }

		// Increment stage and fallthrough
		stage++;
		[[fallthrough]];

	case QUIET_STAGE:
		// If we should skip the quiets, return false since there are no moves after these.
		if (skip_quiets) {
			return false;
		}

		// Step 1. Find the best move and insert it.
		pi_sort();
		move.move = ml[current_move]->move;
		move.score = ml[current_move]->score;

		// Step 2. Increment the current move and if we don't have any more moves, increment the stage flag too.
		current_move++;

		if (current_move >= ml.size()) {
			stage++;
		}

		// Step 3. If the move we found was the TT move, we don't want to search it. Find another one.
		if (move.move == tt_move) {
			goto top;
		}

		return true;

	case NO_STAGE:
		return false;

	// If the stage isn't one of the above, an error has occured.
	default:
		assert(false);
	}

	return false;
}


/// <summary>
/// Finds the best move in front of the current index in the movelist array and inserts it there.
/// </summary>
void MoveStager::pi_sort() {
	int best_index = current_move;
	int best_score = -hash_move_sort;

	for (int m = current_move; m < ml.size(); m++) {

		// If the score for (*ml)[m] is higher than the maximum score we've found, this is the new best move.
		if (ml[m]->score > best_score) {
			best_index = m;

			best_score = ml[m]->score;
		}
	}
	// Copy the move at index and at best_index
	Move_t temp_move = ml.at(current_move);
	Move_t best_move = ml.at(best_index);

	// Insert best_move at index and place temp_move (the previous move at index) at best_index.
	ml.replace(current_move, best_move);
	ml.replace(best_index, temp_move);
}



/// <summary>
/// Get the moves that has been played.
/// </summary>
/// <returns>A pointer to the movelist</returns>
MoveList* MoveStager::get_moves() {
	return &ml;
}