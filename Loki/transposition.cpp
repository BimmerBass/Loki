#include "transposition.h"
#include "movegen.h"

TranspositionTable* tt = new TranspositionTable(TT_DEFAULT_SIZE);

TranspositionTable::TranspositionTable(uint64_t size) {
	numEntries = MB(size) / sizeof(TT_Entry);

	entries = new TT_Entry[numEntries];

	clear_table();

	std::cout << "Initialized " << size << "MB (" << numEntries << " entries) " << " transposition table." << std::endl;
}

TranspositionTable::~TranspositionTable() {
	delete[] entries;
}

// We shift the size by twenty to go from bytes to megabytes
size_t TranspositionTable::size() {
	return (((unsigned long long)numEntries * sizeof(TT_Entry)) >> 20) + 1;
}

void TranspositionTable::resize(uint64_t size) {
	delete[] entries;

	numEntries = MB(size) / sizeof(TT_Entry);

	entries = new TT_Entry[numEntries];

	clear_table();

	std::cout << "Resized transposition table to " << size << "MB (" << numEntries << " entries)." << std::endl;
}


void TranspositionTable::clear_table() {

	for (int i = 0; i < numEntries; i++) {
		entries[i].key = 0;
		entries[i].data.move = NOMOVE;
		entries[i].data.score = 0;
		entries[i].data.depth = 0;
		entries[i].data.flag = NO_FLAG;
	}
}

// When probing the transposition table, we won't use time validating the entry while probing. Rather this will be done in search, so we'll just return the entry
TT_Entry* TranspositionTable::probe_tt(uint64_t key, bool& hit) {
	TT_Entry* slot = &entries[key & (numEntries - 1)];
	uint64_t* data = (uint64_t*) &slot->data;

	if (slot->key == (key ^ *data)) {
		hit = true;
		return slot;
	}

	hit = false;
	return nullptr;
}


// For now we are just using a replace all strategy
void TranspositionTable::store_entry(const GameState_t* pos, int move, int score, int depth, int flag) {
	TT_Entry* slot = &entries[pos->posKey & (numEntries - 1)];
	uint64_t* data = (uint64_t*) &slot->data;

	assert(flag >= 0 && flag <= 2);

	/*
	Here we use the so-called "under-cut" replacement scheme as suggested by H.G. Muller on the talkchess forum.
	It keeps a lot of the depth-first strategy, but doesn't save irrelevant and unneccesary entries.
	*/
	if (depth >= slot->data.depth || depth == slot->data.depth - 1) {
		slot->key = pos->posKey ^ *data;
		slot->data.move = move;
		slot->data.score = score;
		slot->data.depth = depth;
		slot->data.flag = flag;
	}
}