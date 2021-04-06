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
	//tuning_variables.push_back(Parameter(&doubled_rooks, Value(0.002, 0.002), Value(6.0, 6.0)));
	//tuning_variables.push_back(Parameter(&rook_on_queen, Value(0.002, 0.002), Value(7.0, 7.0)));
	//tuning_variables.push_back(Parameter(&rook_on_kingring, Value(0.002, 0.002), Value(5.0, 5.0)));
	//tuning_variables.push_back(Parameter(&rook_open_file, Value(0.002, 0.002), Value(6.0, 6.0)));
	//tuning_variables.push_back(Parameter(&rook_semi_open_file, Value(0.002, 0.002), Value(7.0, 7.0)));
	//tuning_variables.push_back(Parameter(&rook_behind_passer, Value(0, 0.002), Value(0, 5.0)));
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 1500);

	return 0;
}
