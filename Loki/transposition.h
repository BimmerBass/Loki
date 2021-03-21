#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "position.h"

#include <vector>


// Returns the nearest power of two lower than or equal to the number maxSize
inline uint64_t shrink_to_fit(uint64_t maxSize) {
	uint64_t x = 1;
	
	while (x <= maxSize) {
		x <<= 1;
	}
	x >>= 1;

	return x;
}


// Makes a mate score relative to the position instead of the root.
inline int16_t value_to_tt(int score, int ply) {
	return (score > MATE) ? score + ply : ((score < -MATE) ? score - ply : score);
}

// Makes a mate score relative to probing root instead of the stored position
inline int16_t value_from_tt(int score, int ply) {
	return (score > MATE) ? score - ply : ((score < -MATE) ? score + ply : score);
}

enum ttFlag :int { ALPHA = 0, BETA = 1, EXACT = 2, NO_FLAG = 3 };


struct EntryData {
	volatile uint16_t move;
	volatile int16_t score;
	//volatile uint16_t unused;
	volatile uint16_t depth : 7, age : 7, flag : 2;
};


struct TT_Entry {
	//volatile uint64_t key = 0; // The key is the posKey XORed with the data

	volatile uint16_t key = 0;
	volatile EntryData data;
};

class TranspositionTable {
public:
	TranspositionTable(uint64_t size);

	~TranspositionTable();

	void resize(uint64_t size);

	TT_Entry* probe_tt(const uint64_t key, bool& hit);

	void store_entry(const GameState_t* pos, uint16_t move, int score, unsigned int depth, unsigned int flag);

	size_t size();
	void clear_table();
private:

	TT_Entry* entries = nullptr;

	uint64_t numEntries = 0;
};


extern TranspositionTable *tt;



#endif