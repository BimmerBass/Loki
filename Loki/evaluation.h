#ifndef EVALUATION_H
#define EVALUATION_H

#include "movegen.h"
#include "psqt.h"

#include "test_positions.h"

enum GamePhase :int { MG = 0, EG = 1 };


inline int frontmost_sq(SIDE s, Bitboard b) {
	if (b == 0) {
		return NO_SQ;
	}

	return (s == WHITE) ? bitScanReverse(b) : bitScanForward(b);
}

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

		int king_zone_attackers[2] = { 0 };
		int king_zone_attack_units[2] = { 0 };
	};

	extern Bitboard king_flanks[8];

	int evaluate(GameState_t* pos);

	int phase(GameState_t* pos);

	// Returns true if no checkmate can be forced by either side.
	bool material_draw(GameState_t* pos);
	
	// Returns a bitboard with all squares, that the king can move to, together with the king square itself
	inline Bitboard king_ring(int kingSq) {
		assert(kingSq >= A1 && kingSq <= H8);

		return (BBS::king_attacks[kingSq] | (uint64_t(1) << kingSq));
	}


	/*
	
	These are the piece values Loki uses.
	
	*/
	const PSQT::Score pawn_value(98, 108);
	const PSQT::Score knight_value(405, 393);
	const PSQT::Score bishop_value(415, 381);
	const PSQT::Score rook_value(526, 625);
	const PSQT::Score queen_value(1120, 1306);

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
const int max_material[2] = { Eval::queen_value.mg + 2 * Eval::rook_value.mg + 2 * Eval::bishop_value.mg + 2 * Eval::knight_value.mg,
							Eval::queen_value.eg + 2 * Eval::rook_value.eg + 2 * Eval::bishop_value.eg + 2 * Eval::knight_value.eg };


/*

Imbalance constants --- These values are taken directly from Larry Kaufman's article: "Evaluation of material imbalances"

*/
const int bishop_pair[2] = { Eval::pawn_value.mg / 2, Eval::pawn_value.eg / 2 };
const int knight_pawn_penalty[2] = { Eval::pawn_value.mg / 16, Eval::pawn_value.eg / 16 };
const int rook_pawn_bonus[2] = { Eval::pawn_value.mg / 8, Eval::pawn_value.eg / 8 };

/*

Pawn evaluation constants

*/
constexpr int candidate_passer = 0;
constexpr int connected[2] = { 10, 7 }; // Bonus for being directly defended by another pawn

//constexpr int isolated_penalty[2] = { 10 , 17 };
//constexpr int doubled_isolated_penalty[2] = { 15 , 23 };
//constexpr int doubled_penalty[2] = { 5, 10 };

const PSQT::Score doubled_penalty(5, 22);
const PSQT::Score doubled_isolated_penalty(16, 15);
const PSQT::Score isolated_penalty(11, 6);


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
constexpr int threatened_queen[2] = { 90, 90 };


/*

King evaluation constants

*/
constexpr int king_open_file_penalty = 100;
constexpr int king_semi_open_file_penalty = 50;
constexpr int pawnless_flank[2] = { 150, 250 };

#endif