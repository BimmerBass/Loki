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
	//tuning_variables.push_back(Parameter(&outpost, Value(0.002, 0.002), Value(6.0, 6.0)));
	//tuning_variables.push_back(Parameter(&reachable_outpost, Value(0.002, 0.002), Value(6.0, 6.0)));
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 500);

	return 0;
}
