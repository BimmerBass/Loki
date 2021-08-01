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
#ifndef PSQT_H
#define PSQT_H
#include <cmath>
#include <vector>


namespace PSQT {
	class Score {
	public:
		Score(int m, int e) {
			mg = m; eg = e;
		
		}

		Score (const Score &s){
			mg = s.mg;
			eg = s.eg;
		}

		Score() {};

		int mg = 0;
		int eg = 0;
	};

	/*
	Piece square table
	*/

	/*extern const Score PawnTable[64];

	extern const Score KnightTable[64];

	extern const Score BishopTable[64];

	extern const Score RookTable[64];

	extern const Score QueenTable[64];

	extern const Score KingTable[64];
	*/
	extern Score PawnTable[64];

	extern Score KnightTable[64];

	extern Score BishopTable[64];

	extern Score RookTable[64];

	extern Score QueenTable[64];

	extern Score KingTable[64];

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

	void INIT();
}








#endif