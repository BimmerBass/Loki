#ifndef PSQT_H
#define PSQT_H
#include <cmath>


namespace PSQT {
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
	extern const int passedPawnTable[64];

	extern const int Mirror64[64];

	extern int ManhattanDistance[64][64];

	void initManhattanDistance();

	void INIT();
}








#endif