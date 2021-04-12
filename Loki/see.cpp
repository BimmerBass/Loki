#include "position.h"


// The piece values used in the static exchange evaluation
constexpr int see_pieces[6] = { 100, 320, 350, 560, 1000, 20000 };



/*

Function to determine the best possible capture on the board. For the side in question, this is just the most valuable opponent piece minus the least valuable own piece.

*/

int GameState_t::best_capture_possible() const {

	// Assume opponent has a pawn
	int score = see_pieces[PAWN];

	// See if we can capture a piece with a higher value
	for (int pce = QUEEN; pce > PAWN; pce++) {
		if (pieceBBS[pce][(side_to_move == WHITE) ? BLACK : WHITE] != 0) {
			score = see_pieces[pce];
			break;
		}
	}

	// Promotions
	if ((pieceBBS[PAWN][side_to_move] & BBS::RankMasks8[(side_to_move == WHITE) ? RANK_7 : RANK_2]) != 0) { // If we have a pawn on the rank before promotion
		score += see_pieces[QUEEN] - see_pieces[PAWN]; // We loose a queen and gain a pawn
	}


	return score;
	//int smallest_attacker = KING;
	//int biggest_victim = PAWN;
	//
	//for (int pce = PAWN; pce <= QUEEN; pce++) {
	//
	//	if (pieceBBS[pce][(side_to_move == WHITE) ? BLACK : WHITE] != 0) {
	//		biggest_victim = pce;
	//	}
	//
	//}
	//
	//for (int pce = QUEEN; pce >= PAWN; pce--) {
	//
	//	if (pieceBBS[pce][side_to_move] != 0) {
	//		smallest_attacker = pce;
	//	}
	//
	//}
	//
	//return (see_pieces[biggest_victim] - see_pieces[smallest_attacker]);
}





/*

Returns a bitboard with all attackers to the given square

*/

Bitboard GameState_t::attackers_to(int sq, Bitboard occupied) const {

	Bitboard attackers = 0;
	Bitboard sqBrd = uint64_t(1) << sq;

	// Pawn attackers
	attackers |= ((shift<NORTHWEST>(sqBrd) | shift<NORTHEAST>(sqBrd)) & pieceBBS[PAWN][BLACK]);
	attackers |= ((shift<SOUTHWEST>(sqBrd) | shift<SOUTHEAST>(sqBrd)) & pieceBBS[PAWN][WHITE]);

	// Knights
	attackers |= ((BBS::knight_attacks[sq]) & (pieceBBS[KNIGHT][WHITE] | pieceBBS[KNIGHT][BLACK]));

	// Bishops
	attackers |= ((Magics::attacks_bb<BISHOP>(sq, occupied)) & (pieceBBS[BISHOP][WHITE] | pieceBBS[BISHOP][BLACK]));

	// Rooks
	attackers |= ((Magics::attacks_bb<ROOK>(sq, occupied)) & (pieceBBS[ROOK][WHITE] | pieceBBS[ROOK][BLACK]));

	// Queens
	attackers |= ((Magics::attacks_bb<QUEEN>(sq, occupied)) & (pieceBBS[QUEEN][WHITE] | pieceBBS[QUEEN][BLACK]));

	// Kings
	attackers |= ((BBS::king_attacks[sq]) & (pieceBBS[KING][WHITE] | pieceBBS[KING][BLACK]));

	return attackers;
}




/*

Returns the smallest attacker piece type

*/

template <int pce>
int min_attacker(const Bitboard pieceBBS[6][2], SIDE stm, int to_sq, Bitboard stmAttackers, Bitboard& occupied, Bitboard& attackers) {
	assert(pce >= PAWN && pce <= KING);

	// See if any piece of type pce can capture.
	Bitboard b = stmAttackers & pieceBBS[pce][stm];

	if (!b) { // If not, check if the next piecetype can.
		return min_attacker<pce + 1>(pieceBBS, stm, to_sq, stmAttackers, occupied, attackers);
	}

	// If there are any attackers, remove it from the occupied bitboard since it can't capture twice.
	occupied ^= bitScanForward(b);

	// Now we'll neeed to add the possible X-ray attackers that might be behind the capturing piece.
	SIDE Them = (stm == WHITE) ? BLACK : WHITE;
	if constexpr (pce == PAWN || pce == BISHOP || pce == QUEEN) {
		attackers |= (Magics::attacks_bb<BISHOP>(to_sq, occupied) & ((pieceBBS[BISHOP][stm] | pieceBBS[QUEEN][stm]) |
			(pieceBBS[BISHOP][Them] | pieceBBS[QUEEN][Them])));
	}

	if constexpr (pce == ROOK || pce == QUEEN) {
		attackers |= (Magics::attacks_bb<ROOK>(to_sq, occupied) & ((pieceBBS[ROOK][stm] | pieceBBS[QUEEN][stm]) |
			(pieceBBS[ROOK][Them] | pieceBBS[QUEEN][Them])));
	}

	// Only add the attackers in the occupied bitboard.
	attackers &= occupied;

	return pce;
}

template<>
int min_attacker<KING>(const Bitboard[6][2], SIDE, int, Bitboard, Bitboard&, Bitboard&) {
	return KING; // King is the highest piece-type, so return.
}



/*

Helper function to determine slider attackers to to_sq.

*/
Bitboard GameState_t::attackSlider(Bitboard occupied, int to_sq, SIDE side) const {
	Bitboard b = 0;

	b |= (Magics::attacks_bb<BISHOP>(to_sq, occupied) & (pieceBBS[BISHOP][side] | pieceBBS[QUEEN][side]));
	b |= (Magics::attacks_bb<ROOK>(to_sq, occupied) & (pieceBBS[ROOK][side] | pieceBBS[QUEEN][side]));

	return (b & occupied);
}


/*

Static Exchange Evaluation function. Computes the likely material gain/loss as a result of a capture.

*/


int GameState_t::see(unsigned int move) const {

	int gain[32], d = 0, aPiece = NO_TYPE, sidePick = WHITE, to_sq = TOSQ(move), from_sq = FROMSQ(move);
	Bitboard mayXray = 0, fromSet = 0, occupied = 0, attackers = 0;

	assert(piece_on(from_sq, side_to_move) >= PAWN && piece_on(from_sq, side_to_move) <= KING);
	assert(piece_on(to_sq, (side_to_move == WHITE) ? BLACK : WHITE) >= PAWN && piece_on(to_sq, (side_to_move == WHITE) ? BLACK : WHITE) < KING);

	// If move is a special move
	if (SPECIAL(move) == ENPASSANT) {
		return 100;
	}


	sidePick = (side_to_move == WHITE) ? BLACK : WHITE; // Start with the opponent response.
	aPiece = piece_on(from_sq, side_to_move); // The first attacked piece after the move is the piece moved.
	fromSet = uint64_t(1) << from_sq;
	occupied = all_pieces[WHITE] | all_pieces[BLACK]; // Set occupancy bitboard
	attackers = attackers_to(to_sq, occupied);	// Find all attackers to the destination square.

	gain[d] = see_pieces[piece_on(to_sq, (side_to_move == WHITE) ? BLACK : WHITE)]; // The initial gain is the piece captured.

	mayXray = occupied ^ (pieceBBS[KNIGHT][WHITE] | pieceBBS[KNIGHT][BLACK] | pieceBBS[KING][WHITE] | pieceBBS[KING][BLACK]);

	do {
		d++;
		gain[d] = see_pieces[aPiece] - gain[d - 1];

		// Remove the capturer from the attack set and occupied set.
		attackers ^= fromSet;
		occupied ^= fromSet;

		// If the piece moved can open up for another attacker, we'll have to add this.
		if (fromSet && mayXray) {
			attackers |= attackSlider(occupied, to_sq, WHITE);
			attackers |= attackSlider(occupied, to_sq, BLACK);
		}

		// Now we'll need to find the other side's least valuable attacker to the square.
		for (aPiece = PAWN; aPiece <= KING; aPiece++) {
			fromSet = pieceBBS[aPiece][sidePick] & attackers;

			if (fromSet) {
				fromSet = uint64_t(1) << bitScanForward(fromSet);
				sidePick = (sidePick == WHITE) ? BLACK : WHITE;
				break;
			}
		}

	} while (fromSet);

	while (d--) {
		gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
	}

	return gain[0];
}