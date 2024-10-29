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
#ifndef UCI_H
#define UCI_H
#include "search.h"
#include "perft.h"
#include "bench.h"

#include <map>
#include <sstream>

// This is just a neat way of storing the info
enum InfoParameters :int { NAME = 0, VERSION = 1, AUTHOR = 2 };

static std::map<InfoParameters, std::string> EngineInfo {
	{NAME, "Loki"},
	{ VERSION, "3.5.0" },
	{ AUTHOR, "Niels Abildskov" }
};


namespace UCI {
	constexpr int MOVE_BUFFER = 50;

	extern int num_threads; // Global such that it can be accessed by both loop() and parse_go();

	// Main method of the UCI implementation. Responsible for listening for all input
	void loop();

	void print_info();

	// Method for parsing the "position" command
	void parse_position(std::string setup, GameState_t* pos);

	// Method for parsing the "go" command
	void parse_go(std::string params, GameState_t* pos, SearchInfo_t* info);

	// Method for running perft
	void goPerft(std::string l, GameState_t* pos);

	// Helper function for debugging the transposition table. It prints info about the position stored in the tt.
	void printHashEntry(GameState_t* pos);
}









#endif