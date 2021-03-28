#include "pawnhash.h"




PTable::PTable(size_t size_kb) {
	uint64_t upperSize = KB(size_kb) / sizeof(PH_Entry);
	num_entries = nearest_power_two(upperSize); // Make num_entries a power of two.

	entries = new PH_Entry[num_entries];

	clear();
}


PTable::~PTable() {
	delete[] entries;
}


void PTable::clear() {

	for (int i = 0; i < num_entries; i++) {
		entries[i].key = 0;
		entries[i].mg_value = 0; entries[i].eg_value = 0;

		entries[i].passers[0] = 0; entries[i].passers[1] = 0;
		entries[i].attacks[0] = 0; entries[i].attacks[1] = 0;
	}
}


PH_Entry* PTable::probe(Bitboard pawnKey, bool& hit) {
	PH_Entry* slot = &entries[pawnKey & (num_entries - 1)];

	if (slot->key == pawnKey) {
		hit = true;
		return slot;
	}

	hit = false;
	return nullptr;
}


void PTable::store_entry(Bitboard key, int mg_score, int eg_score, Bitboard* passed_pawns, Bitboard* pawn_attacks) {
	PH_Entry* slot = &entries[key & (num_entries - 1)];

	slot->key = key;
	slot->mg_value = mg_score; slot->eg_value = eg_score;

	slot->passers[BLACK] = passed_pawns[BLACK]; slot->passers[WHITE] = passed_pawns[WHITE];

	slot->attacks[BLACK] = pawn_attacks[BLACK]; slot->attacks[WHITE] = pawn_attacks[WHITE];
}