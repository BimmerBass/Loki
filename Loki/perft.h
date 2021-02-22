#include "movegen.h"

#include <chrono>


namespace Perft {
	extern long long leaf_count;

	
	void perftTest(GameState_t* pos, int depth);


	/*
	The below structure resembles a transposition table's, and its use is twofold: 1) It will make perft faster, and
		2) It can be helpful when debugging the zobrist position key which is also used to get an entry to the TT.
	*/


	struct PerftEntry {
		uint64_t posKey = 0;
		long long nodeCount = 0;

		int depth = 0;
	};

	class PerftTable {
	public:
		PerftTable(size_t mb_size);
		~PerftTable();

		void store_entry(uint64_t pos_key, int depth, long long nodes);

		PerftEntry* probe_table(uint64_t pos_key, int depth, bool& hit);

	private:
		int num_entries = 0;
		PerftEntry* entries = nullptr;
	};

	extern PerftTable* pt;
}
