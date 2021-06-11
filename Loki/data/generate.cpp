#include "generate.h"


namespace DataGeneration {


	namespace Analysis {

		/*
		
		Read an EPD file and extract the FENS.
		
		*/
		std::vector<std::string> load_fens(std::string filepath) {
			// Step 1. Open the file and give an error if it can't be opened.
			std::ifstream file(filepath);

			try { if (!file.is_open()) { throw("Couldn't open the file"); } }
			catch (const char* msg) { std::cout << msg << std::endl; exit(EXIT_FAILURE); }

			// Step 2. Initialize the required variables and read the file line by line.
			std::vector<std::string> fens;
			std::string epd;
			int i = 0;
			while (std::getline(file, epd)) {
				auto fen_end = epd.find_first_of('"');

				fens.push_back(epd.substr(0, fen_end));
				i++;

				if (i % 100000 == 0) { std::cout << "Read " << i << " FENS" << std::endl; }
			}

			return fens;
		}



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
		ThreadAnalyzer::ThreadAnalyzer(int _depth, int _bound) : depth(_depth), score_bound(_bound) {

			// Step 1. Make sure all parameters are properly passed.
			try {
				if (_depth < 0) { throw("Depth must be zero or above"); }
				if (_bound < 1) { throw("Score bound must be a positive number."); }
			}
			catch (const char* msg) {
				std::cout << "[!] Error encountered while initializing ThreadAnalyzer object: " << msg << std::endl;
				exit(EXIT_FAILURE);
			}

			// Step 2. Allocate a SearchThread_t object.
			searcher = new SearchThread_t;
		}

		ThreadAnalyzer::~ThreadAnalyzer() {
			if (searcher != nullptr) { delete searcher; }
		}


		/*
		Copy-constructor. This is needed since objects will be pushed back to a vector.
		*/
		ThreadAnalyzer::ThreadAnalyzer(const ThreadAnalyzer& rhs) : depth(rhs.depth), score_bound(rhs.score_bound) {
			generated_entries = rhs.generated_entries;
			fens = rhs.fens;
			searcher = new SearchThread_t;
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
			// Note 2: search_root doesn't go into quiescence, so this should be handled for depth = 0.
			SearchPv pv;
			if (depth != 0) {
				score = Search::search_root(searcher, depth, -INF, INF, &pv);
			}
			else {
				score = Search::quiescence(searcher, -INF, INF);
			}
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

			// Step 2. Set the depth. Since we don't want to listen for GUI interaction, set the thread_id to a non-zero value.
			searcher->info->depth = depth;
			searcher->thread_id = 1;
		}


		/*

		This is the main method of the ThreadAnalyzer class. It is responsible for searching all positions.

		*/
		void ThreadAnalyzer::run(const std::vector<std::string>& _fens) {
			// Step 1. Copy the new FENS to analyze and delete the old ones.
			fens.clear();
			fens = _fens;

			assert(fens.size() > 0);

			// Step 2. Initialize an array of input values for the network. This will be modified after searching a position
			std::array<int8_t, INPUT_SIZE> network_input = { 0 };
			int score = 0;
			generated_entries.clear(); generated_entries.reserve(fens.size());

			// Step 3. Loop through all the fens
			for (const std::string& fen : fens) {
				// Step 2A. Search the position.
				assert(fen != "");
				bool pos_is_good = search(fen, score);

				// Step 3B. If the score is inside our desired bounds, setup a network input from the position.
				// Note: The FEN has already been loaded in the search function.
				if (pos_is_good) {
					generate_network_input(searcher->pos, network_input);

					// Step 3B.1. Now initialize a new data entry and save it to our generated entries.
					Data::DataEntry new_entry;
					memcpy(new_entry.network_input, network_input.data(), INPUT_SIZE * sizeof(int8_t));
					new_entry.score = score;

					generated_entries.push_back(new_entry);
				}
			}

			// Step 4. Shrink the generated_entries vector. This was allocated to hold all FENS but some of them might've had higher scores than what we want.
			generated_entries.shrink_to_fit();
		}




		/*

		Constructor for the Analyzer class.

		*/
		Analyzer::Analyzer(std::string epd_file, std::string output_file, int _depth, size_t _threads, int _bound, size_t _batch, size_t hash)
			: depth(_depth), eval_limit(_bound), thread_count(_threads), batch_size(_batch) {

			// Step 1. Make sure all parameters has been passed properly.
			try {
				if (depth < 0) { throw("Depth must be zero or above."); }
				if (eval_limit < 1) { throw("The score limit must be a positive number"); }
				if (thread_count < 1) { throw("Thread count must be a positive number"); }
				if (batch_size < 1) { throw("Batch size must be a positive number"); }
				if (epd_file == "") { throw("A path must be specified to an EPD file"); }
				if (hash < TT_MIN_SIZE || hash > TT_MAX_SIZE) { throw("Hash table size must be more than 1MB and less than 1000MB"); }
			}
			catch (const char* msg) {
				std::cout << "[!] Error encountered while initializing Analyzer object: " << msg << std::endl;
				exit(EXIT_FAILURE);
			}

			// Step 2. Parse the epd file and distribute the FENs into batches.
			std::vector<std::string> fens = load_fens(epd_file);
			
			if (batch_size >= fens.size()) {
				fen_batches.push_back(fens);
			}
			else {
				int batch_count = fens.size() / batch_size;
				int remainder = fens.size() % batch_size;
				int j = 0;
				for (int b = 0; b < batch_count; b++) {
					fen_batches.push_back(slice_vector<std::string>(fens, b * batch_size, (b + 1) * batch_size - 1));
				}
				if (remainder > 0) { fen_batches.push_back(slice_vector<std::string>(fens, fens.size() - remainder, fens.size() - 1)); }
			}

			// Step 3. Initialize the thread analyzer vector and writer object
			for (int i = 0; i < thread_count; i++) { thread_analyzers.push_back(ThreadAnalyzer(depth, eval_limit)); }
			writer = new Data::DataWriter(output_file);

			// Step 4. Resize the hash table to the desired size.
			tt->resize(hash);
		}

		Analyzer::~Analyzer() {
			if (writer != nullptr) { delete writer; }
		}


		/*
		
		Divide a batch into sub-batches for all the threads.
		
		*/
		std::vector<std::vector<std::string>> Analyzer::divide_batch(const std::vector<std::string>& batch) {
			// Step 1. Determine the batch size for each thread.
			size_t thread_batch_size = batch.size() / thread_count;
			size_t remainder = batch.size() - thread_batch_size * thread_count;

			// Step 2. Add the sub-batches to the output.
			std::vector<std::vector<std::string>> thread_batches;

			for (int i = 0; i < thread_count; i++) {
				thread_batches.push_back(slice_vector<std::string>(batch, i * thread_batch_size, (i + 1) * thread_batch_size - 1));
			}
			// Step 2A. If there is a remainder, append this to the last thread's work.
			if (remainder > 0) {
				std::vector<std::string> remaining = slice_vector<std::string>(batch, batch.size() - remainder, batch.size() - 1);

				(thread_batches.back()).insert(std::end(thread_batches.back()), std::begin(remaining), std::end(remaining));
			}

			return thread_batches;
		}


		/*
		
		Extract the results from the different threads.
		
		*/
		std::vector<Data::DataEntry> Analyzer::extract_thread_results() {
			std::vector<Data::DataEntry> entries;

			for (int i = 0; i < thread_analyzers.size(); i++) {
				entries.insert(entries.end(), std::begin(thread_analyzers[i].generated_entries), std::end(thread_analyzers[i].generated_entries));
			}

			return entries;
		}


		/*
		
		Run the analysis with all the threads.

		*/
		size_t Analyzer::run_analysis() {

			// Step 1. Initialize some variables and containers.
			std::vector<std::thread> workers;
			std::vector<std::vector<std::string>> thread_batches;
			std::vector<Data::DataEntry> entries;
			size_t accumulated_positions = 0;

			// Step 2. Loop through the batches.
			for (const std::vector<std::string>& batch : fen_batches) {
				// Step 2A. Sub-divide the current batch into work for the different threads.
				thread_batches = divide_batch(batch);

				// Step 2B. Run all the threads and wait for them to join.
				workers.clear();
				for (int t = 0; t < thread_count; t++) {
					workers.emplace_back(&ThreadAnalyzer::run, std::ref(thread_analyzers[t]), thread_batches[t]);
				}
				for (int t = 0; t < thread_count; t++) {
					workers[t].join();
				}

				// Step 2C. Extract the entries that the threads have written and write these to the output file.
				entries = extract_thread_results();
				accumulated_positions += entries.size();

				writer->save_data(entries);

				std::cout << "[+] Generated " << batch.size() << " more positions" << std::endl;
			}

			return accumulated_positions;
		}



		/*
		
		Parse an analysis data generation command.
		Note: All parameters below are case sensitive.

		Mandatory parameters:
			data: string
				- The epd file to get all FEN's from.
			output: string
				- The output file path. This should end with .lgd.
			depth: int [0;+∞]
				- The depth to analyze each position to.
			threads: int [1;+∞]
				- The amount of threads to use for simultaneous analysis.
		Optional parameters:
			limit: int [1;+∞]
				- The max/min scores to include in the output file.
				Default: mate scores.
			batchsize: int [1;+∞]
				- The batchsize to use. This helps prevent memory overflows since it only analyzes a part of the positions at a time.
				Default: 100000
			hash: int [1;1000]
				- The size of the hashtable to use in megabytes.
				Default: 16
		*/
		void parse_analyze_command(std::string cmd) {

			// Step 1. Initialize parameters.
			std::string epd = "", output = "";
			int depth = -1, threads = -1, limit = DEFAULT_EVAL_LIMIT, batch_size = DEFAULT_BATCH_SIZE, hash = TT_DEFAULT_SIZE;
			size_t index = 0;

			try {
				// Step 2. Parse the mandatory parameters and throw an error if any are missing.
				index = cmd.find("data");

				if (index != std::string::npos) {
					std::string str = cmd.substr(index + 5);

					// We need to separate this from the rest of the string, so find the next space.
					epd = str.substr(0, str.find_first_of(" "));
				}
				else {
					throw("A path to an EPD file must be specified.");
				}

				// Step 2A. The output.
				index = cmd.find("output");

				if (index != std::string::npos) {
					std::string str = cmd.substr(index + 7);

					output = str.substr(0, str.find_first_of(" "));
				}
				else {
					throw("An output file must be specified.");
				}

				// Step 2B. Depth.
				index = cmd.find("depth");

				if (index != std::string::npos) {
					depth = std::stoi(cmd.substr(index + 6));
				}
				else {
					throw("A depth must be specified.");
				}

				// Step 2C. Threads.
				index = cmd.find("threads");

				if (index != std::string::npos) {
					threads = std::stoi(cmd.substr(index + 8));
				}
				else {
					throw("A thread count must be specified");
				}

				// Step 3. Parse all optional parameters.
				index = cmd.find("limit");

				if (index != std::string::npos) {
					limit = std::stoi(cmd.substr(index + 6));
				}

				// Step 3A. Batch size.
				index = cmd.find("batchsize");

				if (index != std::string::npos) {
					batch_size = std::stoi(cmd.substr(index + 10));
				}

				// Step 3B. Hash size.
				index = cmd.find("hash");

				if (index != std::string::npos) {
					hash = std::stoi(cmd.substr(index + 5));
				}
			}
			catch (const char* msg) {
				std::cout << "Error parsing data generation command: " << msg << std::endl;
				exit(EXIT_FAILURE);
			}
			
			// Step 4. Set up an Analyzer object, print some information and run the data generation.
			Analyzer* analyzer = new Analyzer(epd, output, depth, threads, limit, batch_size, hash);

			std::cout <<
				"+-------------------------------------------------------------------+\n" <<
				"|                   Loki Analysis Data Generation                   |\n" <<
				"+----------------------+--------------------------------------------+\n" <<
				"| EPD datafile			| " << epd << "\n" <<
				"| Output filepath		| " << output << "\n" <<
				"| Analysis depth		| " << depth << "\n" <<
				"| Threads				| " << threads << "\n" <<
				"| Evaluation limit		| " << limit << "\n" <<
				"| Batch size			| " << batch_size << "\n" <<
				"| Hash size (MB)		| " << hash << "\n" <<
				"+-------------------------------------------------------------------+" << std::endl;
			
			size_t positions_generated = analyzer->run_analysis();

			std::cout <<
				"+-------------------------------------------------------------------+\n" <<
				"|               Loki Analysis Data Generation Results               |\n" <<
				"+----------------------+--------------------------------------------+\n" <<
				"| EPD datafile			| " << epd << "\n"
				"| Output filepath		| " << output << "\n"
				"| Analysis depth		| " << depth << "\n"
				"| Threads				| " << threads << "\n"
				"| Evaluation limit		| " << limit << "\n"
				"| Batch size			| " << batch_size << "\n"
				"| Hash size (MB)		| " << hash << "\n"
				"+----------------------+--------------------------------------------+\n" <<
				"| Positions generated	| " << positions_generated << "\n" <<
				"+----------------------+--------------------------------------------+" << std::endl;
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
