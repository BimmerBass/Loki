#include "generate.h"


namespace DataGeneration {


	namespace Analysis {

		/*

		Generate an input array for the network

		*/
		void generate_network_input(const GameState_t* pos, std::array<int8_t, INPUT_SIZE>& input) {
			// Step 1. Set up an array to pass to LNN
			input.fill(0);

			Bitboard piece_board = 0;
			int sq = NO_SQ;

			// Step 2. Add all white pieces
			for (int pce = PAWN; pce <= KING; pce++) {
				piece_board = pos->pieceBBS[pce][WHITE];

				while (piece_board) {
					sq = PopBit(&piece_board);

					input[LNN::calculate_input_index(pce, true, sq)] = 1;
				}
			}

			// Step 3. Now do the same for the black pieces
			for (int pce = PAWN; pce <= KING; pce++) {
				piece_board = pos->pieceBBS[pce][BLACK];

				while (piece_board) {
					sq = PopBit(&piece_board);

					input[LNN::calculate_input_index(pce, false, sq)] = 1;
				}
			}
		}



		/*

		Constructor for the threadAnalyzer class.

		*/
		ThreadAnalyzer::ThreadAnalyzer(const std::vector<std::string>& _fens, int _depth, int _bound) : depth(_depth), score_bound(_bound) {

			// Step 1. Make sure all parameters are properly passed.
			try {
				if (_fens.size() < 1) { throw("A valid list of FEN's must be passed."); }
				if (_depth < 0) { throw("Depth must be zero or above"); }
				if (_bound < 1) { throw("Score bound must be a positive number."); }
			}
			catch (const char* msg) {
				std::cout << "[!] Error encountered while initializing ThreadAnalyzer object: " << msg << std::endl;
				exit(EXIT_FAILURE);
			}

			// Step 2. Copy all the fens and allocate a SearchThread_t object.
			fens = _fens;
			searcher = new SearchThread_t;
		}

		ThreadAnalyzer::~ThreadAnalyzer() {
			if (searcher != nullptr) { delete searcher; }
		}



		/*

		Search a position.

		*/
		bool ThreadAnalyzer::search(std::string fen, int& score) {
			// Step 1. Zero the score, setup the searchinfo and parse the FEN.
			score = 0;
			setup_info();
			searcher->pos->parseFen(fen);

			// Step 2. Now search the position
			// Note: If it is black to move, the score should be inversed.
			SearchPv pv;
			score = Search::search_root(searcher, depth, -INF, INF, &pv);
			if (searcher->pos->side_to_move == BLACK) { score *= -1; }

			// Step 3. Return true if the score is inside the designated bounds.
			return (score >= -score_bound && score <= score_bound);
		}


		/*

		Setup the SearchInfo_t object in the searcher object.

		*/
		void ThreadAnalyzer::setup_info() {

			// Step 1. Clear it.
			searcher->info->clear();

			// Step 2. Set the depth
			searcher->info->depth = depth;
		}


		/*

		This is the main method of the ThreadAnalyzer class. It is responsible for searching all positions.

		*/
		void ThreadAnalyzer::run() {
			// Step 1. Initialize an array of input values for the network. This will be modified after searching a position
			std::array<int8_t, INPUT_SIZE> network_input = { 0 };
			int score = 0;
			generated_entries.clear(); generated_entries.reserve(fens.size());

			// Step 2. Loop through all the fens
			for (const std::string& fen : fens) {
				// Step 2A. Search the position.
				assert(fen != "");
				bool pos_is_good = search(fen, score);

				// Step 2B. If the score is inside our desired bounds, setup a network input from the position.
				// Note: The FEN has already been loaded in the search function.
				if (pos_is_good) {
					generate_network_input(searcher->pos, network_input);

					// Step 2B.1. Now initialize a new data entry and save it to our generated entries.
					Data::DataEntry new_entry;
					memcpy(new_entry.network_input, network_input.data(), INPUT_SIZE * sizeof(int8_t));
					new_entry.score = score;

					generated_entries.push_back(new_entry);
				}
			}

			// Step 3. Shrink the generated_entries vector. This was allocated to hold all FENS but some of them might've had higher scores than what we want.
			generated_entries.shrink_to_fit();
		}















		/*

		Parse a string entered in UCI and set up a data generation session.

		The first parameter to be passed should be "type" which can be either "selfplay" or "analysis".

		Global parameters:
			- As said type.
			- output
				Output file. This will be a .lgd binary file and the ending should be in the input.
			- threads
				Amount of threads to use.
			- evalbound
				A limit on the evaluations. This will default to MATE - 1.
			- depth
				A maximum depth to search to.

		Self-play parameters:
			- positions
				Amount of positions to generate.
			- random
				Can be 1 or 0 (default = 0). Whether the games should be played with random moves (good to get unbalanced positions).
			- draw
				Can be 1 or 0 (default = 1). Whether or not to use draws.
			- firstrandom
				The amount of moves to be played randomly in the opening. A bigger value secures more game diversity (default = 4).
		Analysis parameters:
			- epd
				A path to an epd file to load.
			- batchsize
				Individual batches of the file to analyze at a time (memory overflow protection).
		*/
		void parse_uci_generate(std::string input) {
			// Step 1. Determine if we're using self-play or analysis.
			size_t index = input.find("type");

			if (index != std::string::npos) {

			}
			else {
				std::cout << "A generation type must be specified" << std::endl;
				return;
			}
		}






		/*

	Parsing c-chess-cli (by lucasart) output. This is formatted in a csv file like fen|eval|result.
		Note: At the moment, we don't use the result for anything, so it isn't parsed.

	*/

		struct cChessDataPoint {
			std::string fen;
			int eval;

			cChessDataPoint() { fen = ""; eval = 0; }
			cChessDataPoint(std::string f, int e) { fen = f; eval = e; }
			cChessDataPoint(const cChessDataPoint& ccdp) {
				fen = ccdp.fen; eval = ccdp.eval;
			}
		};

		void parse_c_chess(std::string filepath, std::string output, size_t batch_size) {
			// Step 1. Open the CSV file and set up a writer object to the output.
			std::ifstream epd_file(filepath);
			Data::DataWriter writer(output);

			// Step 2. Set up a vector for all data points and load these.
			std::vector<cChessDataPoint> data;

			std::string line = "";
			while (std::getline(epd_file, line)) {
				//std::vector<std::string> points = split_string(line, ',');
				//data.push_back(cChessDataPoint(points[0], std::stoi(points[1])));
				std::string fen = line.substr(0, line.find_first_of(","));
				int ev = std::stoi(line.substr(line.find_first_of(",") + 1, line.size()));

				data.push_back(cChessDataPoint(fen, ev));
			}

			std::cout << "Loaded " << data.size() << " positions from the data file" << std::endl;

			// Step 3. Partition the data in batches.
			// Note: The batch-size should be smaller for machines with less available memory.
			std::vector<size_t> startpoints;

			if (batch_size >= data.size()) {
				startpoints.push_back(0);
				startpoints.push_back(data.size());
			}
			else {
				int batch_count = data.size() / batch_size;
				int remainder = data.size() % batch_size;

				for (int i = 0; i < batch_count; i++) {
					startpoints.push_back(i * batch_size);
				}
				if (remainder > 0) {
					startpoints.push_back(startpoints.back() + batch_size);
				}
				assert(startpoints.back() < data.size());

				startpoints.push_back(data.size());
			}

			// Step 4. Loop through the batches and generate the positions.
			GameState_t* pos = new GameState_t;
			for (int b = 0; b < startpoints.size() - 1; b++) {

				std::vector<Data::DataEntry> batch_entries;
				std::array<int8_t, INPUT_SIZE> network_input;

				// Step 4A. Load all the FENS and convert that to network inputs.
				for (int i = startpoints[b]; i < startpoints[b + 1]; i++) {
					Data::DataEntry new_entry;

					pos->parseFen(data[i].fen);

					// If we're black in the position, reverse the sign of the eval.
					int score = data[i].eval;

					if (pos->side_to_move == BLACK) { score *= -1; }

					// Generate the network input, add this and the score to the entry and add this to the vector.
					Analysis::generate_network_input(pos, network_input);

					memcpy(new_entry.network_input, network_input.data(), INPUT_SIZE * sizeof(int8_t));
					new_entry.score = score;

					batch_entries.push_back(new_entry);
				}

				// Step 4B. Write the current batch to the output file.
				writer.save_data(batch_entries);

				std::cout << "Wrote " << startpoints[b + 1] << " data points to the output file." << std::endl;
			}

			delete pos;
		}

	}
}