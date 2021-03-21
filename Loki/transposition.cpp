#include "transposition.h"
#include "movegen.h"

TranspositionTable *tt = new TranspositionTable(TT_DEFAULT_SIZE);

TranspositionTable::TranspositionTable(uint64_t size) {
	uint64_t upperSize = MB(size) / sizeof(TT_Entry);
	numEntries = shrink_to_fit(upperSize); // numEntries should be a power of two.
	num_slots = numEntries / 2;
	
	table = new TT_Slot[num_slots];

	clear_table();

	std::cout << "Initialized " << size << "MB (" << numEntries << " entries) " << " transposition table." << std::endl;
}

TranspositionTable::~TranspositionTable() {
	delete[] table;
}

// We shift the size by twenty to go from bytes to megabytes
size_t TranspositionTable::size() {
	return ((numEntries * sizeof(TT_Entry)) >> 20) + 1;
}

void TranspositionTable::resize(uint64_t size) {
	delete[] table;

	uint64_t upperSize = MB(size) / sizeof(TT_Entry);
	numEntries = shrink_to_fit(upperSize);
	num_slots = numEntries / 2;

	table = new TT_Slot[num_slots];

	clear_table();

	std::cout << "Resized transposition table to " << size << "MB (" << numEntries << " entries)." << std::endl;
}


void TranspositionTable::clear_table() {

	for (int i = 0; i < num_slots; i++) {
		table[i].EntryOne.clear();
		table[i].EntryTwo.clear();
	}
}


TT_Entry* TranspositionTable::probe_tt(uint64_t key, bool& hit) {
	TT_Slot* slot = &table[key & (num_slots - 1)];

	// Check both slots for the position.
	if (slot->EntryOne.key == (key >> 48)) {
		hit = true;
		return &slot->EntryOne;
	}
	else if (slot->EntryTwo.key == (key >> 48)) {
		hit = true;
		return &slot->EntryTwo;
	}

	hit = false;
	return nullptr;

	//if (slot->key == (key >> 48)) {
	//	hit = true;
	//	return slot;
	//}
	//
	//hit = false;
	//return nullptr;
}


// For now we are just using a replace all strategy
void TranspositionTable::store_entry(const GameState_t* pos, uint16_t move, int16_t score, uint16_t depth, uint16_t flag) {
	TT_Slot* slot = &table[pos->posKey & (num_slots - 1)];

	// Firstly, check if the depth-preferred entry in the slot can be occupied.
	if (depth >= slot->EntryOne.depth || depth == slot->EntryOne.depth - 1) {
		slot->EntryOne.set(pos, move, score, depth, flag);
		return; // Don't populate the always-replace entry if this is ok.
	}

	// If we couldn't populate the depth-preferred entry, we'll insert the data in the always-replace.
	else {
		slot->EntryTwo.set(pos, move, score, depth, flag);
		return;
	}

}