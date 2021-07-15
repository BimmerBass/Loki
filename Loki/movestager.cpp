#include "movestager.h"

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

		stage++;
		return true;

	case CAPTURE_SCORE_STAGE:
		// Score and generate the captures.
		score<CAPTURES>();
		current_move = 0;

		// Set the new flag and fallthrough
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
		// Score and generate the quiet moves.
		score<QUIET>();
		current_move = 0;

		// Increment stage and fallthrough
		stage++;
		[[fallthrough]];

	case QUIET_STAGE:
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