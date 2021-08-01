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