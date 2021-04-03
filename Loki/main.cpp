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
	//for (int i = 0; i < PSQT::mobilityBonus[3].size(); i++) {
	//	tuning_variables.push_back(Texel::Parameter(&PSQT::mobilityBonus[3][i]));
	//}
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 2000);

	return 0;
}
