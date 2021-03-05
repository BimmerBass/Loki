#ifndef EVALUATION_H
#define EVALUATION_H
#include "movegen.h"
#include "psqt.h"

#include "test_positions.h"

enum GamePhase :int { MG = 0, EG = 1 };

namespace Eval {

	struct Evaluation {
		Evaluation(int m, int e) {
			mg = m; eg = e;
		}

		int interpolate(GameState_t* pos);

		int mg = 0;
		int eg = 0;


		// Indexed by attacks[piecetype][side]
		Bitboard attacks[6][2] = { {0} };

		// Indexed by passed_pawns[side]
		Bitboard passed_pawns[2] = { 0 };
	};

	extern Bitboard king_flanks[8];

	int evaluate(GameState_t* pos);


	int mg_evaluate(GameState_t* pos);
	int eg_evaluate(GameState_t* pos);

	int phase(GameState_t* pos);

	// Returns true if no checkmate can be forced by either side.
	bool material_draw(GameState_t* pos);
	
	// Returns a bitboard with all squares, that the king can move to, together with the king square itself
	inline Bitboard king_ring(int kingSq) {
		assert(kingSq >= A1 && kingSq <= H8);

		return (BBS::king_attacks[kingSq] | (uint64_t(1) << kingSq));
	}

	enum pieceValues : int {
		pawnValMg = 100,
		knightValMg = 320,
		bishopValMg = 350,
		rookValMg = 500,
		queenValMg = 900,
		kingValMg = 20000,

		pawnValEg = 100,
		knightValEg = 320,
		bishopValEg = 350,
		rookValEg = 500,
		queenValEg = 900,
		kingValEg = 20000
	};

	/*
	
	Initialization functions
	
	*/
	void initKingFlanks();
	void INIT();

	namespace Debug {

		void eval_balance();
	}

}


/*

Constants --- NOTE: If a constant doesn't end with "_penalty" it is a bonus unless otherwise specified. Also, constants declared as lists are accessed by constant[MG] for
	middlegame and constant[EG] for endgame

*/
constexpr int tempo = 18;
const int max_material[2] = { Eval::queenValMg + 2 * Eval::rookValMg + 2 * Eval::bishopValMg + 2 * Eval::knightValMg,
							Eval::queenValEg + 2 * Eval::rookValEg + 2 * Eval::bishopValEg + 2 * Eval::knightValEg };



/*

Pawn evaluation constants

*/
constexpr int candidate_passer = 0;
constexpr int connected[2] = { 10, 7 }; // Bonus for being directly defended by another pawn

constexpr int isolated_penalty[2] = { 20 , 35 };
constexpr int doubled_isolated_penalty[2] = { 30,45 };
constexpr int doubled_penalty[2] = { 10, 15 };

// Not implemented yet.
constexpr int backwards_penalty = 0;
constexpr int hanging_penalty = 0;


/*

Piece evaluation constants

*/
constexpr int outpost[2] = { 16, 10 };
constexpr int reachable_outpost[2] = { 7, 5 };

constexpr int knight_on_kingring[2] = { 5, 2 };
constexpr int defended_knight[2] = { 5, 3 };

constexpr int bishop_on_kingring[2] = { 15, 18 };
constexpr int bishop_on_queen[2] = { 12, 18 };
constexpr int blocked_bishop_coefficient_penalty[2] = { 3, 2 };

constexpr int rook_on_kingring[2] = { 10, 20 };
constexpr int rook_on_queen[2] = { 6, 9 };
constexpr int doubled_rooks[2] = { 40, 20 };
constexpr int rook_open_file[2] = { 20, 14 };
constexpr int rook_semi_open_file[2] = { 9, 11 };
constexpr int rook_behind_passer = 90;


constexpr int queen_on_kingring[2] = { 3, 9 };
constexpr int queen_development_penalty = 5; // Only used in the middlegame.


/*

King evaluation constants

*/
constexpr int king_open_file_penalty = 50;
constexpr int king_semi_open_file_penalty = 25;


#endif