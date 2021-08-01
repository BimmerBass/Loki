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
#ifndef SEARCH_CONST_H
#define SEARCH_CONST_H



/*
Internal iterative deepening (IID)
*/
constexpr int iid_depth = 6;
constexpr int iid_reduction = 4;

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
constexpr int aspiration_window = 50;
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
constexpr int razoring_depth = 2;


/*
Delta pruning
*/
constexpr int delta_margin = 200;

#endif