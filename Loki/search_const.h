#ifndef SEARCH_CONST_H
#define SEARCH_CONST_H



/*
Internal iterative deepening (IID)
*/
constexpr int iid_depth = 5;
constexpr int iid_reduction = 3;

/*
Killer moves
*/
constexpr int first_killer = 90000;
constexpr int second_killer = 80000;

/*
Counter moves
*/
constexpr int countermove_bonus = 70000;


/*
Captures
*/
// We need to make the capture scores much higher than the killers and history heuristics such that they don't get sorted as bad captures.
constexpr int capture_bonus = 1000000;

// Indexed by MvvLva[attacker][victim]
extern int MvvLva[6][6];


/*
Hash moves
*/
// The move ordering value for the hash moves has to be very much bigger than all others, since we wan't these to get searched at all costs.
constexpr int hash_move_sort = 100000000;


/*
Aspiration windows
*/
constexpr int aspiration_window = 25;
constexpr int aspiration_depth = 5;


/*
Late move reductions
*/
constexpr int lmr_limit = 4;
constexpr int lmr_depth = 2;
constexpr int lmr_pieceVals[5] = { 100, 300, 300, 500, 900 };

/*
Razoring
*/
constexpr bool use_razoring = true;
constexpr int razoring_depth = 3;


/*
Delta pruning
*/
constexpr int delta_margin = 200;

#endif