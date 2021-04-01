//#include "uci.h"
#include "texel.h"


int main() {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();


	UCI::UCI_loop();

	//Texel::Parameters tuning_variables;


	//tuning_variables.push_back(Texel::Parameter(&Eval::queen_value, Score(INF, INF), Score(0, 0)));

	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 1000);

	//Texel::Parameters tunings;
	//
	//for (int i = 0; i < PSQT::mobilityBonus[3].size(); i++){
	//	tunings.push_back(Texel::Parameter(&PSQT::mobilityBonus[3][i]));
	//}
	//
	//Texel::Tune(tunings, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 100);

	return 0;
}