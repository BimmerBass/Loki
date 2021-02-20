#ifndef UCI_H
#define UCI_H
#include "search.h"
#include "perft.h"

#include <map>

// This is just a neat way of storing the info
enum InfoParameters :int { NAME = 0, VERSION = 1, AUTHOR = 2 };

// Current rating measured against TSCP (1725 elo) (1 min + 0 s, 40 move tc): 1773 elo
static std::map<InfoParameters, std::string> EngineInfo {
	{NAME, "Loki"},
	{ VERSION, "1.0" },
	{ AUTHOR, "Niels Abildskov" }
};


namespace UCI {

	void UCI_loop();

	void parse_position(std::string posLine, GameState_t* pos);

	void parse_go(std::string goLine, GameState_t* pos, SearchInfo_t* info);

	void goPerft(std::string l, GameState_t* pos);

	// Helper function for debugging the transposition table. It prints info about the position stored in the tt.
	void printHashEntry(GameState_t* pos);
}









#endif