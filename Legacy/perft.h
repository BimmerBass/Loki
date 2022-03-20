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
