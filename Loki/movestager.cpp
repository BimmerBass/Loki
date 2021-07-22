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


/// <summary>
/// Determine whether a move is pseudo-legal or not. This method is used to check the TT move in MoveStager since some illegal moves may cause a crash.
/// It can also be used for killers at a later point.
/// </summary>
/// <param name="pos">: The position object.</param>
/// <param name="move">: The encoded move.</param>
/// <param name="in_check">: A flag for signalling whether we're in check or not.</param>
/// <returns>True if the move is pseudo-legal and false if not.</returns>
bool is_pseudo_legal(GameState_t* pos, unsigned int move, bool in_check) {

	// Step 1. Initialize some variables and extract move info.
	SIDE Them = (pos->side_to_move == WHITE) ? BLACK : WHITE;
	SIDE Us = pos->side_to_move;
	int from_sq = FROMSQ(move);
	int to_sq = TOSQ(move);
	int special = SPECIAL(move);
	int promotion_piece = PROMTO(move);

	// Step 2. If we're moving an empty square or an enemy piece, the move is illegal.
	if (pos->piece_list[Us][from_sq] == NO_TYPE || pos->piece_list[Them][from_sq] != NO_TYPE) {
		return false;
	}

	// Step 3. We can't capture our own pieces.
	if (pos->piece_list[Us][to_sq] != NO_TYPE) {
		return false;
	}

	// Step 4. For special moves (promotion, en-passant or castling), we will just generate all moves and see if the list contains the move we're checking.
	// Note: This is slow, but since special moves are so rare, it won't be too much of a problem.
	if (special != NOT_SPECIAL) {
		MoveList moves;
		moveGen::generate<ALL>(pos, &moves);

		return moves.contains(move);
	}

	// Step 5. Since we have checked that we're moving and capturing the correct piece types, we can extract this info.
	int piece_moved = pos->piece_list[Us][from_sq];
	int piece_captured = pos->piece_list[Them][to_sq];
	Bitboard attackBB = 0;
	Bitboard occupied = (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]);

	bool is_capture, single_push, double_push;

	// Step 6. Handle the different pieces.
	switch (piece_moved) {
	case PAWN:
		// Step 6A. Since promotions are handled in step 4, make sure the destination square on the 8'th or 1'st rank.
		if (((BBS::RankMasks8[RANK_8] | BBS::RankMasks8[RANK_1]) & (uint64_t(1) << to_sq)) != 0) {
			return false;
		}

		// Step 6B. Since the move isn't a promotion, it needs to be either a capture, single push or double push.
		is_capture = ((Us == WHITE) ? (to_sq == from_sq + 9 || to_sq == from_sq + 7) : (to_sq == from_sq - 9 || to_sq == from_sq - 7))
			&& (pos->all_pieces[Them] & (uint64_t(1) << to_sq)) != 0;

		single_push = (Us == WHITE) ? to_sq == from_sq + 8 : to_sq == from_sq - 8;

		double_push = ((Us == WHITE) ? (to_sq == from_sq + 16 && (BBS::RankMasks8[RANK_2] & (uint64_t(1) << from_sq)) != 0)
			: (to_sq == from_sq - 16 && (BBS::RankMasks8[RANK_7] & (uint64_t(1) << from_sq)) != 0))
			&& ((pos->all_pieces[WHITE] | pos->all_pieces[BLACK]) & (uint64_t(1) << (Us == WHITE ? from_sq + 8 : from_sq - 8))) == 0;

		if (!(is_capture || single_push || double_push)) {
			return false;
		}

		break;
	
	/*
	For the sliding pieces, we need to make sure their paths aren't blocked.
	*/
	case BISHOP:
		attackBB = Magics::attacks_bb<BISHOP>(from_sq, occupied);

		if ((attackBB & (uint64_t(1) << to_sq)) == 0) {
			return false;
		}
		break;

	case ROOK:
		attackBB = Magics::attacks_bb<ROOK>(from_sq, occupied);

		if ((attackBB & (uint64_t(1) << to_sq)) == 0) {
			return false;
		}
		break;

	case QUEEN:
		attackBB = Magics::attacks_bb<QUEEN>(from_sq, occupied);
		
		if ((attackBB & (uint64_t(1) << to_sq)) == 0) {
			return false;
		}
		break;

	default:	/* There are no special considerations for kings or knights. */
		break;
	}

	return true;
}