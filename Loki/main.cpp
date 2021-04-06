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
	//
	//using namespace Texel;
	//
	//for (int i = 1; i < 5; i++) {
	//	tuning_variables.push_back(Parameter(&PSQT::queen_development_penalty[i], Value(0.002, 0), Value(4.0, 0)));
	//}
	//
	//tuning_variables.push_back(Parameter(&queen_on_kingring, Value(0.002, 0.002), Value(4.0, 4.0)));
	//tuning_variables.push_back(Parameter(&threatened_queen, Value(0.0025, 0.0025), Value(4.0, 4.0)));
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 2000);

	return 0;
}
