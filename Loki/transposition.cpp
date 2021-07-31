#include "transposition.h"
#include "movegen.h"

TranspositionTable *tt = new TranspositionTable(TT_DEFAULT_SIZE);

/// <summary>
/// Default constructor for the transposition table.
/// </summary>
/// <param name="size">The size of the table in megabytes.</param>
TranspositionTable::TranspositionTable(uint64_t size) {
	uint64_t upperSize = MB(size) / sizeof(TT_Entry);
	numEntries = nearest_power_two(upperSize); // numEntries should be a power of two.
	num_slots = numEntries / 2;
	
	table = new TT_Slot[num_slots];

	clear_table();

	std::cout << "Initialized " << size << "MB (" << numEntries << " entries) " << " transposition table." << std::endl;
}

/// <summary>
/// Default destructor of the transposition table.
/// </summary>
TranspositionTable::~TranspositionTable() {
	delete[] table;
}



/// <summary>
/// Get the size of the transposition table.
/// </summary>
/// <returns>The size of the transposition table in MB.</returns>
size_t TranspositionTable::size() {
	return ((numEntries * sizeof(TT_Entry)) >> 20) + 1;
}


/// <summary>
/// Set a new size for the transposition table.
/// </summary>
/// <param name="size">The new size in MB</param>
void TranspositionTable::resize(uint64_t size) {
	delete[] table;

	uint64_t upperSize = MB(size) / sizeof(TT_Entry);
	numEntries = nearest_power_two(upperSize);
	num_slots = numEntries / 2;

	table = new TT_Slot[num_slots];

	clear_table();

	std::cout << "Resized transposition table to " << size << "MB (" << numEntries << " entries)." << std::endl;
}


/// <summary>
/// Clear all entries in the transposition table.
/// </summary>
void TranspositionTable::clear_table() {

	for (int i = 0; i < num_slots; i++) {
		table[i].EntryOne.data.clear();
		table[i].EntryTwo.data.clear();
	}

	generation = 0;
}


EntryData_t* TranspositionTable::probe_tt(uint64_t key, bool& hit) {
	TT_Slot* slot = &table[key & (num_slots - 1)];

#if defined(_WIN64) || defined(IS_64BIT) // Retrieve normally for 64-bit systems.
	if (slot->EntryOne.data.get_key() == (key >> 48)) {
		hit = true;
		return &slot->EntryOne.data;
	}
	else if (slot->EntryTwo.data.get_key() == (key >> 48)) {
		hit = true;
		return &slot->EntryTwo.data;
	}
#else // For 32-bit systems we need to make sure the entry isn't corrupted by two threads reading/writing simultaneously.
	
	// Check the first entry.
	uint64_t* data = slot->EntryOne.data.get_data();
	if (slot->EntryOne.key == (key ^ *data)) { // Entry matches.
		hit = true;
		return &slot->EntryOne.data;
	}

	// Check the second entry.
	data = slot->EntryTwo.data.get_data();
	if (slot->EntryTwo.key == (key ^ *data)) {
		hit = true;
		return &slot->EntryTwo.data;
	}
#endif

	hit = false;
	return nullptr;
}


// For now we are just using a replace all strategy
void TranspositionTable::store_entry(const GameState_t* pos, uint16_t move, int16_t score, uint16_t depth, uint16_t flag) {
	TT_Slot* slot = &table[pos->posKey & (num_slots - 1)];

	// Step 1. Check if the data can be written to the first entry.
	if (depth >= slot->EntryOne.data.get_depth() || depth == slot->EntryOne.data.get_depth() - 1 || generation > slot->EntryOne.data.get_age()) {

		// Step 1A. For 32-bit systems, get the data as a 64-bit integer.
#if !(defined(_WIN64) || defined(IS_64BIT))
		uint64_t* data = slot->EntryOne.data.get_data();
#endif

		// Step 1B. Write the new data.
		// If generation has reached its limit (127), we'll subtract a random number from that so we'll be able to still replace later on.
		slot->EntryOne.data.set(pos, move, score, depth, flag, (generation >= 127) ? generation - (pos->posKey & 1) : generation);

		// Step 1C. For 32-bit systems, we need to save the position key XOR'd with the data in order to make the tt thread safe.
#if !(defined(_WIN64) || defined(IS_64BIT))
		slot->EntryOne.key = pos->posKey ^ *data;
#endif
		
		// Return since we don't want to write the same position to both slots.
		return;
	}

	// Step 2. If we can't populate the depth-preferred slot, write to the always replace one.
	else {
		// Step 2A. For 32-bit systems, get the data as a 64-bit integer.
#if !(defined(_WIN64) || defined(IS_64BIT))
		uint64_t* data = slot->EntryTwo.data.get_data();
#endif

		// Step 2B. Write the data.
		// Note: The generation doesn't matter here, so we'll just set it to 0.
		slot->EntryTwo.data.set(pos, move, score, depth, flag, 0);

		// Step 2C. For 32-bit systems, we need to save the position key XOR'd with the data in order to make the tt thread safe.
#if !(defined(_WIN64) || defined(IS_64BIT))
		slot->EntryTwo.key = pos->posKey ^ *data;
#endif
	}
}