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
	//
	//for (int i = 0; i < 100; i++) {
	//	tuning_variables.push_back(&PSQT::safety_table[i]);
	//}
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 3000);

	return 0;
}
