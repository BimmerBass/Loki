//#include "uci.h"
#include "texel.h"


int main() {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();


	//UCI::UCI_loop();

	//Texel::tuning_positions* p = Texel::load_epd("C:\\Users\\abild\\Desktop\\quiet-labeled.epd");
	//
	//std::cout << "Number of positions: " << p->size() << std::endl;

	Texel::Parameters tunings;

	for (int i = 0; i < PSQT::mobilityBonus[3].size(); i++){
		tunings.push_back(Texel::Parameter(&PSQT::mobilityBonus[3][i]));
	}
	
	Texel::Tune(tunings, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 500);

	return 0;
}