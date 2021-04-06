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
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 500);

	return 0;
}
