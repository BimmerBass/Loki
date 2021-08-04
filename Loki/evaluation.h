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

	/*struct Evaluation {
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
	bool material_draw(GameState_t* pos);*/
	
	template<EvalType = NORMAL>
	class Evaluate {
	public:
		int score(const GameState_t* pos);

	private:

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
const Score doubled_penalty;
const Score doubled_isolated_penalty;
const Score isolated_penalty;
const Score backwards_penalty;

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
extern const Score king_open_file_penalty;
extern const Score king_semi_open_file_penalty;
extern const Score pawnless_flank;

extern const Score safety_table[100];
extern const Score pawnStorm[64];
extern const Score king_pawn_distance_penalty[8];
extern const Score open_kingfile_penalty[8];
extern const Score semiopen_kingfile_penalty[8];


/*
Other constants
*/
constexpr int tempo = 18;
const int max_material[2] = { queen_value.mg + 2 * rook_value.mg + 2 * bishop_value.mg + 2 * knight_value.mg,
							queen_value.eg + 2 * rook_value.eg + 2 * bishop_value.eg + 2 * knight_value.eg };



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


#endif