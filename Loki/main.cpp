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
	//
	//for (int i = 0; i < 32; i++) {
	//	tuning_variables.push_back(Texel::Parameter(&PSQT::space_bonus[i]));
	//}
	//
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 2000);

	return 0;
}
