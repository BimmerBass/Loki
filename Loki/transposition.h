#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "position.h"

#include <vector>


enum TT_FLAG :int { ALPHA = 0, BETA = 1, EXACT = 2, NO_FLAG = 3 };


struct EntryData {
	volatile int move;
	volatile int score;
	volatile int depth;
	volatile int flag;
};


struct TT_Entry {
	volatile uint64_t key; // The key is the posKey XORed with the data

	volatile EntryData data;
};

class TranspositionTable {
public:
	TranspositionTable(uint64_t size);

	~TranspositionTable();

	void resize(uint64_t size);

	TT_Entry* probe_tt(const uint64_t key, bool& hit);

	void store_entry(const GameState_t* pos, int move, int score, int depth, int flag);

	size_t size();
private:
	void clear_table();

	TT_Entry* volatile entries;

	int numEntries;
};


extern TranspositionTable tt;



#endif