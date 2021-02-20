#include "perft.h"



namespace Perft {

	PerftTable* pt = new PerftTable(32);

	long leaf_count = 0;


	namespace {

		void perft(GameState_t* pos, int depth) {

			if (depth <= 0) {
				leaf_count += 1;
				return;
			}


			bool tableHit = false;
			PerftEntry* entry = pt->probe_table(pos->posKey, depth, tableHit);

			if (tableHit) {
				leaf_count += entry->nodeCount;
				return;
			}


			MoveList* moves = moveGen::generate<ALL>(pos);
			long long previous_cnt = leaf_count;

			for (int m = 0; m < moves->size(); m++) {
				if (!pos->make_move((*moves)[m])) {
					continue;
				}

				perft(pos, depth - 1);

				pos->undo_move();

			}

			pt->store_entry(pos->posKey, depth, leaf_count - previous_cnt);

		}

	}


	void perftTest(GameState_t* pos, int depth) {
		leaf_count = 0;

		std::cout << "Starting perft test to depth " << depth << std::endl;


		MoveList* moves = moveGen::generate<ALL>(pos);

		std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();

		int legal = 0;

		for (int m = 0; m < moves->size(); m++) {
			if (!pos->make_move((*moves)[m])) {
				continue;
			}

			legal += 1;
			long old_nodes = leaf_count;

			perft(pos, depth - 1);

			pos->undo_move();

			std::cout << "[" << legal << "] " << printMove((*moves)[m]->move) << "	---> " << (leaf_count - old_nodes) << " nodes." << std::endl;
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(start_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(end_time).time_since_epoch().count();

		std::cout << "\nPerft test complete after: " << (end - start) << " milliseconds." << std::endl;

		std::cout << "\nNodes visited: " << leaf_count << std::endl;
		std::cout << std::fixed << "Nodes/second: " << (double(leaf_count) / (double(end - start) / 1000.0)) << std::endl;

	}



	PerftTable::PerftTable(size_t mb_size) {
		num_entries = (MB(mb_size) / sizeof(PerftEntry));

		entries = new PerftEntry[num_entries];
	}

	PerftTable::~PerftTable() {
		delete entries;
	}

	void PerftTable::store_entry(uint64_t pos_key, int depth, long long nodes) {
		PerftEntry* slot = &entries[pos_key % num_entries];

		slot->posKey = pos_key;
		slot->depth = depth;
		slot->nodeCount = nodes;
	}

	PerftEntry* PerftTable::probe_table(uint64_t pos_key, int depth, bool& hit) {
		PerftEntry* slot = &entries[pos_key % num_entries];

		if (slot->posKey == pos_key && slot->depth == depth) {
			hit = true;
			return slot;
		}

		hit = false;
		return nullptr;
	}

}