#ifndef UCI_H
#define UCI_H
#include "search.h"
#include "perft.h"

#include <map>
#include <sstream>

// This is just a neat way of storing the info
enum InfoParameters :int { NAME = 0, VERSION = 1, AUTHOR = 2 };

// Current rating measured against MadChess2.2 and Raven 1.10: 2510 (5s + 0.1s with 300 games)
static std::map<InfoParameters, std::string> EngineInfo {
	{NAME, "Loki"},
	{ VERSION, "3.0.0" },
	{ AUTHOR, "Niels Abildskov" }
};


namespace UCI {
	extern int num_threads; // Global such that it can be accessed by both loop() and parse_go();

	void loop();

	void parse_position(std::string setup, GameState_t* pos);

	void parse_go(std::string params, GameState_t* pos, SearchInfo_t* info);

	void goPerft(std::string l, GameState_t* pos);

	// Helper function for debugging the transposition table. It prints info about the position stored in the tt.
	void printHashEntry(GameState_t* pos);
}









#endif