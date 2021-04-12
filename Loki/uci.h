#ifndef UCI_H
#define UCI_H
#include "search.h"
#include "perft.h"

#include <map>

// This is just a neat way of storing the info
enum InfoParameters :int { NAME = 0, VERSION = 1, AUTHOR = 2 };

// Current rating measured against MadChess2.2 and Raven 1.10: 2510 (5s + 0.1s with 300 games)
static std::map<InfoParameters, std::string> EngineInfo {
	{NAME, "Loki"},
	{ VERSION, "3.0.0" },
	{ AUTHOR, "Niels Abildskov" }
};


namespace UCI {

	void UCI_loop();

	void parse_position(char* posLine, GameState_t* pos);

	void parse_go(char* goLine, GameState_t* pos, SearchInfo_t* info);

	void goPerft(std::string l, GameState_t* pos);

	// Helper function for debugging the transposition table. It prints info about the position stored in the tt.
	void printHashEntry(GameState_t* pos);
}









#endif