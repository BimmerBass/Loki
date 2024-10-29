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
#ifndef EVALTABLE_H
#define EVALTABLE_H
#include <cstdint>

/// <summary>
/// The default size of the evaluation hash table in bytes. It is 128KB.
/// </summary>
constexpr size_t EVAL_TABLE_SIZE = 128 << 10;



/// <summary>
/// EvalEntry_t is the container for the data in a single entry in the evaluation hash table.
/// </summary>
class EvalEntry_t {
public:
	void set(uint64_t pos_key, int eval) { key = pos_key >> 32; score = eval; }

	uint32_t get_key() const { return key; }
	int get_score() const { return score; }
private:
	uint32_t key = 0;
	int score = 0;

};



/// <summary>
/// EvaluationTable is the class responsible for managing the evaluation hash table.
/// </summary>
class EvaluationTable {
public:
	EvaluationTable();
	~EvaluationTable();

	void store(uint64_t key, int eval);
	const EvalEntry_t* probe(uint64_t key, bool& hit);

private:
	size_t num_slots = 0;
	EvalEntry_t* entries;
};




#endif