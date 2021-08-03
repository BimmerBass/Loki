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
#ifndef SEARCH_H
#define SEARCH_H
#include "movegen.h"
#include "misc.h"
#include "movestager.h"

#include "transposition.h"
#include "evaluation.h"


#include <cmath>
#include <atomic>
#include <thread>
#include <vector>
#include <array>

struct SearchPv {
	int length = 0;
	std::array<int, MAXDEPTH + 1> pv = { 0 };

	void clear() {
		pv.fill(0);
		length = 0;
	}
};


namespace Search {
	extern ThreadPool_t* threads;
	extern std::vector<std::thread> threads_running;

	// isStop is a flag to signal to all the threads that the search should stop immediately.
	extern std::atomic<bool> isStop;


	void runSearch(GameState_t* pos, SearchInfo_t* info, int num_threads);

	// searchPosition is run on each thread and it is here iterative deepening will be done.
	void searchPosition(SearchThread_t* ss);
	
	// Clears the SearchThread_t before beginning a search in searchPosition.
	void clearForSearch(SearchThread_t* ss);

	int aspiration_search(SearchThread_t* ss, int depth, int estimate, SearchPv* line);

	int search_root(SearchThread_t* ss, int depth, int alpha, int beta, SearchPv* pvLine);

	int alphabeta(SearchThread_t* ss, int depth, int alpha, int beta, bool can_null, SearchPv* pvLine);

	int quiescence(SearchThread_t* ss, int alpha, int beta);

	namespace Debug {
		// MTDF is useful for debugging the transposition table as suggested by Tord Romstad on the WinBoard forum.
		int MDTF(SearchThread_t* ss, int estimate, int depth, SearchPv* pvLine);
	}

	void INIT();
}


/*

Helper function for search parameters.

*/
/*
NMP
*/
extern int NM_Reductions[MAXDEPTH][2000];
extern int nullmove_reduction(int depth, int lead);

/*
LMR
*/
extern int Reductions[MAXDEPTH][MAXPOSITIONMOVES];
extern int late_move_reduction(int d, int c);
extern void lmr_conditions(const SearchThread_t* ss, bool improving, bool capture, bool is_pv, bool gives_check, bool promotes, const Move_t& move, int& R);

/*
LMP
*/
extern int LMP_Limit[MAXDEPTH];
extern int late_move_pruning(int depth, bool improving);

/*
FP
*/
extern int futility_margin(int depth, bool improving);

/*
Razoring
*/
extern int razoring_margin(int depth, bool i);

extern bool is_passed(int fromSq, GameState_t* pos);

/*

Helper functions for search in general

*/

extern void check_stopped_search(SearchThread_t* ss);

extern void ChangePV(int move, SearchPv* parent, SearchPv* child);


extern long long getNodes();
extern long long getFailHigh();
extern long long getFailHighFirst();

extern void uci_moveinfo(int move, int depth, int index);

extern int to_cp(int score);
extern int to_mate(int score);
#endif