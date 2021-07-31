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