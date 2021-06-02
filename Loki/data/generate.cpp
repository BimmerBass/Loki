#include "generate.h"


namespace DataGeneration {
	
	// Setup of a saved position
	void SavedBoard::setup(const GameState_t* pos, int evaluation) {
		// Step 1. Copy the gamestate.
		pieces[PAWN][WHITE] = pos->pieceBBS[PAWN][WHITE];
		pieces[KNIGHT][WHITE] = pos->pieceBBS[KNIGHT][WHITE];
		pieces[BISHOP][WHITE] = pos->pieceBBS[BISHOP][WHITE];
		pieces[ROOK][WHITE] = pos->pieceBBS[ROOK][WHITE];
		pieces[QUEEN][WHITE] = pos->pieceBBS[QUEEN][WHITE];
		pieces[KING][WHITE] = pos->pieceBBS[KING][WHITE];

		pieces[PAWN][BLACK] = pos->pieceBBS[PAWN][BLACK];
		pieces[KNIGHT][BLACK] = pos->pieceBBS[KNIGHT][BLACK];
		pieces[BISHOP][BLACK] = pos->pieceBBS[BISHOP][BLACK];
		pieces[ROOK][BLACK] = pos->pieceBBS[ROOK][BLACK];
		pieces[QUEEN][BLACK] = pos->pieceBBS[QUEEN][BLACK];
		pieces[KING][BLACK] = pos->pieceBBS[KING][BLACK];

		stm = pos->side_to_move;
		en_passant = pos->enPasSq;
		castling_rights = pos->castleRights;
		move50 = pos->fiftyMove;

		// Step 2. Save the score
		score = evaluation;
	}
}