#include "perft.h"


namespace Perft {
#if defined(PERFT_TT)
	PerftTable* pt = new PerftTable(32);
#else
	PerftTable* pt = nullptr;
#endif
	long long leaf_count = 0;


	namespace {

		void perft(GameState_t* pos, int depth) {
			
			if (depth <= 0) {
				std::array<neuron_t, INPUT_SIZE> net_input = net.get_input();

				std::array<int8_t, INPUT_SIZE> initial_input;
				initial_input.fill(0);

				for (int i = PAWN; i <= KING; i++) {
					Bitboard pceBoard = pos->pieceBBS[i][WHITE];

					while (pceBoard) {
						int sq = PopBit(&pceBoard);
						initial_input[LNN::calculate_input_index(i, true, sq)] = 1;
					}
				}
				for (int i = PAWN; i <= KING; i++) {
					Bitboard pceBoard = pos->pieceBBS[i][BLACK];

					while (pceBoard) {
						int sq = PopBit(&pceBoard);
						initial_input[LNN::calculate_input_index(i, false, sq)] = 1;
					}
				}

				for (int i = 0; i < INPUT_SIZE; i++) {
				
					if (initial_input[i] != static_cast<int8_t>(net_input[i])) {
						std::cout << "FAIL" << std::endl;
					}
				}

				leaf_count += 1;
				return;
			}

#if defined(PERFT_TT)
			bool tableHit = false;
			PerftEntry* entry = pt->probe_table(pos->posKey, depth, tableHit);

			if (tableHit) {
				leaf_count += entry->nodeCount;
				return;
			}
#endif

			MoveList moves; moveGen::generate<ALL>(pos, &moves);

#if defined(PERFT_TT) // Just a pedantic suppression of C4189 when not using perft_tt
			long long previous_cnt = leaf_count;
#endif

			for (int m = 0; m < moves.size(); m++) {
				net.network_updates.calculate_update(moves[m]->move,
					pos->piece_on(FROMSQ(moves[m]->move), pos->side_to_move),
					(pos->side_to_move == WHITE) ? (pos->piece_list[BLACK][TOSQ(moves[m]->move)] != NO_TYPE) : (pos->piece_list[WHITE][TOSQ(moves[m]->move)] != NO_TYPE),
					pos->piece_on(TOSQ(moves[m]->move), (pos->side_to_move == WHITE) ? BLACK : WHITE),
					(pos->side_to_move == WHITE) ? true : false
				);
				net.do_incremental();
				
				if (!pos->make_move(moves[m])) {
					continue;
				}

				perft(pos, depth - 1);

				pos->undo_move();
				net.undo_incremental();
			}

#if defined(PERFT_TT)
			pt->store_entry(pos->posKey, depth, leaf_count - previous_cnt);
#endif
		}

	}


	void perftTest(GameState_t* pos, int depth) {
		leaf_count = 0;

		std::cout << "Starting perft test to depth " << depth << std::endl;


		MoveList moves; moveGen::generate<ALL>(pos, &moves);

		std::array<int8_t, INPUT_SIZE> initial_input;
		initial_input.fill(0);

		for (int i = PAWN; i <= KING; i++) {
			Bitboard pceBoard = pos->pieceBBS[i][WHITE];

			while (pceBoard) {
				int sq = PopBit(&pceBoard);
				initial_input[LNN::calculate_input_index(i, true, sq)] = 1;
			}
		}
		for (int i = PAWN; i <= KING; i++) {
			Bitboard pceBoard = pos->pieceBBS[i][BLACK];

			while (pceBoard) {
				int sq = PopBit(&pceBoard);
				initial_input[LNN::calculate_input_index(i, false, sq)] = 1;
			}
		}

		net.load_position(initial_input);

		std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();

		int legal = 0;

		for (int m = 0; m < moves.size(); m++) {
			net.network_updates.calculate_update(moves[m]->move,
				pos->piece_on(FROMSQ(moves[m]->move), pos->side_to_move),
				(pos->side_to_move == WHITE) ? (pos->piece_list[BLACK][TOSQ(moves[m]->move)] != NO_TYPE) : (pos->piece_list[WHITE][TOSQ(moves[m]->move)] != NO_TYPE),
				pos->piece_on(TOSQ(moves[m]->move), (pos->side_to_move == WHITE) ? BLACK : WHITE),
				(pos->side_to_move == WHITE) ? true : false
			);
			net.do_incremental();

			if (!pos->make_move(moves[m])) {
				continue;
			}

			legal += 1;
			long long old_nodes = leaf_count;

			perft(pos, depth - 1);

			pos->undo_move();
			net.undo_incremental();

			std::cout << "[" << legal << "] " << printMove(moves[m]->move) << "	---> " << (leaf_count - old_nodes) << " nodes." << std::endl;
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(start_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(end_time).time_since_epoch().count();

		std::cout << "\nPerft test complete after: " << (end - start) << " milliseconds." << std::endl;

		std::cout << std::fixed << "Nodes/second: " << (double(leaf_count) / (double(end - start) / 1000.0)) << std::endl;
		std::cout << "\nNodes visited: " << leaf_count << std::endl;

	}



	PerftTable::PerftTable(size_t mb_size) {
		num_entries = int(MB(mb_size) / sizeof(PerftEntry));

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