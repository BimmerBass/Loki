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

		if (!strncmp(line, "uci", 3)) { // Give the GUI information about the engine
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
			tt->clear_table();
			char newLine[] = "position startpos\n";
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
					MoveList ml; moveGen::generate<ALL>(pos, &ml);

					int move = parseMove(moveStr, &ml);

					if (move == NOMOVE) {
						break;
					}
					Move_t m;
					m.move = move;
					pos->make_move(&m);
				}
			}
		}

		else if (!strncmp(line, "undo", 4)) {
			pos->undo_move();
			continue;
		}

		else if (!strncmp(line, "printmovelist", 13)) {
			MoveList ml;
			moveGen::generate<ALL>(pos, &ml);
			printMoveList(&ml);
			continue;
		}

		else if (!strncmp(line, "go", 2)) {
			parse_go(line, pos, info);
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

	delete pos;
	delete info;
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

	delete[] cStr;
}



void UCI::parse_position(char* posLine, GameState_t* pos) {
	posLine += 9;

	char* ptrChar = posLine;

	if (!strncmp(posLine, "startpos", 8)) {
		pos->parseFen(START_FEN);
	}

	else {
		ptrChar = strstr(posLine, "fen");

		if (ptrChar == nullptr) {
			pos->parseFen(START_FEN);
		}
		else {
			ptrChar += 4;

			pos->parseFen(std::string(ptrChar));
		}
	}

	ptrChar = strstr(posLine, "moves");
	int move = NOMOVE;

	if (ptrChar != nullptr) {
		ptrChar += 6;
		
		std::string moveStr = "";
		while (*ptrChar) {
			moveStr = "";

			MoveList ml; moveGen::generate<ALL>(pos, &ml);

			for (int i = 0; i < 5; i++) {
				moveStr += *(ptrChar + i);
			}

			move = parseMove(moveStr, &ml);

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
		}
	}
}



void UCI::parse_go(char* goLine, GameState_t* pos, SearchInfo_t* info) {
	
	int depth = -1, movestogo = 30, movetime = -1;
	long long time = -1, inc = 0;
	info->timeset = false;

	char* ptr = nullptr;

	ptr = strstr(goLine, "infinite");
	if (ptr) {
		;
	}

	ptr = strstr(goLine, "binc");
	if (ptr && pos->side_to_move == BLACK) {
		inc = atoi(ptr + 5);
	}

	ptr = strstr(goLine, "winc");
	if (ptr && pos->side_to_move == WHITE) {
		inc = atoi(ptr + 5);
	}

	ptr = strstr(goLine, "wtime");
	if (ptr && pos->side_to_move == WHITE) {
		time = atoi(ptr + 6);
	}

	ptr = strstr(goLine, "btime");
	if (ptr && pos->side_to_move == BLACK) {
		time = atoi(ptr + 6);
	}

	ptr = strstr(goLine, "movestogo");
	if (ptr) {
		movestogo = atoi(ptr + 10);
	}

	ptr = strstr(goLine, "movetime");
	if (ptr) {
		movetime = atoi(ptr + 9);
	}

	ptr = strstr(goLine, "depth");
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
		std::cout << "Flag:		" << (entry->data.flag == ttFlag::EXACT ? "EXACT" : ((entry->data.flag == ttFlag::BETA) ? "BETA" : "ALPHA")) << std::endl;
	}
	else {
		std::cout << "Position is not stored in transposition table" << std::endl;
	}
}