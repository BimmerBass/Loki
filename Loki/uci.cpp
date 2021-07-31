#include "uci.h"

int UCI::num_threads = THREADS_DEFAULT_NUM;


/*

print_info is just a helper method to run when given "uci"

*/
void UCI::print_info() {
	std::cout << "id name " << EngineInfo[NAME] << " " << EngineInfo[VERSION] << std::endl;
	std::cout << "id author " << EngineInfo[AUTHOR] << std::endl;

	// Step 3C.1. Output all ajustible options for Loki.
	std::cout << "option name Hash type spin default " << TT_DEFAULT_SIZE << " min " << TT_MIN_SIZE << " max " << TT_MAX_SIZE << std::endl;
	std::cout << "option name Threads type spin default " << THREADS_DEFAULT_NUM << " min " << THREADS_MIN_NUM << " max " << THREADS_MAX_NUM << std::endl;
	std::cout << "uciok" << std::endl;
}

/*

UCI_loop is the main function for handling communication with the GUI.

*/

void UCI::loop() {

	// Step 1. Initialize an empty board and a search-driver. NOTE: This is the ony place where non-global strucures are declared with malloc in Loki.
	GameState_t* pos = new GameState_t();
	SearchInfo_t* info = new SearchInfo_t();

	// Step 1A. Set up the starting position on the board. This is done to prevent Loki from crashing if a "go" is given before a "position ..."
	pos->parseFen(START_FEN);

	// Step 2. Make sure the transposition table is the default size
	if (tt->size() != TT_DEFAULT_SIZE) {
		tt->resize(uint64_t(TT_DEFAULT_SIZE));
	}
	int mb = TT_DEFAULT_SIZE; // The set size for the transposition table.

	// Step 3. Begin listening for GUI-commands
	std::string input;
	while (std::getline(std::cin, input)) {

		// Step 3A. If a newline is given with nothing else, just wait for another instruction
		if (input[0] == '\n' || input == "") {
			continue;
		}

		// Step 3B. If we're told to start a new game, clear the transposition table and set up the starting position
		if (input.find(std::string("ucinewgame")) != std::string::npos) {
			tt->clear_table();

			pos->parseFen(START_FEN);

			continue;
		}

		// Step 3C. If we are given a "uci" command, we should output all uci parameters and info of Loki.
		else if (input.find(std::string("uci")) != std::string::npos) {

			print_info();

			continue;

		}

		// Step 3D. When the GUI sends the "isready" command, Loki needs to signify that it is ready to take commands
		else if (input.find(std::string("isready")) != std::string::npos) {
			std::cout << "readyok" << std::endl;
			continue;
		}

		// Step 3E. When we are told to change the hash size, do so.
		else if (input.find(std::string("setoption name Hash value ")) != std::string::npos) {
			// Step 3E.1. Extract the number of MB requested.
			std::stringstream strm(input);
			std::string unused[4];
			strm >> unused[0] >> unused[1] >> unused[2] >> unused[3] >> mb;


			// Step 3E.2. Make sure the size is inside of the TT-size bounds
			mb = std::min(TT_MAX_SIZE, std::max(TT_MIN_SIZE, mb));

			// Step 3E.3. Finally, resize the transposition table.
			tt->resize(uint64_t(mb));

			continue;
		}


		// Step 3F. When the GUI requests a certain number of threads for searching, set it.
		else if (input.find(std::string("setoption name Threads value ")) != std::string::npos) {
			// Step 3F.1. Extract the requested number of threads
			std::stringstream strm(input);
			std::string unused[4];
			strm >> unused[0] >> unused[1] >> unused[2] >> unused[3] >> num_threads;

			// Step 3F.2. Make sure the number of threads does not exceed the minimum/maximum number.
			num_threads = std::min(THREADS_MAX_NUM, std::max(THREADS_MIN_NUM, num_threads));

			continue;
		}

		// Step 3G. If we get the "position" command, parse it.
		else if (input.find(std::string("position")) != std::string::npos) {
			parse_position(input, pos);
			continue;
		}

		// Step 3H. If we get the "go" command, parse its parameters and begin searching
		else if (input.find(std::string("go")) != std::string::npos) {
			parse_go(input, pos, info);

			if (info->quit) {
				break;
			}

			continue;
		}

		// Step 3I. If we get told to quit, set the info->quit flag, but don't continue. Continuing would wait for a last instruction.
		else if (input.find(std::string("quit")) != std::string::npos) {
			info->quit = true;
		}

		// Below are some helper functions that can be used for debugging.
		else if (input == "d") { // Print the board state.
			pos->displayBoardState();
			continue;
		}

		else if (input.find(std::string("probetable")) != std::string::npos) { // Print the hash-entry for the position
			printHashEntry(pos);
			continue;
		}

		else if (input.find(std::string("evaltest")) != std::string::npos) { // Do an evaluation test to make sure the eval is the same for white and black.
			Eval::Debug::eval_balance();
			continue;
		}

		else if (input.find(std::string("perft")) != std::string::npos) { // Run a perft test on the current position
			goPerft(input, pos);
			continue;
		}

		// Step 3J. If we receive a "bench", run a benchmark node-count measurement
		else if (input.find("bench") != std::string::npos) {
			Bench::run_benchmark();
			continue;
		}

		// If we've been told to quit, do it.
		if (info->quit) {
			break;
		}
	}


	// Lastly, delete the board and search-driver
	delete pos;
	delete info;
}


/*

parse_position takes the GUI command as an input and sets up the position with the internal board structure of Loki

*/

void UCI::parse_position(std::string setup, GameState_t* pos) {

	// Step 1. Find out where "position " ends.
	size_t pos_end = setup.find("position") + 9;
	std::string params = setup.substr(pos_end);

	// Step 2. If we've been given the "position startpos", parse the START_FEN.
	if (params.find(std::string("startpos")) != std::string::npos) {
		pos->parseFen(START_FEN);
	}
	else { // Otherwise, load the given fen.
		size_t fen_start = params.find("fen ");

		// Step 2A. If no "fen" were found, just load the starting position.
		if (fen_start == std::string::npos) {
			pos->parseFen(START_FEN);
		}
		else {
			fen_start += 4;

			pos->parseFen(params.substr(fen_start));
		}
	}


	// Step 3. If a sequence of moves were given, make them on the board
	unsigned int move = NOMOVE;
	size_t moves_start = setup.find("moves ");

	if (moves_start != std::string::npos) {
		moves_start += 6;
		
		std::string move_string = "";

		std::istringstream ss(setup.substr(moves_start));
		
		// Step 3A. Go through all moves.
		while (std::getline(ss, move_string, ' ')) {

			// Step 3A.1. Generate the legal moves for the position.
			MoveList ml; moveGen::generate<ALL>(pos, &ml);

			move = parseMove(move_string, &ml);

			if (move == NOMOVE) { // No more moves to be made
				break;
			}

			else {
				Move_t m; m.move = move; // We need to convert the unsigned int to a Move_t in order for GameState_t::make_move to work.

				if (!pos->make_move(&m)) {
					break;
				}
				pos->ply = 0;
			}
		}
	}
}


/*

parse_go takes all search parameters from the GUI and starts up the search. FIXME: Improve time allocation here.

*/

void UCI::parse_go(std::string params, GameState_t* pos, SearchInfo_t* info) {

	// Step 1. Initialize some of the variables
	int depth = MAXDEPTH, movestogo = 30, movetime = -1;
	long long time = -1, inc = 0;
	info->timeset = false;
	info->starttime = getTimeMs();

	// Step 2. Parse the parameters given from the GUI.
	
	// Step 2A. If the infinite flag has been set, just search indefinitely
	if (params.find("infinite") != std::string::npos) {
		;
	}

	size_t index = std::string::npos;

	// Step 2B. Parse the time and increment, starting with time
	
	index = params.find((pos->side_to_move == WHITE) ? "wtime" : "btime");

	if (index != std::string::npos) {
		time = std::stoi(params.substr(index + 6));
	}

	// Step 2B.1. Parse increment the same way.
	index = params.find((pos->side_to_move == WHITE) ? "winc" : "binc");

	if (index != std::string::npos) {
		inc = std::stoi(params.substr(index + 5));
	}

	// Step 2C. Parse movestogo command --> this gives the number of moves we have left until the next time control is reached
	index = params.find("movestogo");

	if (index != std::string::npos) {
		movestogo = std::stoi(params.substr(index + 10));
	}

	// Step 2D. If were told to search for a certain amount of time, set this.
	index = params.find("movetime");

	if (index != std::string::npos) {
		movetime = std::stoi(params.substr(index + 9));
	}

	// Step 2E. If a certain depth has been given, set the max depth to that.
	index = params.find("depth");

	if (index != std::string::npos) {
		depth = std::stoi(params.substr(index + 6));
	}

	// Step 3. Configure the search time and depth depending on the parameters we've been given.
	// FIXME: Add some more elaborate time management scheme here. Right now, Loki's time management is taken from Vice...
	info->depth = depth;

	// Step 3B. If "movetime" has been given, search for this specific amount of time.
	if (movetime != -1) {
		time = movetime;
		movestogo = 1; // We divide remaining time with movestogo, so set this to 1.
	}

	// Step 3A. If we have a limited time to do the search, allocate some depending on movestogo
	if (time != -1) {
		info->timeset = true;

		// Step 3A.1. Subtract a buffer time to compensate for the time used parsing the command.
		time /= movestogo;
		int total_time = time + inc - MOVE_BUFFER;

		info->stoptime = info->starttime + total_time;

		// Step 3A.2. If we had less time than the move buffer, our endtime is before our starttime. Handle this.
		if (info->stoptime <= info->starttime) {
			info->stoptime = info->starttime + 5;
		}
	}

	// Step 4. Finally, run the search.
	
	Search::isStop = false;
	Search::runSearch(pos, info, num_threads);
}


/*

goPerft parses the depth at which perft should be run, and runs it.

*/

void UCI::goPerft(std::string l, GameState_t* pos) {
	int depth = 1;

	// Step 1. Parse the depth at which perft should be run. If none is given, set it to 1
	int index = l.find("depth ");
	if (index != std::string::npos) {
		if (l.substr(index + 6) != "") {
			depth = std::stoi(l.substr(index + 6));

		}
	}

	// Step 2. Run perft.
	Perft::perftTest(pos, depth);
}


/*

printHashEntry is a helper function for displaying the hash entry for the current position. Useful for debugging

*/

void UCI::printHashEntry(GameState_t* pos) {
	bool ttHit = false;

	EntryData_t* entry = tt->probe_tt(pos->posKey, ttHit);

	if (ttHit) {
		pos->displayBoardState();

		std::cout << "TT entry info:" << std::endl;
		std::cout << "Move:		" << printMove(entry->get_move()) << std::endl;
		std::cout << "Score:	" << entry->get_score() << std::endl;
		std::cout << "Depth:	" << entry->get_depth() << std::endl;
		std::cout << "Flag:		" << (entry->get_flag() == ttFlag::EXACT ? "EXACT" : ((entry->get_flag() == ttFlag::BETA) ? "BETA" : "ALPHA")) << std::endl;
	}
	else {
		std::cout << "Position is not stored in transposition table" << std::endl;
	}
}