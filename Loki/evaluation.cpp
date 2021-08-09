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


// King safety evaluation
const Score missing_king_pawn(58, 5);
const Score no_enemy_queen(-258, -298);
Score weak_king_square(-31, -8);


const Score king_pawn_shelter[8][7] = {
	{ S(7, 3)	,	S(5, 3)		,	S(18, 16)	,	S(22, 18)	,	S(40, 36)	, S(46, 44)	,	S(47, 53) },	/* File A */
	{ S(1, -3)	,	S(-9, 5)	,	S(16, 2)	,	S(26, 14)	,	S(30, 40)	, S(46, 44)	,	S(47, 57) },	/* File B */
	{ S(3, -7)	,	S(-7, -3)	,	S(12, 10)	,	S(20, 26)	,	S(38, 36)	, S(42, 50)	,	S(41, 47) },	/* File C */
	{ S(-7, -3)	,	S(3, -1)	,	S(-9, -1)	,	S(-1, -11)	,	S(-7, -7)	, S(-5, 3)	,	S(-11, 3) },	/* File D */
	{ S(3, 7)	,	S(3, -7)	,	S(1, 11)	,	S(-3, 5)	,	S(1, -9)	, S(-11, -3),	S(3, 7)	  },	/* File E */
	{ S(7, 5)	,	S(-1, -5)	,	S(0, 4)		,	S(26, 18)	,	S(32, 32)	, S(48, 34)	,	S(49, 49) },	/* File F */
	{ S(-1, -3)	,	S(-9, 19)	,	S(6, 18)	,	S(12, 32)	,	S(32, 26)	, S(50, 40)	,	S(53, 39) },	/* File G */
	{ S(-3, 1)	,	S(3, 9)		,	S(22, 18)	,	S(18, 10)	,	S(34, 20)	, S(42, 42)	,	S(43, 55) }		/* File H */
	/*	1sq			  2sq			  3sq			  4sq			  5sq			  6sq			7sq  */
};



const Score king_pawn_storm[8][7] = {
	{ S(-15, -3),	S(-3, -5)	,	S(-8, -10)	,	S(-3, 1)	,	S(-12, -14)	,	S(-23, -17)	,	S(-27, -35) },	/* File A */
	{ S(-11, 7)	,	S(13, -13)	,	S(-8, 14)	,	S(-15, -3)	,	S(-14, -22)	,	S(-25, -25)	,	S(-31, -19) },	/* File B */
	{ S(5, 3)	,	S(9, 9)		,	S(0, -12)	,	S(-5, -1)	,	S(-14, -8)	,	S(-9, -21)	,	S(-29, -25) },	/* File C */
	{ S(13, -3)	,	S(7, 1)		,	S(-8, 0)	,	S(-9, -11)	,	S(-8, -10)	,	S(-1, -23)	,	S(-19, -35) },	/* File D */
	{ S(-3, 9)	,	S(-3, 3)	,	S(2, -6)	,	S(1, -17)	,	S(-18, -6)	,	S(-19, 3)	,	S(-35, -29) },	/* File E */
	{ S(11, 3)	,	S(3, -7)	,	S(-2, -2)	,	S(-17, -7)	,	S(-12, -16)	,	S(-15, -3)	,	S(-19, -15) },	/* File F */
	{ S(-25, -9),	S(-5, -17)	,	S(-2, -16)	,	S(-19, -7)	,	S(-10, 0)	,	S(-33, -1)	,	S(-49, -15) },	/* File G */
	{ S(3, 1)	,	S(-3, -11)	,	S(2, -2)	,	S(3, -13)	,	S(-16, 6)	,	S(-13, -9)	,	S(-21, -15) } 	/* File H */
	/*	1sq			  2sq			  3sq			  4sq			  5sq			  6sq				7sq		*/
};


const Score defending_minors[4][3][3] = {
	{{S(0, 0), S(15, 15), S(10, 10)}		,	{S(0, 0),	S(10, 10),	S(-5, -5)}		,	{S(0, 0), S(15, 15), S(10, 10)}},		/* 0 Defending pawns */
	{{S(0, 0), S(10, 10), S(5, 5)}			,	{S(0, 0),	S(5, 5),	S(-10, -10)}	,	{S(0, 0), S(10, 10), S(5, 5)}},			/* 1 Defending pawn  */
	{{S(0, 0), S(-5, -5), S(-10, -10)}		,	{S(0, 0),	S(-10, -10), S(-15, -15)}	,	{S(0, 0), S(-5, -5), S(-10, -10)}},		/* 2 Defending pawns */
	{{S(0, 0), S(-10, -10), S(-15, -15)}	,	{S(0, 0),	S(-15, -15), S(-20, -20)}	,	{S(0, 0), S(-10, -10), S(-15, -15)}} 	/* 3 Defending pawns */
	/*			0 knights									 1 knight									2 knights				*/
};


const Score safety_table[100] = {
		S(4, -2),	S(-8, 10)	,	S(2, 0)		,	S(-19, -8)	,	S(-15, -19)	,	S(10, -22)	,	S(5, -35)	,	S(29, -15)	,	S(46, 8)	,	S(69, 49)	,
	S(40, 24)	,	S(73, 47)	,	S(56, 22)	,	S(39, 3)	,	S(28, 22)	,	S(13, 17)	,	S(24, 12)	,	S(27, 35)	,	S(14, 22)	,	S(23, 19)	,
	S(14, 42)	,	S(43, 23)	,	S(16, 40)	,	S(39, 31)	,	S(45, 41)	,	S(51, 47)	,	S(65, 51)	,	S(80, 62)	,	S(91, 87)	,	S(75, 115)	,
	S(97, 79)	,	S(89, 111)	,	S(109, 105)	,	S(111, 117)	,	S(117, 125)	,	S(149, 147)	,	S(131, 159)	,	S(155, 149)	,	S(179, 191)	,	S(163, 163)	,
	S(171, 179)	,	S(171, 171)	,	S(207, 181)	,	S(209, 211)	,	S(209, 237)	,	S(229, 211)	,	S(237, 227)	,	S(265, 231)	,	S(259, 255)	,	S(267, 251)	,
	S(253, 245)	,	S(259, 251)	,	S(247, 265)	,	S(285, 289)	,	S(281, 269)	,	S(301, 303)	,	S(303, 301)	,	S(317, 317)	,	S(327, 345)	,	S(339, 327)	,
	S(329, 337)	,	S(319, 321)	,	S(347, 325)	,	S(353, 363)	,	S(391, 377)	,	S(395, 373)	,	S(383, 373)	,	S(417, 401)	,	S(417, 395)	,	S(427, 423)	,
	S(423, 403)	,	S(417, 415)	,	S(431, 421)	,	S(437, 443)	,	S(419, 455)	,	S(453, 471)	,	S(465, 463)	,	S(471, 495)	,	S(481, 493)	,	S(499, 495)	,
	S(473, 485)	,	S(505, 497)	,	S(501, 527)	,	S(515, 527)	,	S(537, 533)	,	S(545, 549)	,	S(555, 541)	,	S(557, 549)	,	S(561, 569)	,	S(575, 567)	,
	S(563, 565)	,	S(601, 563)	,	S(585, 585)	,	S(585, 581)	,	S(579, 615)	,	S(619, 621)	,	S(627, 647)	,	S(643, 633)	,	S(635, 641)	,	S(643, 649)
};


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
			king_safety<WHITE>(); king_safety<BLACK>();

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

		// Apply the scores.
		mg_score += (S == WHITE) ? mg : -mg;
		eg_score += (S == WHITE) ? eg : -eg;
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
				else if ((piece_attacks & enemy_outer_king_ring) != 0) { // If it can move to a square that (probably) attacks the king
					Data.king_zone_attacks[Them]++;
				
					// Since we're not directly attacking the king, only add half the attack units
					Data.king_safety_units[Them] += 1;
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
				else if ((piece_attacks & enemy_outer_king_ring) != 0) { // If it can move to a square that (probably) attacks the king
					Data.king_zone_attacks[Them]++;

					// Since we're not directly attacking the king, only add half the attack units
					Data.king_safety_units[Them] += 1;
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
				else if ((piece_attacks & enemy_outer_king_ring) != 0) { // If it can move to a square that (probably) attacks the king
					Data.king_zone_attacks[Them]++;

					// Since we're not directly attacking the king, only add half the attack units
					Data.king_safety_units[Them] += 2;
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
				else if ((piece_attacks & enemy_outer_king_ring) != 0) { // If it can move to a square that (probably) attacks the king
					Data.king_zone_attacks[Them]++;

					// Since we're not directly attacking the king, only add half the attack units
					Data.king_safety_units[Them] += 3;
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
	


	namespace {

		template<SIDE S>
		bool defended_by_pawn(const GameState_t* pos, int sq) {
			constexpr DIRECTION upRight = (S == WHITE) ? NORTHEAST : SOUTHEAST;
			constexpr DIRECTION upLeft = (S == WHITE) ? NORTHWEST : SOUTHWEST;

			return ((shift<upRight>(pos->pieceBBS[PAWN][S]) | shift<upLeft>(pos->pieceBBS[PAWN][S])) & (uint64_t(1) << sq)) != 0;
		}
		
		/// <summary>
		/// Score the pawn shield around the king.
		/// </summary>
		/// <param name="pos">The position</param>
		/// <returns>A score with safety values</returns>
		template<EvalType T, SIDE S>
		Score king_pawns(const GameState_t* pos) {
			// Step 1. Variable declaration.
			constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;

			Score safety;

			int king_sq = pos->king_squares[S];
			int king_file = king_sq % 8;
			int sq, dist;

			// Step 2. Evaluate pawn advancement in front of the king.
			// We do this by simply looping over the files adjacent to the king's
			for (int f = std::max(0, king_file - 1); f <= std::min(7, king_file + 1); f++) {
				// Step 2A. Find the closest of our own pawns and score it based on the distance to the king and it's file.
				sq = backmost_sq<S>(BBS::FileMasks8[f] & pos->pieceBBS[PAWN][S]);

				if (sq == NO_SQ) { // There are no pawns.
					safety.mg += missing_king_pawn.mg;
					safety.eg += missing_king_pawn.eg;
				}
				else {
					dist = PSQT::ManhattanDistance[king_sq][sq];

					safety.mg += king_pawn_shelter[f][dist].mg;
					safety.eg += king_pawn_shelter[f][dist].eg;
				}

				// Step 2B. Find the closest enemy pawn and evaluate the pawn storm based on the file and the distance to our king.
				sq = backmost_sq<S>(BBS::FileMasks8[f] & pos->pieceBBS[PAWN][Them]);
				
				if (sq != NO_SQ) {
					dist = PSQT::ManhattanDistance[king_sq][sq];
				
					safety.mg += king_pawn_storm[f][dist].mg;
					safety.eg += king_pawn_storm[f][dist].eg;
				}
			}
			
			return safety;
		}
	}


	/// <summary>
	/// Evaluate the king safety.
	/// </summary>
	template<EvalType T> template<SIDE S>
	void Evaluate<T>::king_safety() {
		// Step 1. Initialize variables
		constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;

		int safety_mg = 0;
		int safety_eg = 0;

		// Step 2. Evaluate the kings pawn shield and pawn storm
		Score kings_pawns = king_pawns<T, S>(pos);
		safety_mg += kings_pawns.mg;
		safety_eg += kings_pawns.eg;

		// Step 3. Only evaluate king safety if either: 1) There are more than two attackers, or 2) There are more than one attacker and the enemy has a queen
		if (Data.king_zone_attacks[S] > 2 || (pos->pieceBBS[QUEEN][Them] != 0 && Data.king_zone_attacks[S] > 1)) {
			// Step 4. Score defending pawns, knights and bishops.
			//Bitboard king_area = king_ring(pos->king_squares[S]) | outer_kingRing(pos->king_squares[S]);
			//int pawn_defenders = std::clamp(countBits(pos->pieceBBS[PAWN][S] & king_area) - 1, 0, 3);
			//int knight_defenders = std::clamp(countBits(pos->pieceBBS[KNIGHT][S] & king_area) - 1, 0, 2);
			//int bishop_defenders = std::clamp(countBits(pos->pieceBBS[BISHOP][S] & king_area) - 1, 0, 2);
			//
			//safety_mg += defending_minors[pawn_defenders][knight_defenders][bishop_defenders].mg;
			//safety_eg += defending_minors[pawn_defenders][knight_defenders][bishop_defenders].eg;
			
			// Step 5. Determine the weak squares around the king.
			Bitboard weak = weak_squares<S>();
			Bitboard weak_count = countBits(weak & king_ring(pos->king_squares[S]));
		
			// Step 6. Now apply the remaining
			safety_mg += safety_table[Data.king_safety_units[S]].mg;
				+ weak_king_square.mg * weak_count
				//+ no_enemy_queen.mg * (pos->pieceBBS[QUEEN][Them] == 0);
		
			safety_eg += safety_table[Data.king_safety_units[S]].eg;
				+ weak_king_square.eg * weak_count
				//+ no_enemy_queen.eg * (pos->pieceBBS[QUEEN][Them] == 0);
		}

		// Step 5. Scale the scores.
		// This is done in order because a safety value of -300cp should be considered 9 times worse than one of -100cp instead of only 3 times.
		int king_safety_mg = -1 * safety_mg * safety_mg / 256;
		int king_safety_eg = -1 * safety_eg * safety_eg / 128;

		mg_score += (S == WHITE) ? king_safety_mg : -king_safety_mg;
		eg_score += (S == WHITE) ? king_safety_eg : -king_safety_eg;
	}


	/// <summary>
	/// Identify weak squares around the king.
	/// </summary>
	/// <returns>A bitboard with all squares attacked more than once and only defended by our king or queen.</returns>
	template<EvalType T> template<SIDE S>
	Bitboard Evaluate<T>::weak_squares() {
		// Weak squares are defined as being attacked more than once and only defended by our king or queen.
		constexpr SIDE Them = (S == WHITE) ? BLACK : WHITE;

		return attacked_by_all<Them>()
			& ~Data.attacked_by_two[S]
			& (~attacked_by_all<S>() | Data.attacks[S][QUEEN] | king_ring(pos->king_squares[S]));
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