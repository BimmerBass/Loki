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
#ifndef EVALUATION_H
#define EVALUATION_H

#include <array>
#include "movegen.h"
#include "test_positions.h"
#include "evaltable.h"


/*
Type definitions
*/
enum GamePhase :int { MG = 0, EG = 1 };
enum EvalType :int { NORMAL = 0, TRACE = 1 };

class Score {
public:
	Score(int m, int e) { mg = m; eg = e; }
	Score(const Score& s) { mg = s.mg; eg = s.eg; }
	Score() { mg = 0; eg = 0; }

	int mg = 0;
	int eg = 0;
};



namespace Eval {

	/* Returns true if no checkmate can be forced by either side.
	bool material_draw(GameState_t* pos);
	*/
	
	template<EvalType T = NORMAL>
	class Evaluate {
	public:
		int score(const GameState_t* _pos, bool use_table = true);

	private:
		// The position object that we get when score is called. This is just stored such that all member methods can access it without it being passed as a parameter.
		const GameState_t* pos = nullptr;

		// We keep a struct that holds information gathered earlier in the search
		// This helps to evaluate king safety etc..
		struct EvalData {
			// Piece attacks. Indexed by [side][piecetype]
			Bitboard attacks[2][6] = { {0} };
			Bitboard attacked_by_two[2] = { 0 };

			// Passed pawns
			Bitboard passed_pawns[2] = { 0 };

			int king_zone_attacks[2] = { 0 };	/* The amount of pieces attacking the king */
			int king_safety_units[2] = { 0 };	/* A kind of "weighted" amount of pieces attacking the king. Used to index the safety table */
		};
		// The constant ZeroData is used to quickly clear our evaluation data.
		const EvalData ZeroData;
		EvalData Data;

		// Scores for middlegame and endgame respectively
		int mg_score = 0;
		int eg_score = 0;

		// Clear all data from the previous evaluation.
		void clear();

		/*
		Evaluation sub-methods.
		*/
		int game_phase();

		template<SIDE S> void material();
		template<SIDE S> void psqt();
		template<SIDE S> void imbalance();
		template<SIDE S> void pawns();
		template<SIDE S> void space();
		template<SIDE S, piece pce> void mobility();
		template<SIDE S> void king_safety();

		// An evaluation hash table to re-use recently calculated evaluations.
		EvaluationTable eval_table;

		template<SIDE S> Bitboard weak_squares();
		template<SIDE S> Bitboard attacked_by_all();
	};
	
	
	// The king flanks array is used to determine which pawns to analyze depending on the king's file.
	extern Bitboard king_flanks[8];
	

	/*
	Initialization functions
	*/
	void initKingFlanks();

	void INIT();

	/// <summary>
	/// Used to check that the eval works and doesn't give different values for black and white.
	/// </summary>
	namespace Debug {
		void eval_balance();
	}
}


/*
Material values.
*/
extern const Score pawn_value;
extern const Score knight_value;
extern const Score bishop_value;
extern const Score rook_value;
extern const Score queen_value;


/*
Piece-square tables (defined in psqt.cpp)
*/
namespace PSQT {

	extern const Score PawnTable[64];

	extern const Score KnightTable[64];

	extern const Score BishopTable[64];

	extern const Score RookTable[64];

	extern const Score QueenTable[64];

	extern const Score KingTable[64];

	// Other psqt's
	extern const int Mirror64[64];
	extern int ManhattanDistance[64][64];

	// Initializer methods.
	void initManhattanDistance();
	extern void INIT();
}


/*
Imbalance
*/
extern const Score bishop_pair;
extern const Score knight_pawn_penaly;
extern const Score rook_pawn_bonus;


/*
Pawn evaluation
*/
extern const Score doubled_penalty;
extern const Score doubled_isolated_penalty;
extern const Score isolated_penalty;
extern const Score backwards_penalty;

extern const Score passedPawnTable[64];


/*
Space evaluation
*/
extern const Score space_bonus[32];


/*
Mobility evaluation
*/
extern const std::array<const Score*, 4> mobility_bonus;


/*
Piece evaluation
*/
extern const Score outpost;
extern const Score reachable_outpost;
extern const Score knight_on_kingring;
extern const Score defended_knight;
extern const Score bishop_on_kingring;
extern const Score bishop_on_queen;
extern const Score bad_bishop_coeff;
extern const Score doubled_rooks;
extern const Score rook_on_queen;
extern const Score rook_on_kingring;
extern const Score rook_open_file;
extern const Score rook_semi_open_file;
extern const Score rook_behind_passer;
extern const Score queen_on_kingring;
extern const Score threatened_queen;
extern const Score queen_development_penalty[5];


/*
King evaluation
*/
extern const Score missing_king_pawn;
extern const Score no_enemy_queen;
extern const Score weak_king_square;
extern const Score king_pawn_shelter[8][7];
extern const Score king_pawn_storm[8][7];
extern const Score defending_minors[4][3][3];
extern const Score safety_table[100];


/*
Other constants
*/
constexpr int tempo = 18;
extern const int max_material[2];



/* ---------------------------------- HELPER FUNCTIONS ---------------------------------- */

/// <summary>
/// Get all squares sorrounding the king, including the king's square.
/// </summary>
/// <param name="kingSq">The square, that the king is on.</param>
/// <returns>A bitboard with the king-square and all sorrounding ones.</returns>
inline Bitboard king_ring(int kingSq) {
	assert(kingSq >= A1 && kingSq <= H8);

	return (BBS::king_attacks[kingSq] | (uint64_t(1) << kingSq));
}


/// <summary>
/// Get the (max) 16 squares sorrounding the king-ring.
/// </summary>
/// <param name="kingSq">The square, that the king is on.</param>
/// <returns>A bitboard with the squares sorrounding the king-ring.</returns>
inline Bitboard outer_kingRing(int kingSq) {
	assert(kingSq >= A1 && kingSq <= H8);

	return (BBS::EvalBitMasks::outer_kingring[kingSq]);
}


/// <summary>
/// Compute the forward-most square on a bitboard, depending on which side to move.
/// </summary>
/// <param name="s">The side to move.</param>
/// <param name="b">The bitboard with squares.</param>
/// <returns>The index of the forward-most square relative to the side to move.</returns>
inline int frontmost_sq(SIDE s, Bitboard b) {
	if (b == 0) {
		return NO_SQ;
	}

	return (s == WHITE) ? bitScanReverse(b) : bitScanForward(b);
}

template<SIDE S>
inline int backmost_sq(const Bitboard b) {
	if (b == 0) {
		return NO_SQ;
	}

	return (S == WHITE) ? bitScanForward(b) : bitScanReverse(b);
}


// Calculate the amount of non-pawn material for one side.
template<GamePhase P, SIDE S> inline int non_pawn_material(const GameState_t* pos) {
	// Step 1. Get the amount of pieces.
	int knightCnt = countBits(pos->pieceBBS[KNIGHT][S]);
	int bishopCnt = countBits(pos->pieceBBS[BISHOP][S]);
	int rookCnt = countBits(pos->pieceBBS[ROOK][S]);
	int queenCnt = countBits(pos->pieceBBS[QUEEN][S]);

	// Step 2. Return the non-pawn material.
	return (P == MG) ? (knightCnt * knight_value.mg + bishopCnt * bishop_value.mg + rookCnt * rook_value.mg + queenCnt * queen_value.mg)
		: (knightCnt * knight_value.eg + bishopCnt * bishop_value.eg + rookCnt * rook_value.eg + queenCnt * queen_value.eg);
}

#endif