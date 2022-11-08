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
#ifndef TT_ENTRY
#define TT_ENTRY
#include "position.h"


/// <summary>
/// Make the score of a position relative to the root in case of mate scores.
/// </summary>
/// <param name="score">The score form a shallow search.</param>
/// <param name="ply">The ply-depth from root.</param>
/// <returns></returns>
inline int16_t value_to_tt(int score, int ply) {
	return (score > MATE) ? score + ply : ((score < -MATE) ? score - ply : score);
}

/// <summary>
/// Make the score of a position relative to its depth instead of relative to root.
/// </summary>
/// <param name="score">The score form a shallow search.</param>
/// <param name="ply">The ply-depth from root.</param>
/// <returns></returns>
inline int16_t value_from_tt(int score, int ply) {
	return (score > MATE) ? score - ply : ((score < -MATE) ? score + ply : score);
}



/// <summary>
/// The different kinds of flags an entry can have.
/// </summary>
enum ttFlag :int { NO_FLAG = -1, ALPHA = 0, BETA = 1, EXACT = 2 };




/// <summary>
/// EntryData_t holds all the data inside a transposition table entry.
/// </summary>
class EntryData_t {
public:
	void set(const GameState_t* pos, uint16_t _move, int16_t _score, uint16_t _depth, uint16_t _flag, uint16_t _age);
	void clear();

	// Data retrieval getter methods.
	uint16_t get_key() const { return data.key; }
	uint16_t get_move() const { return data.move; }
	int16_t get_score() const { return data.score; }
	int8_t get_depth() const { return data.depth; }
	int8_t get_flag() const { return data.flag; }
	int8_t get_age() const { return data.age; }

	// Function for expressing the data as a unsigned 64-bit integer. Used for 32-bit multithreading.
	uint64_t* get_data() const { return (uint64_t*)&data; }
private:
	// The data is made to fit into a single 64 bit register.
	// Note: This means that it has to be handled differently on 32-bit systems when using multiple threads.
	struct data_t {
		uint16_t key;
		uint16_t move;
		int16_t score;
		int16_t depth : 7, flag : 2, age : 7;
	};
	data_t data;
};



/// <summary>
/// TT_Entry is the structure for a single entry inside the TT bucket. This is responsible for handling data differently between 32-bit and 64-bit systems.
/// </summary>
struct TT_Entry {
#if defined(_WIN64) || defined(IS_64BIT)
	EntryData_t data;
#else
	// For 32-bit systems we need to hold an extra 64-bit key in order to make multithreading possible.
	uint64_t key = 0;
	EntryData_t data;
#endif
};









#endif