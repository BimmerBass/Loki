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
inline int value_to_tt(int score, int ply) {
	return (score > MATE) ? score + ply : ((score < -MATE) ? score - ply : score);
}

// Makes a mate score relative to probing root instead of the stored position
inline int value_from_tt(int score, int ply) {
	return (score > MATE) ? score - ply : ((score < -MATE) ? score + ply : score);
}

enum class ttFlag :int { ALPHA = 0, BETA = 1, EXACT = 2, NO_FLAG = 3 };


struct EntryData {
	volatile unsigned int move = NOMOVE;
	volatile int score = -INF;
	volatile unsigned int depth = 0;
	volatile ttFlag flag = ttFlag::NO_FLAG;
};


struct TT_Entry {
	volatile uint64_t key = 0; // The key is the posKey XORed with the data

	volatile EntryData data;
};

class TranspositionTable {
public:
	TranspositionTable(uint64_t size);

	~TranspositionTable();

	void resize(uint64_t size);

	TT_Entry* probe_tt(const uint64_t key, bool& hit);

	void store_entry(const GameState_t* pos, int move, int score, unsigned int depth, ttFlag flag);

	size_t size();
	void clear_table();
private:

	TT_Entry* entries = nullptr;

	uint64_t numEntries = 0;
};


extern TranspositionTable *tt;



#endif