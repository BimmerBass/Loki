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
#include "evaltable.h"



/// <summary>
/// Default constructor for the evaluation table. It allocates memory for the entries.
/// </summary>
EvaluationTable::EvaluationTable() {
	num_slots = EVAL_TABLE_SIZE / sizeof(EvalEntry_t);
	entries = new EvalEntry_t[num_slots];
}

/// <summary>
/// Destructor. Frees the memory allocated during construction.
/// </summary>
EvaluationTable::~EvaluationTable() {
	if (entries != nullptr) { delete[] entries; }
}


/// <summary>
/// Store an entry in the table.
/// </summary>
/// <param name="key">The position's zobrist hash key.</param>
/// <param name="eval">The static evaluation of the position.</param>
void EvaluationTable::store(uint64_t key, int eval) {
	EvalEntry_t* entry = &entries[key & (num_slots - 1)];

	entry->set(key, eval);
}


/// <summary>
/// Probe the table to see if a pre-calculated evaluation exists for the position.
/// </summary>
/// <param name="key">The position's zobrist hash key.</param>
/// <param name="hit">A reference to a flag signalling if the got a hit.</param>
/// <returns>In case of a hit: A pointer to the entry. Otherwise, a null pointer.</returns>
const EvalEntry_t* EvaluationTable::probe(uint64_t key, bool& hit) {
	const EvalEntry_t* entry = &entries[key & (num_slots - 1)];

	// Step 1. If the keys match, we have a hit.
	if (entry->get_key() == (key >> 32)) {
		hit = true;
		return entry;
	}

	// Step 2. Otherwise, return a null pointer.
	hit = false;
	return nullptr;
}