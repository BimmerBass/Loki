#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "tt_entry.h"

#include <vector>


/// <summary>
/// Return the nearest power of two less than or equal to some number.
/// </summary>
/// <param name="maxSize">The maximum number we can return.</param>
/// <returns></returns>
inline uint64_t nearest_power_two(uint64_t maxSize) {
	uint64_t x = 1;
	
	while (x <= maxSize) {
		x <<= 1;
	}
	x >>= 1;

	return x;
}



/// <summary>
/// The TranspositionTable class is responsible for managing the table itself and probing/storing positions from/to it.
/// </summary>
class TranspositionTable {
public:
	TranspositionTable(uint64_t size);

	~TranspositionTable();

	void resize(uint64_t size);

	EntryData_t* probe_tt(const uint64_t key, bool& hit);

	void store_entry(const GameState_t* pos, uint16_t move, int16_t score, int16_t eval, uint16_t depth,  uint16_t flag);

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
	// Each slot in the transposition table holds two entries:
	// 1st: Depth and age preferred replacement scheme.
	// 2nd: Always replace.
	struct TT_Slot {
		TT_Entry EntryOne;
		TT_Entry EntryTwo;
	};

	TT_Slot* table = nullptr;

	size_t num_slots = 0;
	size_t numEntries = 0;

	uint16_t generation = 0;
};


extern TranspositionTable *tt;



#endif