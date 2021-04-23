#include "uci.h"

int UCI::num_threads = THREADS_DEFAULT_NUM;

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
	while (true) {

		std::string input;
		std::getline(std::cin, input);

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

			std::cout << "id name " << EngineInfo[NAME] << " " << EngineInfo[VERSION] << std::endl;
			std::cout << "id author" << EngineInfo[AUTHOR] << std::endl;

			// Step 3C.1. Output all ajustible options for Loki.
			std::cout << "option name Hash type spin default " << TT_DEFAULT_SIZE << " min " << TT_MIN_SIZE << " max " << TT_MAX_SIZE << std::endl;
			std::cout << "option name Threads type spin default " << THREADS_DEFAULT_NUM << " min " << THREADS_MIN_NUM << " max " << THREADS_MAX_NUM << std::endl;
			std::cout << "uciok" << std::endl;

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

parse_position takes the GUI command as an input and sets up the position with the internal board structure of Loki.

*/

void UCI::parse_position(std::string setup, GameState_t* pos) {

}


/*

parse_go takes all search parameters from the GUI and starts up the search.

*/

void UCI::parse_go(std::string params, GameState_t* pos, SearchInfo_t* info) {


}


/*

goPerft parses the depth at which perft should be run, and runs it.

*/

void UCI::goPerft(std::string l, GameState_t* pos) {

}


/*

printHashEntry is a helper function for displaying the hash entry for the current position. Useful for debugging

*/

void UCI::printHashEntry(GameState_t* pos) {
	bool ttHit = false;

	volatile TT_Entry* entry = tt->probe_tt(pos->posKey, ttHit);

	if (ttHit) {
		pos->displayBoardState();

		std::cout << "TT entry info:" << std::endl;
		std::cout << "Move:		" << printMove(entry->move) << std::endl;
		std::cout << "Score:	" << entry->score << std::endl;
		std::cout << "Depth:	" << entry->depth << std::endl;
		std::cout << "Flag:		" << (entry->flag == ttFlag::EXACT ? "EXACT" : ((entry->flag == ttFlag::BETA) ? "BETA" : "ALPHA")) << std::endl;
	}
	else {
		std::cout << "Position is not stored in transposition table" << std::endl;
	}
}