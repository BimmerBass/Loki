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

	extern const Score PawnTable[64];

	extern const Score KnightTable[64];

	extern const Score BishopTable[64];

	extern const Score RookTable[64];

	extern const Score QueenTable[64];

	extern const Score KingTable[64];


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