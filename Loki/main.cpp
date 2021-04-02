#include "uci.h"
//#include "texel.h"


int main() {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();


	UCI::UCI_loop();

	//Texel::Parameters tuning_variables;
	
	//for (int sq = 0; sq < 64; sq++) {
	//	tuning_variables.push_back(Texel::Parameter(&PSQT::KingTable[sq], Score(100, 100), Score(-100, -100)));
	//}
	//
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 1500);

	return 0;
}
