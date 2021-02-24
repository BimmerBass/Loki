#include "uci.h"

#define INPUTBUFFER 400 * 6

static int threadNum = THREADS_DEFAULT_NUM;

void UCI::UCI_loop() {
	
	setvbuf(stdout, NULL, _IOLBF, sizeof(NULL));
	setvbuf(stdin, NULL, _IOLBF, sizeof(NULL));
	
	// Initialize board, searchinfo and make sure the tt size is correct.
	//GameState_t* pos = new GameState_t();
	//SearchInfo_t* info = new SearchInfo_t();
	GameState_t* pos = new GameState_t();
	SearchInfo_t* info = new SearchInfo_t();


	if (tt->size() != TT_DEFAULT_SIZE) {
		tt->resize(uint64_t(TT_DEFAULT_SIZE));
	}


	// Give the GUI information about the engine
	std::cout << "id name " << EngineInfo[NAME] << " " << EngineInfo[VERSION] << std::endl;
	std::cout << "id author " << EngineInfo[AUTHOR] << std::endl;

	// Option for changing transposition table size
	std::cout << "option name Hash type spin default " << TT_DEFAULT_SIZE << " min " << TT_MIN_SIZE << " max " << TT_MAX_SIZE << std::endl;

	// Option to change number of threads
	std::cout << "option name Threads type spin default " << THREADS_DEFAULT_NUM << " min " << THREADS_MIN_NUM << " max " << THREADS_MAX_NUM << std::endl;

	// After all "id" and "option" commands has been sent, let's tell the GUI that were ready.
	std::cout << "uciok" << std::endl;

	
	char line[INPUTBUFFER] = { 0 };
	int mb = TT_DEFAULT_SIZE;

	while (true) {
		// Get GUI input
		memset(&line[0], 0, sizeof(line));
		fflush(stdout);
		if (!fgets(line, INPUTBUFFER, stdin)) {
			continue;
		}

		if (line[0] == '\n') {
			continue;
		}

		if (!strncmp(line, "uci", 3)) {
			std::cout << "id name " << EngineInfo[NAME] << " " << EngineInfo[VERSION] << std::endl;
			std::cout << "id author " << EngineInfo[AUTHOR] << std::endl;

			std::cout << "option name Hash type spin default " << TT_DEFAULT_SIZE << " min " << TT_MIN_SIZE << " max " << TT_MAX_SIZE << std::endl;
			std::cout << "option name Threads type spin default " << THREADS_DEFAULT_NUM << " min " << THREADS_MIN_NUM << " max " << THREADS_MAX_NUM << std::endl;
			std::cout << "uciok" << std::endl;
			continue;
		}

		else if (!strncmp(line, "isready", 7)) {
			std::cout << "readyok" << std::endl;
		}


		else if (!strncmp(line, "setoption name Hash value ", 26)) {
#if (defined(_WIN32) || defined(_WIN64))
			sscanf_s(line, "%*s %*s %*s %*s %d", &mb);
#else
			sscanf(line, "%*s %*s %*s %*s %d", &mb);
#endif

			// Here we are just making sure that the set size doesn't exceed the set limits
			if (mb < TT_MIN_SIZE) {
				mb = TT_MIN_SIZE;
			}
			else if (mb > TT_MAX_SIZE) {
				mb = TT_MAX_SIZE;
			}

			tt->resize(uint64_t(mb));
			continue;
		}


		else if (!strncmp(line, "setoption name Threads value ", 29)) {
#if (defined(_WIN32) || defined(_WIN64))
			sscanf_s(line, "%*s %*s %*s %*s %d", &threadNum);
#else
			sscanf(line, "%*s %*s %*s %*s %d", &threadNum);
#endif
			if (threadNum < THREADS_MIN_NUM) {
				threadNum = THREADS_MIN_NUM;
			}
			else if (threadNum > THREADS_MAX_NUM) {
				threadNum = THREADS_MAX_NUM;
			}
			std::cout << "Now using " << threadNum << " threads" << std::endl;
		}


		else if (!strncmp(line, "ucinewgame", 10)) {
			std::string newLine = "position startpos\n";
			parse_position(newLine, pos);
			continue;
		}

		else if (!strncmp(line, "position", 8)) {
			parse_position(line, pos);
			continue;
		}

		else if (!strncmp(line, "probetable", 10)) {
			printHashEntry(pos);
			continue;
		}

		else if (!strncmp(line, "evaltest", 8)) {
			Eval::Debug::eval_balance();
			continue;
		}

		else if (!strncmp(line, "do", 2)) {
			char* ptr = line;

			if (ptr != nullptr) {

				while (*ptr) {
					ptr += 3;
					std::string moveStr = "";
					for (int i = 0; i < 5; i++) {
						moveStr += *(ptr + i);
					}
					MoveList* ml = moveGen::generate<ALL>(pos);

					int move = parseMove(moveStr, ml);

					if (move == NOMOVE) {
						break;
					}
					Move_t m;
					m.move = move;
					pos->make_move(&m);

					delete ml;
				}
			}
		}

		else if (!strncmp(line, "undo", 4)) {
			pos->undo_move();
			continue;
		}

		else if (!strncmp(line, "go", 2)) {
			parse_go(std::string(line), pos, info);
		}

		else if (!strncmp(line, "d", 1)) {
			pos->displayBoardState();
			continue;
		}

		else if (!strncmp(line, "perft", 5)) {
			goPerft(std::string(line), pos);
			continue;
		}

		else if (!strncmp(line, "quit", 4)) {
			info->quit = true;
			break;
		}

		else {
			std::cout << "Unknown command: " << line << std::endl;
			continue;
		}

		// If we've been told to quit in the search, we should do it.
		if (info->quit == true) {
			break;
		}
	}

	//delete pos;
	//delete info;
}



void UCI::goPerft(std::string l, GameState_t* pos) {
	char* cStr = new char[l.length() + 1];
#if defined(_WIN32) || defined(_WIN64)
	strcpy_s(cStr, l.length() + 1, l.c_str());
#else
	strcpy(cStr, l.c_str());
#endif

	char* ptr = nullptr;
	ptr = strstr(cStr, "depth");
	if (ptr != 0) {
		int depth = atoi(ptr + 6);

		Perft::perftTest(pos, depth);
	}
}



void UCI::parse_position(std::string posLine, GameState_t* pos) {
	// We're converting std::string to char* because it is easier to parse.
	char* cStr = new char[posLine.length() + 1];
#if defined(_WIN32) || defined(_WIN64)
	strcpy_s(cStr, posLine.length() + 1, posLine.c_str());
#else
	strcpy(cStr, posLine.c_str());
#endif


	char* ptrChar = cStr;
	ptrChar += 9;

	if (!strncmp(cStr, "startpos", 8)) {
		pos->parseFen(START_FEN);
	}

	else {
		ptrChar = strstr(cStr, "fen");

		if (ptrChar == nullptr) {
			pos->parseFen(START_FEN);
		}
		else {
			ptrChar += 4;

			pos->parseFen(std::string(ptrChar));
		}
	}

	ptrChar = strstr(cStr, "moves");
	int move = NOMOVE;

	if (ptrChar != nullptr) {
		ptrChar += 6;
		
		std::string moveStr = "";
		while (*ptrChar) {
			moveStr = "";

			MoveList* ml = moveGen::generate<ALL>(pos);

			for (int i = 0; i < 5; i++) {
				moveStr += *(ptrChar + i);
			}

			move = parseMove(moveStr, ml);

			if (move == NOMOVE) {
				break;
			}
			Move_t m;
			m.move = move;
			if (!pos->make_move(&m)) {
				break;
			}
			pos->ply = 0;

			while (*ptrChar && *ptrChar != ' ') {
				ptrChar++;
			}
			ptrChar++;

			delete ml;
		}
	}


	delete[] cStr;
}



void UCI::parse_go(std::string goLine, GameState_t* pos, SearchInfo_t* info) {
	// Again, we convert goLine to a char* for easier parsing. FIXME: do the parsing with a std::string
	char* go_line = new char[goLine.length() + 1];
#if defined(_WIN32) || defined(_WIN64)
	strcpy_s(go_line, goLine.length() + 1, goLine.c_str());
#else
	strcpy(go_line, goLine.c_str());
#endif

	int depth = -1, movestogo = 30, movetime = -1;
	long long time = -1, inc = 0;
	info->timeset = false;

	char* ptr = nullptr;

	ptr = strstr(go_line, "infinite");
	if (ptr) {
		;
	}

	ptr = strstr(go_line, "binc");
	if (ptr && pos->side_to_move == BLACK) {
		inc = atoi(ptr + 5);
	}

	ptr = strstr(go_line, "winc");
	if (ptr && pos->side_to_move == WHITE) {
		inc = atoi(ptr + 5);
	}

	ptr = strstr(go_line, "wtime");
	if (ptr && pos->side_to_move == WHITE) {
		time = atoi(ptr + 6);
	}

	ptr = strstr(go_line, "btime");
	if (ptr && pos->side_to_move == BLACK) {
		time = atoi(ptr + 6);
	}

	ptr = strstr(go_line, "movestogo");
	if (ptr) {
		movestogo = atoi(ptr + 10);
	}

	ptr = strstr(go_line, "movetime");
	if (ptr) {
		movetime = atoi(ptr + 9);
	}

	ptr = strstr(go_line, "depth");
	if (ptr) {
		depth = atoi(ptr + 6);
	}

	if (movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	info->starttime = getTimeMs();
	info->depth = depth;

	if (time != -1) {
		info->timeset = true;
		time /= movestogo;

		if (time > 50) {
			time -= 50;
		}
		else {
			time = time;
		}

		info->stoptime = info->starttime + time + inc;
	}

	if (depth == -1) {
		info->depth = MAXDEPTH;
	}

	std::cout << "time: " << time << " start: " << info->starttime << " stop: "
		<< info->stoptime << " depth: " << info->depth << " timeset: " << info->timeset << "\n";

	Search::runSearch(pos, info, threadNum);
}



void UCI::printHashEntry(GameState_t* pos) {
	bool ttHit = false;

	volatile TT_Entry* entry = tt->probe_tt(pos->posKey, ttHit);

	if (ttHit) {
		pos->displayBoardState();

		std::cout << "TT entry info:" << std::endl;
		std::cout << "Move:		" << printMove(entry->data.move) << std::endl;
		std::cout << "Score:	" << entry->data.score << std::endl;
		std::cout << "Depth:	" << entry->data.depth << std::endl;
		std::cout << "Flag:		" << (entry->data.flag == EXACT ? "EXACT" : ((entry->data.flag == BETA) ? "BETA" : "ALPHA")) << std::endl;
	}
	else {
		std::cout << "Position is not stored in transposition table" << std::endl;
	}
}