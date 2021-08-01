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

	inline Bitboard outer_kingRing(int kingSq) {
		assert(kingSq >= A1 && kingSq <= H8);

		return (BBS::EvalBitMasks::outer_kingring[kingSq]);
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


using namespace PSQT;

/*

Constants --- NOTE: If a constant doesn't end with "_penalty" it is a bonus unless otherwise specified. Also, constants declared as lists are accessed by constant[MG] for
	middlegame and constant[EG] for endgame

*/
constexpr int tempo = 18;
const int max_material[2] = { Eval::queen_value.mg + 2 * Eval::rook_value.mg + 2 * Eval::bishop_value.mg + 2 * Eval::knight_value.mg,
							Eval::queen_value.eg + 2 * Eval::rook_value.eg + 2 * Eval::bishop_value.eg + 2 * Eval::knight_value.eg };


/*

Imbalance constants --- These values are used as suggested by GM Larry Kaufmans paper on material imbalances.

*/
const Score bishop_pair(18, 55);
const Score knight_pawn_penaly(1, 1);
const Score rook_pawn_bonus(3, 1);


/*

Pawn evaluation constants

*/
constexpr int candidate_passer = 0;
//constexpr int connected[2] = { 10, 7 }; // Bonus for being directly defended by another pawn
//extern PSQT::Score connected;

const Score doubled_penalty(5, 22);
const Score doubled_isolated_penalty(16, 15);
const Score isolated_penalty(11, 6);
const Score backwards_penalty(7, 1);

// Not implemented yet.
constexpr int hanging_penalty = 0;


/*

Piece evaluation constants

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


const PSQT::Score queen_on_kingring(3, 19);
const PSQT::Score threatened_queen(52, 70);

/*

King evaluation constants

*/
constexpr int king_open_file_penalty = 100;
constexpr int king_semi_open_file_penalty = 50;
const Score pawnless_flank(248, -78);

#endif