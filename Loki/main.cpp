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
	//tuning_variables.push_back(Texel::Parameter(&doubled_penalty, Score(INF, INF), Score(0, 0)));
	//tuning_variables.push_back(Texel::Parameter(&doubled_isolated_penalty, Score(INF, INF), Score(0, 0)));
	//tuning_variables.push_back(Texel::Parameter(&isolated_penalty, Score(INF, INF), Score(0, 0)));
	//
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 1000);

	return 0;
}
