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
	//tuning_variables.push_back(&pawnless_flank);
	//
	//for (int i = 0; i < 8; i++) {
	//	tuning_variables.push_back(&PSQT::king_pawn_distance_penalty[i]);
	//}
	//
	//for (int i = 0; i < 64; i++) {
	//	tuning_variables.push_back(&PSQT::pawnStorm[i]);
	//}
	//
	//for (int i = 0; i < 8; i++) {
	//	tuning_variables.push_back(&PSQT::open_kingfile_penalty[i]);
	//}
	//
	//for (int i = 0; i < 8; i++) {
	//	tuning_variables.push_back(&PSQT::semiopen_kingfile_penalty[i]);
	//}
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 3000);

	return 0;
}
