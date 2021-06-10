#ifndef MOVESTAGER_H
#define MOVESTAGER_H
#include "movegen.h"
#include "thread.h"


enum STAGE_T:int {
	HASH_STAGE = 0,
	SCORE_CAPTURES_STAGE = 1,
	GOOD_EQUAL_CAPTURES_STAGE = 2,
	SCORE_QUIETS_STAGE = 3,
	QUIETS_STAGE = 4,
	BAD_CAPTURES_STAGE = 5,
	NO_STAGE = 6
};


class MoveStager {
public:
	// Constructor for main search
	MoveStager(SearchThread_t* ss, unsigned int ttMove);

	// Constructor for quiescence search
	MoveStager(SearchThread_t* ss);

	// Will generate all pseudo-legal moves without scoring them.
	void prepare();

	bool next_move(Move_t& move, bool skip_quiet);
private:
	MoveList legal_moves;
	size_t current_move;
	STAGE_T current_stage;

	bool is_qsearch;
};















#endif