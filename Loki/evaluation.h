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

#include "movegen.h"

#include "test_positions.h"


/*
Type definitions
*/
enum GamePhase :int { MG = 0, EG = 1 };
enum EvalType :int { NORMAL = 0, TRACE = 1 };

class Score {
public:
	Score(int m, int e) {
		mg = m; eg = e;

	}
	Score(const Score& s) {
		mg = s.mg;
		eg = s.eg;
	}
	Score() { mg = 0; eg = 0; };

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
const Score pawn_value(98, 108);
const Score knight_value(405, 393);
const Score bishop_value(415, 381);
const Score rook_value(526, 625);
const Score queen_value(1120, 1306);


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
}



/*
Imbalance
*/
const Score bishop_pair(18, 55);
const Score knight_pawn_penaly(1, 1);
const Score rook_pawn_bonus(3, 1);


/*
Pawn evaluation
*/
constexpr int candidate_passer = 0;
const Score doubled_penalty(5, 22);
const Score doubled_isolated_penalty(16, 15);
const Score isolated_penalty(11, 6);
const Score backwards_penalty(7, 1);


/*
Piece evaluation
*/
const Score outpost(31, 13);
const Score reachable_outpost(18, -2);

const Score knight_on_kingring(8, -13);
const Score defended_knight(0, 10);

const Score bishop_on_kingring(11, 4);
const Score bishop_on_queen(32, 24);
const Score bad_bishop_coeff(0, 5);

const Score doubled_rooks(31, 9);
const Score rook_on_queen(6, 49);
const Score rook_on_kingring(34, -20);
const Score rook_open_file(43, -11);
const Score rook_semi_open_file(11, 19);
const Score rook_behind_passer(0, 10);


const Score queen_on_kingring(3, 19);
const Score threatened_queen(52, 70);


/*
King evaluation
*/
constexpr int king_open_file_penalty = 100;
constexpr int king_semi_open_file_penalty = 50;
const Score pawnless_flank(248, -78);





	
	
/*
Pawn-specific tables
*/
extern const Score passedPawnTable[64];

/*
Piece-specific tables

*/
extern const std::vector<std::vector<Score>> mobilityBonus;

extern const Score queen_development_penalty[5];

/*
Other square tables
*/

extern const int Mirror64[64];

extern int ManhattanDistance[64][64];


/*
Space term
*/

extern const Score space_bonus[32];

/*
King-safety specific tables
*/
extern const Score safety_table[100];

extern const int castledPawnAdvancementMg[64];
extern const Score pawnStorm[64];

extern const Score king_pawn_distance_penalty[8];


extern const Score open_kingfile_penalty[8];

extern const Score semiopen_kingfile_penalty[8];

void initManhattanDistance();

extern void INIT();




constexpr int tempo = 18;
const int max_material[2] = { queen_value.mg + 2 * rook_value.mg + 2 * bishop_value.mg + 2 * knight_value.mg,
							queen_value.eg + 2 * rook_value.eg + 2 * bishop_value.eg + 2 * knight_value.eg };






// Returns a bitboard with all squares, that the king can move to, together with the king square itself
inline Bitboard king_ring(int kingSq) {
	assert(kingSq >= A1 && kingSq <= H8);

	return (BBS::king_attacks[kingSq] | (uint64_t(1) << kingSq));
}

inline Bitboard outer_kingRing(int kingSq) {
	assert(kingSq >= A1 && kingSq <= H8);

	return (BBS::EvalBitMasks::outer_kingring[kingSq]);
}


inline int frontmost_sq(SIDE s, Bitboard b) {
	if (b == 0) {
		return NO_SQ;
	}

	return (s == WHITE) ? bitScanReverse(b) : bitScanForward(b);
}


#endif