#include "position.h"


// The piece values used in the static exchange evaluation
constexpr int see_pieces[6] = { 100, 320, 350, 560, 1000, 20000 };



/*

Function to determine the best possible capture on the board. For the side in question, this is just the most valuable opponent piece minus the least valuable own piece.

*/

int GameState_t::best_capture_possible() const {

	int smallest_attacker = KING;
	int biggest_victim = PAWN;

	for (int pce = PAWN; pce <= QUEEN; pce++) {

		if (pieceBBS[pce][(side_to_move == WHITE) ? BLACK : WHITE] != 0) {
			biggest_victim = pce;
		}

	}

	for (int pce = QUEEN; pce >= PAWN; pce--) {

		if (pieceBBS[pce][side_to_move] != 0) {
			smallest_attacker = pce;
		}

	}

	return (see_pieces[biggest_victim] - see_pieces[smallest_attacker]);
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