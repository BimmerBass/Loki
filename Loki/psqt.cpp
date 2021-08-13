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




namespace PSQT {

#define S(mg, eg) Score(mg, eg)


	const Score PawnTable[64] = {
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,
		S(-15, 31)	,	S(15, 31)	,	S(-22, 35)	,	S(-4, 27)	,	S(-2, 7)	,	S(32, 25)	,	S(51, 23)	,	S(1, 5)		,
		S(-15, 21)	,	S(7, 23)	,	S(4, 11)	,	S(-11, 29)	,	S(13, 17)	,	S(2, 23)	,	S(47, 17)	,	S(-1, 9)	,
		S(-22, 37)	,	S(-6, 35)	,	S(-16, 11)	,	S(13, 1)	,	S(-1, 15)	,	S(4, 9)		,	S(-8, 33)	,	S(-24, 23)	,
		S(-6, 63)	,	S(3, 51)	,	S(5, 29)	,	S(-8, 39)	,	S(16, 3)	,	S(13, 25)	,	S(-5, 55)	,	S(-26, 49)	,
		S(18, 119)	,	S(-8, 133)	,	S(6, 113)	,	S(12, 89)	,	S(-2, 91)	,	S(30, 87)	,	S(-4, 125)	,	S(32, 133)	,
		S(69, 149)	,	S(27, 145)	,	S(47, 103)	,	S(31, 125)	,	S(23, 115)	,	S(37, 123)	,	S(31, 127)	,	S(15, 133)	,
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)
	};


	const Score KnightTable[64] = {
		S(-55, -37)	,	S(-14, -10)	,	S(-47, -15)	,	S(-51, 15)	,	S(1, -5)	,	S(-33, 13)	,	S(-12, -36)	,	S(-17, -69)	,
		S(-57, 5)	,	S(23, -23)	,	S(-11, -11)	,	S(15, -1)	,	S(17, -3)	,	S(19, 5)	,	S(-17, 29)	,	S(-7, -23)	,
		S(-22, -2)	,	S(-13, 13)	,	S(9, -9)	,	S(5, 19)	,	S(15, 21)	,	S(25, -9)	,	S(37, -19)	,	S(-6, -24)	,
		S(-3, 1)	,	S(29, 5)	,	S(1, 35)	,	S(19, 37)	,	S(27, 29)	,	S(31, 23)	,	S(-1, 31)	,	S(7, -3)	,
		S(-3, 5)	,	S(13, 17)	,	S(13, 31)	,	S(59, 35)	,	S(43, 33)	,	S(79, 21)	,	S(27, 17)	,	S(49, -15)	,
		S(-24, -26)	,	S(31, -1)	,	S(72, -3)	,	S(39, 23)	,	S(100, -11)	,	S(29, 27)	,	S(73, -29)	,	S(8, -12)	,
		S(-7, -39)	,	S(-19, 1)	,	S(47, 7)	,	S(17, 9)	,	S(5, -5)	,	S(35, -13)	,	S(-35, 21)	,	S(9, -57)	,
		S(-59, -52)	,	S(22, -32)	,	S(-43, -29)	,	S(-25, -29)	,	S(1, -7)	,	S(-41, -9)	,	S(-40, 8)	,	S(-5, -73)
	};


	const Score BishopTable[64] = {
		S(9, -29)	,	S(-16, 23)	,	S(-11, 6)	,	S(-2, 3)	,	S(8, 4)		,	S(-16, 10)	,	S(-34, 18)	,	S(-19, 32)	,
		S(73, -46)	,	S(17, -14)	,	S(12, -2)	,	S(-1, 1)	,	S(8, 3)		,	S(6, 0)		,	S(36, -10)	,	S(17, -27)	,
		S(-2, -10)	,	S(19, -12)	,	S(16, -5)	,	S(10, 2)	,	S(13, 8)	,	S(31, -10)	,	S(13, -8)	,	S(10, 8)	,
		S(-50, 16)	,	S(20, -15)	,	S(0, -1)	,	S(22, -2)	,	S(30, -1)	,	S(-5, -9)	,	S(-5, -14)	,	S(48, -33)	,
		S(-4, -5)	,	S(-6, 8)	,	S(-4, -1)	,	S(21, 1)	,	S(23, -8)	,	S(24, -6)	,	S(1, -6)	,	S(-2, 10)	,
		S(-34, 4)	,	S(3, -9)	,	S(7, -17)	,	S(25, -16)	,	S(44, -29)	,	S(54, -20)	,	S(79, -20)	,	S(-40, 17)	,
		S(28, -29)	,	S(-3, -10)	,	S(18, -26)	,	S(48, -38)	,	S(6, -4)	,	S(74, -28)	,	S(-26, 1)	,	S(-95, 13)	,
		S(-34, -1)	,	S(-70, -17)	,	S(36, -10)	,	S(3, -37)	,	S(28, -33)	,	S(-94, 32)	,	S(-73, -56)	,	S(-64, -14)	,
	};

	const Score RookTable[64] = {
		S(-12, -24)	,	S(2, -20)	,	S(21, -23)	,	S(36, -32)	,	S(41, -40)	,	S(27, -34)	,	S(-40, -8)	,	S(-18, -38)	,
		S(-38, -14)	,	S(-5, -23)	,	S(-15, -10)	,	S(-1, -13)	,	S(23, -32)	,	S(33, -32)	,	S(-32, 1)	,	S(-77, 4)	,
		S(-45, -1)	,	S(-9, -14)	,	S(15, -36)	,	S(-14, -15)	,	S(20, -35)	,	S(18, -32)	,	S(14, -30)	,	S(-32, -23)	,
		S(-49, 9)	,	S(-16, -8)	,	S(-26, 8)	,	S(22, -23)	,	S(-5, -12)	,	S(-19, -5)	,	S(25, -23)	,	S(-24, -9)	,
		S(-49, 10)	,	S(23, -23)	,	S(9, 5)		,	S(51, -30)	,	S(44, -21)	,	S(13, 1)	,	S(-22, -8)	,	S(-15, -2)	,
		S(-22, 5)	,	S(-8, 10)	,	S(53, -21)	,	S(27, -1)	,	S(32, -21)	,	S(-43, 7)	,	S(1, -8)	,	S(38, -21)	,
		S(-6, 11)	,	S(15, 7)	,	S(30, 2)	,	S(56, -13)	,	S(91, -29)	,	S(40, -4)	,	S(52, -13)	,	S(-22, 9)	,
		S(27, -15)	,	S(48, -22)	,	S(0, -12)	,	S(26, -16)	,	S(56, -18)	,	S(49, -29)	,	S(48, -25)	,	S(-21, -8)
	};


	const Score QueenTable[64] = {
		S(-13, -7)	,	S(-3, -31)	,	S(-1, -3)	,	S(27, -57)	,	S(-3, -7)	,	S(-15, -45)	,	S(-33, -21)	,	S(-41, 11)	,
		S(-13, -49)	,	S(-17, 5)	,	S(19, -41)	,	S(17, -29)	,	S(27, -35)	,	S(23, -15)	,	S(-3, -33)	,	S(-3, 9)	,
		S(-27, 27)	,	S(17, -47)	,	S(-7, 13)	,	S(3, -1)	,	S(5, 19)	,	S(7, 31)	,	S(21, 33)	,	S(15, 25)	,
		S(-7, -19)	,	S(-27, 15)	,	S(-11, 23)	,	S(-13, 61)	,	S(-11, 53)	,	S(11, 23)	,	S(7, 39)	,	S(7, 33)	,
		S(-35, -1)	,	S(-35, 17)	,	S(-13, 21)	,	S(-9, 31)	,	S(21, 45)	,	S(15, 49)	,	S(37, 13)	,	S(7, 37)	,
		S(-23, -9)	,	S(-37, 25)	,	S(21, -7)	,	S(9, 51)	,	S(67, 43)	,	S(47, 59)	,	S(23, 69)	,	S(53, 19)	,
		S(-33, -15)	,	S(-63, 41)	,	S(15, 25)	,	S(-19, 57)	,	S(-5, 61)	,	S(63, 25)	,	S(51, -17)	,	S(33, 37)	,
		S(-29, -11)	,	S(-23, 47)	,	S(41, 7)	,	S(49, -3)	,	S(25, 41)	,	S(-7, 49)	,	S(-3, 41)	,	S(65, 9)
	};

	const Score KingTable[64] = {
		S(-4, -76)	,	S(41, -57)	,	S(21, -42)	,	S(-40, -39)	,	S(29, -55)	,	S(-21, -34)	,	S(46, -63)	, S(33, -96)	,
		S(-9, -55)	,	S(-3, -39)	,	S(-1, -18)	,	S(-47, -6)	,	S(-35, -4)	,	S(-23, -10)	,	S(13, -29)	, S(15, -49)	,
		S(-30, -38)	,	S(-16, -22)	,	S(-22, -6)	,	S(-40, 13)	,	S(-36, 13)	,	S(-32, 5)	,	S(-4, -14)	, S(-6, -42)	,
		S(-34, -29)	,	S(-2, -13)	,	S(-30, 15)	,	S(-38, 16)	,	S(-34, 18)	,	S(-28, 9)	,	S(-36, -5)	, S(-36, -33)	,
		S(16, -15)	,	S(-2, 12)	,	S(-10, 15)	,	S(-38, 28)	,	S(-40, 16)	,	S(-32, 23)	,	S(-22, 17)	, S(-42, -23)	,
		S(-78, 4)	,	S(-30, 5)	,	S(-30, 18)	,	S(-12, 5)	,	S(-22, 15)	,	S(-16, 31)	,	S(-34, 24)	, S(-24, -2)	,
		S(-38, -25)	,	S(-34, 11)	,	S(-70, 4)	,	S(-52, -4)	,	S(-74, -6)	,	S(-32, -8)	,	S(-80, 17)	, S(-60, -17)	,
		S(-84, -46)	,	S(-90, -39)	,	S(-56, -26)	,	S(-62, -53)	,	S(-84, -19)	,	S(-46, -6)	,	S(-36, -59)	, S(-56, -40)
	};




	// This is to make the psqt give the same values for black pieces. For example square a1 for white corresponds to square h8 for black.
	const int Mirror64[64] = {
		56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
		48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
		40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
		32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
		24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
		16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
		8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
		0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
	};


#undef S


	/// <summary>
	/// Compute the manhattan distance between two squares.
	/// </summary>
	/// <param name="sq1">The first square</param>
	/// <param name="sq2">The second square</param>
	/// <returns>The amount of moves it would take a king to get from sq1 to sq2 and vice versa (Manhattan Distance)</returns>
	int compute_md(int sq1, int sq2) {
		int file1, file2, rank1, rank2;

		int rankDistance, fileDistance;

		file1 = sq1 % 8; rank1 = sq1 / 8;
		file2 = sq2 % 8; rank2 = sq2 / 8;

		rankDistance = abs(rank2 - rank1);
		fileDistance = abs(file2 - file1);

		return rankDistance + fileDistance;
	}


	int ManhattanDistance[64][64] = {};

	/// <summary>
	/// Initialize the Manhattan Distance table
	/// </summary>
	void initManhattanDistance() {

		for (int sq1 = 0; sq1 < 64; sq1++) {

			for (int sq2 = 0; sq2 < 64; sq2++) {
				ManhattanDistance[sq1][sq2] = compute_md(sq1, sq2);
			}

		}
	}

	/// <summary>
	/// Initialize the psqt's. For now, it's only the Manhattan-Distance table.
	/// </summary>
	void INIT() {
		initManhattanDistance();
	}
}