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
	//
	//for (int sq = 0; sq < 64; sq++) {
	//	tuning_variables.push_back(Texel::Parameter(&PSQT::passedPawnTable[sq], Score(200, 200), Score(0, 0)));
	//}
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 1000);

	return 0;
}
