#ifndef PAWNHASH_H
#define PAWNHASH_H


#include "defs.h"



// Scores are stored relative to white.
struct PH_Entry {
	Bitboard key = 0;

	int mg_value = 0, eg_value = 0;

	Bitboard passers[2] = { 0 };
	Bitboard attacks[2] = { 0 };
};


class PTable {
public:
	PTable(size_t size_kb);
	~PTable();

	void store_entry(Bitboard key, int mg_score, int eg_score, Bitboard* passed_pawns, Bitboard* pawn_attacks);

	PH_Entry* probe(Bitboard pawn_key, bool& hit);

	void clear();
private:
	PH_Entry* entries = nullptr;

	size_t num_entries = 0;
};










#endif