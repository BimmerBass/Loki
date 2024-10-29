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
#include "tt_entry.h"



/// <summary>
/// Populate an entry with new data.
/// </summary>
/// <param name="pos">The position's ojbect. Used to get the hash key</param>
/// <param name="_move">The best move from search.</param>
/// <param name="_score">The score of the position.</param>
/// <param name="_depth">The depth the position has been searched to.</param>
/// <param name="_flag">The type of entry (PV/upper/lower bound).</param>
/// <param name="_age">The current age of the transposition table.</param>
void EntryData_t::set(const GameState_t* pos, uint16_t _move, int16_t _score, uint16_t _depth, uint16_t _flag, uint16_t _age) {
	// Use the upper 16-bits of the position key as the entry's key.
	data.key = (pos->posKey >> 48);
	data.move = _move;
	data.score = _score;
	data.depth = _depth;
	data.flag = _flag;
	data.age = _age;
}


/// <summary>
/// Clear the entry.
/// </summary>
void EntryData_t::clear() {
	data.key = 0;
	data.move = NOMOVE;
	data.score = 0;
	data.depth = 0;
	data.flag = NO_FLAG;
	data.age = 0;
}