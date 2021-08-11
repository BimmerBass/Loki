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
#include "evaluation.h"


/*
Initialization of constants. (PSQT are handled in psqt.cpp)
*/
#define S(m, e) Score(m, e) // Inspired by Stockfish/Ethereal. Just a nice and clean way of initializing the constants.

// Material values.
const Score pawn_value(98, 108);
const Score knight_value(405, 393);
const Score bishop_value(415, 381);
const Score rook_value(526, 625);
const Score queen_value(1120, 1306);


// Material imbalances.
const Score bishop_pair(18, 55);
const Score knight_pawn_penaly(1, 1);
const Score rook_pawn_bonus(3, 1);


// Pawn evaluation values.
const Score doubled_penalty(5, 22);
const Score doubled_isolated_penalty(16, 15);
const Score isolated_penalty(11, 6);
const Score backwards_penalty(7, 1);

const Score passedPawnTable[64] = {
		S(34, 26)	, S(52, 11)	,	S(20, 5)	,	S(13, 50)	,	S(3, 27)	,	S(0, 66)	,	S(4, 66)	,	S(16, 4)	,
		S(0, 0)		, S(2, 0)	,	S(4, 1)		,	S(12, 19)	,	S(0, 23)	,	S(17, 1)	,	S(1, 1)		,	S(1, 2)		,
		S(0, 1)		, S(0, 6)	,	S(1, 0)		,	S(2, 3)		,	S(1, 3)		,	S(9, 0)		,	S(16, 2)	,	S(9, 0)		,
		S(3, 27)	, S(2, 19)	,	S(1, 21)	,	S(0, 18)	,	S(0, 7)		,	S(14, 13)	,	S(18, 15)	,	S(5, 23)	,
		S(6, 53)	, S(36, 35)	,	S(19, 35)	,	S(14, 18)	,	S(28, 35)	,	S(18, 31)	,	S(1, 49)	,	S(1, 51)	,
		S(40, 107)	, S(62, 73)	,	S(20, 59)	,	S(1, 51)	,	S(40, 45)	,	S(42, 49)	,	S(12, 71)	,	S(5, 61)	,
		S(84, 137)	, S(48, 163),	S(26, 145)	,	S(66, 91)	,	S(116, 107)	,	S(54, 133)	,	S(16, 129)	,	S(38, 153)	,
		S(63, 74)	, S(16, 13)	,	S(47, 13)	,	S(4, 19)	,	S(11, 11)	,	S(44, 32)	,	S(26, 30)	,	S(11, 17)
};


// Space evaluation values
const Score space_bonus[32] = {
		S(-40, 0)	,	S(-21, 0)	,	S(66, 0)	,	S(41, 0)	,	S(36, 0)	,	S(35, 0)	,	S(23, 0)	, S(18, 0),
		S(5, 0)		,	S(2, 0)		,	S(-1, 0)	,	S(0, 0)		,	S(1, 0)		,	S(12, 0)	,	S(13, 0)	, S(20, 0),
		S(31, 0)	,	S(40, 0)	,	S(39, 0)	,	S(74, 0)	,	S(69, 0)	,	S(68, 0)	,	S(99, 0)	, S(144, 0),
		S(90, 0)	,	S(201, 0)	,	S(90, 0)	,	S(101, 0)	,	S(148, 0)	,	S(219, 0)	,	S(50, 0)	, S(187, 0)
};


// Mobility evaluation values
const Score knightMobility[9] = { S(-69, -76), S(-36, -68), S(-17, -30), S(-2, -14), S(4, -6), S(11, 8), // Knight
		S(20, 9), S(33, 12), S(42, 5)
};
const Score bishopMobility[14] = { S(-46, -54), S(-21, -30), S(11, -13), S(28, 13), S(36, 28), S(50, 37),
	S(54, 51), S(59, 51), S(59, 62), S(66, 59), S(83, 57), S(102, 56),
	S(82, 69), S(91, 73)
};
const Score rookMobility[15] = { S(-60,-82), S(-24,-15), S(0, 17) ,S(3, 43), S(4, 72), S(14,100), // Rook. TODO: Re-tune this
  S(20,102), S(30,122), S(41,133), S(41 ,139), S(41,153), S(45,160),
  S(57,165), S(58,170), S(67,175)
};
const Score queenMobility[28] = {
	S(-38, -66), S(-25, -14), S(-1, -21), S(21, 6), S(31, 34), S(30, 39), S(36, 46), S(40, 46), S(46, 61), // Queen
	S(45, 70), S(52, 80), S(53, 92), S(54, 107), S(61, 107), S(69, 109),
	S(71, 118), S(66, 131), S(69, 137), S(67, 144), S(69, 156), S(97, 138),
	S(105, 144), S(96, 156), S(85, 172), S(111, 191),
	S(129, 170), S(115, 170), S(96, 202)
};
const std::array<const Score*, 4> mobility_bonus = {
	knightMobility,
	bishopMobility,
	rookMobility,
	queenMobility
};


// Piece evaluation values.
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

// Penalties for early queen development in the middlegame.
const Score queen_development_penalty[5] = { S(0, 0), S(0, 0), S(0, 0), S(3, 0), S(12, 0) };


// King pawn shield evaluation
const Score minimum_kp_distance[15] = { S(15, 3), S(3, 7), S(-7, 3), S(-15, -3), S(-19, -11), S(-7, -19), S(-3, -17),
								S(-3, -3), S(5, 3), S(-7, 3), S(-1, -3), S(-1, -5), S(23, 1), S(-1, 7), S(15, 13) }; // Indexed by manhattan distance - 1

// Indexed such that if the king is on the king-side it is [0][sq] and if it's on the queen-side it's [1][sq]
const Score king_shelter[2][64] = {
	{
		S(10, 4)	,	S(16, -14)	,	S(-2, 6)	,	S(22, 22)	,	S(-4, -6)	,	S(-10, 6)	,	S(2, 8)		,	S(-14, -2)	,
		S(-8, 10)	,	S(4, 0)		,	S(8, -8)	,	S(16, -2)	,	S(8, 0)		,	S(12, -6)	,	S(14, -16)	,	S(0, -10)	,
		S(0, -4)	,	S(-22, 8)	,	S(-6, -2)	,	S(14, 10)	,	S(16, -6)	,	S(12, -4)	,	S(12, -14)	,	S(12, -6)	,
		S(0, 2)		,	S(6, 14)	,	S(12, -6)	,	S(4, 12)	,	S(22, -4)	,	S(0, 6)		,	S(16, -12)	,	S(26, -12)	,
		S(-6, -20)	,	S(0, 10)	,	S(-16, -8)	,	S(2, 2)		,	S(-10, 16)	,	S(2, 2)		,	S(0, -8)	,	S(-10, 4)	,
		S(-6, -10)	,	S(4, 8)		,	S(4, -10)	,	S(-10, 12)	,	S(-4, 0)	,	S(-2, 2)	,	S(-12, 12)	,	S(-18, -2)	,
		S(6, 0)		,	S(16, 4)	,	S(0, 2)		,	S(-20, 22)	,	S(-6, -16)	,	S(-6, -2)	,	S(0, 8)		,	S(-32, -8)	,
		S(4, -10)	,	S(16, -16)	,	S(-4, -26)	,	S(-20, 0)	,	S(2, 6)		,	S(-14, -26)	,	S(-12, -8)	,	S(-4, -2)
	},
	{
		S(-1, -1)	,	S(-5, 1)	,	S(-5, 11)	,	S(-1, -5)	,	S(11, -5)	,	S(5, -5)	,	S(-1, 1)	, S(-1, -7)	,
		S(1, -1)	,	S(23, -17)	,	S(25, -7)	,	S(-11, -9)	,	S(-5, -7)	,	S(5, 7)		,	S(7, 1)		, S(1, -9)	,
		S(5, 1)		,	S(3, -3)	,	S(7, 3)		,	S(-9, -1)	,	S(-5, -5)	,	S(-5, -19)	,	S(1, -5)	, S(1, 11)	,
		S(7, 3)		,	S(1, 3)		,	S(11, 7)	,	S(1, 15)	,	S(5, 5)		,	S(9, -1)	,	S(11, 3)	, S(9, -1)	,
		S(3, 9)		,	S(5, 9)		,	S(3, 9)		,	S(-1, 3)	,	S(-5, 11)	,	S(-5, 1)	,	S(-3, 9)	, S(-1, -3)	,
		S(-9, 7)	,	S(-3, 11)	,	S(5, 19)	,	S(3, 5)		,	S(1, 9)		,	S(-1, -9)	,	S(3, 7)		, S(-5, -7)	,
		S(-7, 7)	,	S(-1, 5)	,	S(-7, 5)	,	S(-3, 1)	,	S(-3, -7)	,	S(-1, -1)	,	S(1, 1)		, S(-1, 7)	,
		S(1, 7)		,	S(9, -5)	,	S(3, -5)	,	S(3, -1)	,	S(-3, 1)	,	S(-7, 7)	,	S(-1, -1)	, S(-3, 9)
	}
};


// King safety evaluation

#undef S

/*
Other
*/
const int max_material[2] = { queen_value.mg + 2 * rook_value.mg + 2 * bishop_value.mg + 2 * knight_value.mg,
							queen_value.eg + 2 * rook_value.eg + 2 * bishop_value.eg + 2 * knight_value.eg };



namespace Eval {

	/// <summary>
	/// Evaluate a position with a side-relative score.
	/// </summary>
	/// <param name="_pos">The position object that is to be evaluated.</param>
	/// <returns>A numerical score for the position, relative to the side to move.</returns>
	template<EvalType T>
	int Evaluate<T>::score(const GameState_t* _pos, bool use_table) {
		// Step 1. Clear the object and store the position object.
		clear();
		pos = _pos;
		int v = 0;

		// Step 2. Probe the evaluation hash table for an entry.
		bool hit = false;
		const EvalEntry_t* entry = eval_table.probe(pos->posKey, hit);

		if (use_table && hit) {
			v = entry->get_score();
		}
		else {
			// Step 3. Evaluate material
			material<WHITE>();
			material<BLACK>();

			// Step 4. Evaluate piece placements
			psqt<WHITE>(); psqt<BLACK>();

			// Step 5. Material imbalances
			imbalance<WHITE>(); imbalance<BLACK>();

			// Step 6. Pawn structure evaluation
			pawns<WHITE>(); pawns<BLACK>();

			// Step 7. Space evaluation
			space<WHITE>(); space<BLACK>();

			// Step 8. Mobility
			mobility<WHITE, KNIGHT>(); mobility<BLACK, KNIGHT>();
			mobility<WHITE, BISHOP>(); mobility<BLACK, BISHOP>();
			mobility<WHITE, ROOK>(); mobility<BLACK, ROOK>();
			mobility<WHITE, QUEEN>(); mobility<BLACK, QUEEN>();

			// Step 9. King safety evaluation.
			//king_safety<WHITE>(); king_safety<BLACK>();

			// Step 10. Compute the phase and interpolate the middle game and endgame scores.
			int phase = game_phase();

			v = (phase * mg_score + (24 - phase) * eg_score) / 24;

			// Step 11. Store the evaluation in the hash table (white's POV)
			eval_table.store(pos->posKey, v);
		}

		// Step 12. Add tempo for the side to move, make the score side-relative and return
		v += (pos->side_to_move == WHITE) ? tempo : -tempo;
		v *= (pos->side_to_move == WHITE) ? 1 : -1;

		return v;
	}


	/// <summary>
	/// Calculate the game phase based on the amount of material left on the board.
	/// </summary>
	/// <returns>A number between 0 and 24 representing the game phase.</returns>
	template<EvalType T>
	int Evaluate<T>::game_phase() {
		// We calculate the game phase by giving 1 point for each bishop and knight, 2 for each rook and 4 for each queen.
		// This gives the starting position a phase of 24.
		int p = 0;

		p += 1 * (countBits(pos->pieceBBS[KNIGHT][WHITE] | pos->pieceBBS[KNIGHT][BLACK]));
		p += 1 * (countBits(pos->pieceBBS[BISHOP][WHITE] | pos->pieceBBS[BISHOP][BLACK]));
		p += 2 * (countBits(pos->pieceBBS[ROOK][WHITE] | pos->pieceBBS[ROOK][BLACK]));
		p += 4 * (countBits(pos->pieceBBS[QUEEN][WHITE] | pos->pieceBBS[QUEEN][BLACK]));

		return std::min(24, p);
	}


	/// <summary>
	/// Clear the Evaluate object.
	/// </summary>
	template<EvalType T>
	void Evaluate<T>::clear() {
		// Clear the EvalData.
		Data = ZeroData;

		// Clear the scores and the position pointer.
		mg_score = eg_score = 0;
		pos = nullptr;
	}


	/* --------------------------------------------------------------- */
	/* -------------------- Evaluation sub-terms --------------------- */
	/* --------------------------------------------------------------- */

	/// <summary>
	/// Evaluate the material on the board for side S.
	/// </summary>
	template<EvalType T> template<SIDE S>
	void Evaluate<T>::material() {
		int mg = 0;
		int eg = 0;

		// Step 1. Get the number of each material type
		int pawnCnt = countBits(pos->pieceBBS[PAWN][S]);
		int knightCnt = countBits(pos->pieceBBS[KNIGHT][S]);
		int bishopCnt = countBits(pos->pieceBBS[BISHOP][S]);
		int rookCnt = countBits(pos->pieceBBS[ROOK][S]);
		int queenCnt = countBits(pos->pieceBBS[QUEEN][S]);

		// Step 2. Add middlegame values
		mg += pawnCnt * pawn_value.mg;
		mg += knightCnt * knight_value.mg;
		mg += bishopCnt * bishop_value.mg;
		mg += rookCnt * rook_value.mg;
		mg += queenCnt * queen_value.mg;

		// Step 3. Add endgame values
		eg += pawnCnt * pawn_value.eg;
		eg += knightCnt * knight_value.eg;
		eg += bishopCnt * bishop_value.eg;
		eg += rookCnt * rook_value.eg;
		eg += queenCnt * queen_value.eg;


		// Step 4. Add the values to eval and make it side-dependent
		mg_score += (S == WHITE) ? mg : -mg;
		eg_score += (S == WHITE) ? eg : -eg;
	}


	// Anonymous namespace for adding piece-square-table values.
	namespace {
		const Score* tables[6] = { &PSQT::PawnTable[0], &PSQT::KnightTable[0], &PSQT::BishopTable[0], &PSQT::RookTable[0], &PSQT::QueenTable[0], &PSQT::KingTable[0] };

		/// <summary>
		/// 
		/// </summary>
		/// <param name="pce"></param>
		/// <param name="sq"></param>
		/// <returns></returns>
		template<SIDE side, GamePhase p>
		int addPsqtVal(int pce, int sq) {
			int v = 0;

			assert(p == MG || p == EG);
			assert(side == WHITE || side == BLACK);
			assert(sq >= 0 && sq <= 63);
			assert(pce >= PAWN && pce < NO_TYPE);
			assert(PSQT::Mirror64[PSQT::Mirror64[sq]] == sq);

			v += (p == MG) ? tables[pce][(side == WHITE) ? sq : PSQT::Mirror64[sq]].mg : tables[pce][(side == WHITE) ? sq : PSQT::Mirror64[sq]].eg;

			return v;
		}
	} // namespace



	/// <summary>
	/// Evaluate the placement of a side's pieces using piece-square tables.
	/// </summary>
	template<EvalType T> template<SIDE S>
	void Evaluate<T>::psqt() {
		int mg = 0;
		int eg = 0;

		Bitboard pceBoard = 0;
		int sq = NO_SQ;

		for (int pce = PAWN; pce <= KING; pce++) {
			pceBoard = pos->pieceBBS[pce][S];

			while (pceBoard) {
				sq = PopBit(&pceBoard);

				mg += addPsqtVal<S, MG>(pce, sq);
				eg += addPsqtVal<S, EG>(pce, sq);
			}
		}

		// Add the side-relative scores.
		mg_score += (S == WHITE) ? mg : -mg;
		eg_score += (S == WHITE) ? eg : -eg;
	}


	/// <summary>
	/// Evaluate the material imbalances in the position. This is an approximation of the principle that different piece-combinations might not have the 
	/// same value as their individual values summed together.
	/// </summary>
	template<EvalType T> template<SIDE S>
	void Evaluate<T>::imbalance() {
		int mg = 0;
		int eg = 0;

		// Step 1. Bishop pair bonus. FIXME: Should we also have the square-colors of the bishops as a requirement?
		if (countBits(pos->pieceBBS[BISHOP][S]) >= 2) {
			mg += bishop_pair.mg;
			eg += bishop_pair.eg;
		}

		int pawns_removed = 8 - countBits(pos->pieceBBS[PAWN][S]);

		// Step 2. Give rooks bonuses as pawns disappear.
		int rook_count = countBits(pos->pieceBBS[ROOK][S]);

		mg += rook_count * pawns_removed * rook_pawn_bonus.mg;
		eg += rook_count * pawns_removed * rook_pawn_bonus.eg;

		// Step 3. Give the knights penalties as pawns dissapear.
		int knight_count = countBits(pos->pieceBBS[KNIGHT][S]);

		mg -= knight_count * pawns_removed * knight_pawn_penaly.mg;
		eg -= knight_count * pawns_removed * knight_pawn_penaly.eg;

		// Step 4. Store the side-relative scores.
		mg_score += (S == WHITE) ? mg : -mg;
		eg_score += (S == WHITE) ? eg : -eg;
	}



	/// <summary>
	/// Evaluate the pawn-structure. This includes things like doubled, passed, isolated pawns etc..
	/// </summary>
	template<EvalType T> template<SIDE S>
	void Evaluate<T>::pawns() {
		int mg = 0;
		int eg = 0;

		// Declare some side-relative constants
		constexpr Bitboard* passedBitmask = (S == WHITE) ? BBS::EvalBitMasks::passed_pawn_masks[WHITE] : BBS::EvalBitMasks::passed_pawn_masks[BLACK];

		constexpr int relative_ranks[8] = { (S == WHITE) ? RANK_1 : RANK_8, (S == WHITE) ? RANK_2 : RANK_7,
			(S == WHITE) ? RANK_3 : RANK_6, (S == WHITE) ? RANK_4 : RANK_5, (S == WHITE) ? RANK_5 : RANK_4,
			(S == WHITE) ? RANK_6 : RANK_3, (S == WHITE) ? RANK_7 : RANK_2, (S == WHITE) ? RANK_8 : RANK_1 };

		constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;

		constexpr DIRECTION downLeft = (S == WHITE) ? SOUTHWEST : NORTHWEST;
		constexpr DIRECTION downRight = (S == WHITE) ? SOUTHEAST : NORTHEAST;
		constexpr DIRECTION upLeft = (S == WHITE) ? NORTHWEST : SOUTHWEST;
		constexpr DIRECTION upRight = (S == WHITE) ? NORTHEAST : SOUTHEAST;


		Bitboard pawnBoard = pos->pieceBBS[PAWN][S];
		int sq = NO_SQ;
		int relative_sq = NO_SQ; // For black it is PSQT::Mirror64[sq] but for white it is just == sq


		// Before evaluating all pawns, we will score the amount of doubled pawns by file.
		int doubled_count = 0;
		for (int f = FILE_A; f <= FILE_H; f++) {
			doubled_count += (countBits(BBS::FileMasks8[f] & pos->pieceBBS[PAWN][S]) > 1) ? 1 : 0;
		}

		mg -= doubled_count * doubled_penalty.mg;
		eg -= doubled_count * doubled_penalty.eg;


		// Now evaluate each individual pawn
		while (pawnBoard) {
			sq = PopBit(&pawnBoard);
			relative_sq = (S == WHITE) ? sq : PSQT::Mirror64[sq];

			int r = relative_ranks[sq / 8];
			int f = sq % 8;

			// Passed pawn bonus
			if ((passedBitmask[sq] & pos->pieceBBS[PAWN][Them]) == 0) { // No enemy pawns in front
				mg += passedPawnTable[relative_sq].mg;
				eg += passedPawnTable[relative_sq].eg;

				// Save the passed pawn's position such that we can give a bonus if it is defended by pieces later.
				Data.passed_pawns[S] |= (uint64_t(1) << sq);
			}

			// Isolated penalty and/or doubled
			bool doubled = (countBits(BBS::FileMasks8[f] & pos->pieceBBS[PAWN][S]) > 1) ? true : false;
			bool isolated = ((BBS::EvalBitMasks::isolated_bitmasks[f] & pos->pieceBBS[PAWN][S]) == 0) ? true : false;

			if (doubled && isolated) {
				mg -= doubled_isolated_penalty.mg;
				eg -= doubled_isolated_penalty.eg;
			}
			else if (isolated) {
				mg -= isolated_penalty.mg;
				eg -= isolated_penalty.eg;
			}
		}

		// Populate the attacks bitboard with pawn attacks. This will be used in the evaluation of pieces.
		Data.attacked_by_two[S] = attacked_by_all<S>() & (shift<upRight>(pos->pieceBBS[PAWN][S]) | shift<upLeft>(pos->pieceBBS[PAWN][S]));
		Data.attacks[S][PAWN] = (shift<upRight>(pos->pieceBBS[PAWN][S]) | shift<upLeft>(pos->pieceBBS[PAWN][S]));

		// Lastly, evaluate the kings pawn shelter
		king_pawns<S>();

		// Apply the scores.
		mg_score += (S == WHITE) ? mg : -mg;
		eg_score += (S == WHITE) ? eg : -eg;
	}



	/// <summary>
	/// Score the pawn shelter and pawn storm around the king.
	/// </summary>
	template<EvalType T> template<SIDE S>
	void Evaluate<T>::king_pawns() {
		// Constants
		constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;
		constexpr int relative_ranks[8] = { (S == WHITE) ? RANK_1 : RANK_8, (S == WHITE) ? RANK_2 : RANK_7,
			(S == WHITE) ? RANK_3 : RANK_6, (S == WHITE) ? RANK_4 : RANK_5, (S == WHITE) ? RANK_5 : RANK_4,
			(S == WHITE) ? RANK_6 : RANK_3, (S == WHITE) ? RANK_7 : RANK_2, (S == WHITE) ? RANK_8 : RANK_1 };

		// Step 1. Variable initialization
		Score kp_eval, kp_safety;
			
		Bitboard our_pawns = pos->pieceBBS[PAWN][S];
		Bitboard their_pawns = pos->pieceBBS[PAWN][Them];
		int sq = NO_SQ;
		int king_sq = pos->king_squares[S];

		// Idea from Stockfish: Clamp the king-file between the B- and G-file such that we still evaluate the F-pawn when the king is on the H-file for example.
		int king_file = std::clamp(king_sq % 8, 1, 6); 
		int real_king_file = king_sq % 8;
		
		// Step 2. Find the closest pawn and score the king based on the Manhattan-Distance.
		// Note: A distance of 15 counts as no pawns being left on the board.
		int shortest_dist = 15;

		if (our_pawns != 0) {
			while (our_pawns) {
				sq = PopBit(&our_pawns);

				shortest_dist = std::min(shortest_dist, PSQT::ManhattanDistance[king_sq][sq]);
			}
		}
		kp_eval += minimum_kp_distance[shortest_dist - 1];

		// Since we have removed bits from our_pawns, we need to set it again.
		our_pawns = pos->pieceBBS[PAWN][S];
		

		// Step 3. Loop through the files adjacent to the king-file and score the pawn shelter/storm.
		for (int f = king_file - 1; f <= king_file + 1; f++) {

			// Step 3A. Evaluate the pawn shelter based on the which side the king is on and the pawn's square.
			int sq = backmost_sq<S>(pos->pieceBBS[PAWN][S] & BBS::FileMasks8[f]);
			if (sq == NO_SQ) { continue; }
			
			if (real_king_file >= FILE_E) {
				kp_eval += king_shelter[0][(S == WHITE) ? sq : PSQT::Mirror64[sq]];
			}
			else {
				kp_eval += king_shelter[1][(S == WHITE) ? sq : PSQT::Mirror64[sq]];
			}

		}

		// Lastly, add the eval scores.
		mg_score += (S == WHITE) ? kp_eval.mg : -kp_eval.mg;
		eg_score += (S == WHITE) ? kp_eval.eg : -kp_eval.eg;
	}



	/// <summary>
	/// Evaluate the space. This is a term for how much of the board is controlled by one side.
	/// </summary>
	template<EvalType T> template<SIDE S>
	void Evaluate<T>::space() {
		int points = 0;

		// The main space area is rank 3, 4, 5 and 6, and file c, d, e, f
		constexpr Bitboard war_zone = (BBS::FileMasks8[FILE_C] | BBS::FileMasks8[FILE_D] | BBS::FileMasks8[FILE_E] | BBS::FileMasks8[FILE_F])
			& (BBS::RankMasks8[RANK_3] | BBS::RankMasks8[RANK_4] | BBS::RankMasks8[RANK_5] | BBS::RankMasks8[RANK_6]);
		constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;

		// A space point is given for squares not attacked by enemy pawns and either 1) defended by our own, or 2) behind our own.
		// Therefore we need to define the rearspan of our pawns.
		Bitboard rearSpanBrd = 0;

		Bitboard pawnBrd = pos->pieceBBS[PAWN][S];
		int sq = 0;
		while (pawnBrd) {
			sq = PopBit(&pawnBrd);

			rearSpanBrd |= BBS::EvalBitMasks::rear_span_masks[S][sq];
		}

		Bitboard space_zone = war_zone & ~Data.attacks[Them][PAWN]; // Don't consider squares attacked by enemy.

		points += countBits(Data.attacks[S][PAWN]);
		points += 2 * countBits(rearSpanBrd & space_zone);
		
		// Apply the scores based on our space points
		mg_score += (S == WHITE) ? space_bonus[std::min(31, points)].mg : -space_bonus[std::min(31, points)].mg;
		eg_score += (S == WHITE) ? space_bonus[std::min(31, points)].eg : -space_bonus[std::min(31, points)].eg;
	}



	/// <summary>
	/// Mobility evaluation. If our pieces have a lot of squares to move to, it is usually a sign that we have a good position.
	/// </summary>
	template<EvalType T> template<SIDE S, piece pce>
	void Evaluate<T>::mobility() {
		int mg = 0;
		int eg = 0;

		constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;
		constexpr DIRECTION Down = (S == WHITE) ? SOUTH : NORTH;

		Bitboard enemy_king_ring = king_ring(pos->king_squares[Them]);
		Bitboard enemy_outer_king_ring = outer_kingRing(pos->king_squares[Them]);

		Bitboard friendly_outer_king_ring = outer_kingRing(pos->king_squares[S]);

		Bitboard piece_attacks = 0; // Individual piece attacks.

		Bitboard pceBoard = pos->pieceBBS[pce][S];
		int sq = 0;


		if (pce < KNIGHT || pce > QUEEN || pceBoard == 0) {
			return;
		}

		// For mobility, we'll only score moves to squares not attacked by pawns, and not to pawns that are blocked.
		Bitboard good_squares = ~(Data.attacks[Them][PAWN] | pos->pieceBBS[KING][S] |
			(shift<Down>(pos->all_pieces[WHITE] | pos->all_pieces[BLACK]) & pos->pieceBBS[PAWN][S]));

		int attack_cnt = 0;
		while (pceBoard != 0) {
			sq = PopBit(&pceBoard);

			if constexpr (pce == KNIGHT) {
				// Calculate attacks and update the EvalData object.
				piece_attacks = BBS::knight_attacks[sq];

				Data.attacked_by_two[S] |= attacked_by_all<S>() & piece_attacks;
				Data.attacks[S][pce] |= piece_attacks;

				

				if ((piece_attacks & enemy_king_ring) != 0) { // If the piece attacks the enemy king
					Data.king_zone_attacks[Them]++; // Increment attackers

					// Add attack units to index the king attack table
					Data.king_safety_units[Them] += 2;
				}

				piece_attacks &= good_squares; // Only score mobility to good squares.

				attack_cnt = countBits(piece_attacks);
				assert(attack_cnt < 9);

				mg += mobility_bonus[pce - 1][attack_cnt].mg;
				eg += mobility_bonus[pce - 1][attack_cnt].eg;
			}

			else if constexpr (pce == BISHOP) {
				// Calculate attacks and update the EvalData object.
				piece_attacks = Magics::attacks_bb<BISHOP>(sq, (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]));
				
				Data.attacked_by_two[S] |= attacked_by_all<S>() & piece_attacks;
				Data.attacks[S][pce] |= piece_attacks;

				if ((piece_attacks & enemy_king_ring) != 0) { // If the piece attacks the enemy king
					Data.king_zone_attacks[Them]++; // Increment attackers

					// Add attack units to index the king attack table
					Data.king_safety_units[Them] += 2;
				}


				piece_attacks &= good_squares; // Only score mobility to good squares.

				attack_cnt = countBits(piece_attacks);
				assert(attack_cnt < 15);

				mg += mobility_bonus[pce - 1][attack_cnt].mg;
				eg += mobility_bonus[pce - 1][attack_cnt].eg;
			}

			else if constexpr (pce == ROOK) {
				// Calculate attacks and update the EvalData object.
				piece_attacks = Magics::attacks_bb<ROOK>(sq, (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]));

				Data.attacked_by_two[S] |= attacked_by_all<S>() & piece_attacks;
				Data.attacks[S][pce] |= piece_attacks;

				if ((piece_attacks & enemy_king_ring) != 0) { // If the piece attacks the enemy king
					Data.king_zone_attacks[Them]++; // Increment attackers

					// Add attack units to index the king attack table
					Data.king_safety_units[Them] += 3;
				}

				piece_attacks &= good_squares; // Only score mobility to good squares.

				attack_cnt = countBits(piece_attacks);

				mg += mobility_bonus[pce - 1][attack_cnt].mg;
				eg += mobility_bonus[pce - 1][attack_cnt].eg;
			}

			else if constexpr (pce == QUEEN) {
				// Calculate attacks and update the EvalData object.
				piece_attacks = Magics::attacks_bb<QUEEN>(sq, (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]));

				Data.attacked_by_two[S] |= attacked_by_all<S>() & piece_attacks;
				Data.attacks[S][pce] |= piece_attacks;

				if ((piece_attacks & enemy_king_ring) != 0) { // If the piece attacks the enemy king
					Data.king_zone_attacks[Them]++; // Increment attackers

					// Add attack units to index the king attack table
					Data.king_safety_units[Them] += 5;
				}

				piece_attacks &= good_squares; // Only score mobility to good squares.

				attack_cnt = countBits(piece_attacks);
				assert(attack_cnt < 29);

				mg += mobility_bonus[pce - 1][attack_cnt].mg;
				eg += mobility_bonus[pce - 1][attack_cnt].eg;
			}

			else { // Just in case we went into the loop without a proper piece-type.
				assert(pce >= KNIGHT && pce <= QUEEN); // Raise an error.
				return;
			}
		}

		
		mg_score += (S == WHITE) ? mg : -mg;
		eg_score += (S == WHITE) ? eg : -eg;
	}
	




	/// <summary>
	/// Return a bitboard of all (currently calculated) attacks by one of the sides.
	/// </summary>
	/// <returns>Bitboard with all known attacks.</returns>
	template<EvalType T> template<SIDE S>
	Bitboard Evaluate<T>::attacked_by_all() {
		return (Data.attacks[S][PAWN] | Data.attacks[S][KNIGHT] | Data.attacks[S][BISHOP]
			| Data.attacks[S][ROOK] | Data.attacks[S][QUEEN] | king_ring(pos->king_squares[S]));
	}

	Bitboard king_flanks[8] = { 0 };

	/// <summary>
	/// Initialize the king_flanks array.
	/// </summary>
	void initKingFlanks() {
		for (int f = FILE_A; f <= FILE_H; f++) {
			if (f < FILE_D) {
				king_flanks[f] = (BBS::FileMasks8[FILE_A] | BBS::FileMasks8[FILE_B] | BBS::FileMasks8[FILE_C]);
			}
			else if (f == FILE_D) {
				king_flanks[f] = (BBS::FileMasks8[FILE_C] | BBS::FileMasks8[FILE_D] | BBS::FileMasks8[FILE_E]);
			}
			else if (f == FILE_E) {
				king_flanks[f] = (BBS::FileMasks8[FILE_D] | BBS::FileMasks8[FILE_E] | BBS::FileMasks8[FILE_F]);
			}
			else if (f > FILE_E) {
				king_flanks[f] = (BBS::FileMasks8[FILE_F] | BBS::FileMasks8[FILE_G] | BBS::FileMasks8[FILE_H]);
			}
		}
	}

	// Initialize the evaulation function. At the moment this is just the king flanks.
	void INIT() {
		initKingFlanks();
	}
}




/// <summary>
/// Loop through a set of positions, evaluate each one, mirror it and evaluate it again. These two values should be the same, otherwise the evaluation function is broken.
/// </summary>
void Eval::Debug::eval_balance() {

	GameState_t* pos = new GameState_t;
	Evaluate<NORMAL> eval;

	int total = 0;
	int passed = 0;
	int failed = 0;

	int w_ev = 0;
	int b_ev = 0;

	for (int p = 0; p < test_positions.size(); p++) {
		total++;
		pos->parseFen(test_positions[p]);

		w_ev = eval.score(pos, false);

		pos->mirror_board();

		b_ev = eval.score(pos, false);


		if (w_ev == b_ev) {
			std::cout << "Position " << (p + 1) << "	--->	" << "PASSED:" << " " << w_ev << " == " << b_ev << std::endl;
			passed++;
		}
		else {
			std::cout << "Position " << (p + 1) << "	--->	" << "FAILED: " << w_ev << " != " << b_ev << "		(FEN: " << test_positions[p] << ")" << std::endl;
			failed++;
		}
	}

	std::cout << total << " positions analyzed." << std::endl;
	std::cout << passed << " positions passed." << std::endl;
	std::cout << failed << " positions failed. (" << (double(failed) / double(total)) * 100.0 << "%)" << std::endl;

	delete pos;
}