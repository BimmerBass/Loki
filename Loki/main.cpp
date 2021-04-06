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
	//tuning_variables.push_back(Parameter(&outpost, Value(0.002, 0.002), Value(4.0, 4.0)));
	//tuning_variables.push_back(Parameter(&reachable_outpost, Value(0.002, 0.002), Value(4.0, 4.0)));
	//tuning_variables.push_back(Parameter(&defended_knight, Value(0.002, 0.002), Value(4.0, 4.0)));
	//tuning_variables.push_back(Parameter(&knight_on_kingring, Value(0.002, 0.002), Value(4.0, 4.0)));
	//
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 3000);

	return 0;
}
