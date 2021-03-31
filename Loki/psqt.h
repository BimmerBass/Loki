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
	extern const int PawnTableMg[64];
	extern const int PawnTableEg[64];

	extern const int KnightTableMg[64];
	extern const int KnightTableEg[64];

	extern const int BishopTableMg[64];
	extern const int BishopTableEg[64];

	extern const int RookTableMg[64];
	extern const int RookTableEg[64];

	extern const int QueenTableMg[64];
	extern const int QueenTableEg[64];

	extern const int KingTableMg[64];
	extern const int KingTableEg[64];

	/*
	Other square tables
	*/
	extern const Score passedPawnTable[64];

	extern const int Mirror64[64];

	extern int ManhattanDistance[64][64];
	
	extern const std::vector<std::vector<Score>> mobilityBonus;


	/*
	King-safety specific tables
	*/
	extern const Score safety_table[100];

	extern const int castledPawnAdvancementMg[64];
	extern const Score pawnStorm[64];

	extern const Score king_pawn_distance_penalty[8];

	void initManhattanDistance();

	void INIT();
}








#endif