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
	uint16_t key = 0;
	uint16_t move = NOMOVE;
	int16_t score = 0;
	
	uint16_t depth : 7, flag : 2, age : 7;
	
	// Clear the entry
	void clear() {
		key = 0; move = NOMOVE; score = 0; depth = 0; flag = NO_FLAG; age = 0;
	}

	// Store data to the entry
	void set(const GameState_t* pos, uint16_t _move, int16_t _score, uint16_t _depth, uint16_t _flag, uint16_t _age) {
		key = (pos->posKey >> 48); move = _move; score = value_to_tt(_score, pos->ply); depth = _depth; flag = _flag; age = _age;
	}
	
};

struct TT_Slot {
	TT_Entry EntryOne;
	TT_Entry EntryTwo;
};

class TranspositionTable {
public:
	TranspositionTable(uint64_t size);

	~TranspositionTable();

	void resize(uint64_t size);

	TT_Entry* probe_tt(const uint64_t key, bool& hit);

	void store_entry(const GameState_t* pos, uint16_t move, int16_t score, uint16_t depth,  uint16_t flag);

	size_t size();
	void clear_table();

	void setAge(int a) {
		generation = std::min(a, 127);
	}
	void increment_age() {
		generation = std::min(generation + 1, 127); // We use 7 bits to store age information in the entries, and we shouldn't store anymore since they'd overflow.

		if (generation >= 127) {
			generation /= 2; // We need to increment the generation in games that are more than 127 moves.
		}
	}
	uint16_t getAge() {
		return generation;
	}

	private:

	TT_Slot* table = nullptr;

	size_t num_slots = 0;
	size_t numEntries = 0;

	uint16_t generation = 0;
};


extern TranspositionTable *tt;



#endif