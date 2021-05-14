//#include "uci.h"
#include "texel.h"


int main(int argc, char* argv[]) {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();


	// If "bench" has been added as an argument, just run this and quit
	//if (argc > 1 && !strncmp(argv[1], "bench", 5)) {
	//	Bench::run_benchmark();
	//	return 0;
	//}
	//
	//UCI::loop();

	Texel::Parameters tuning_params;

	using namespace Texel;

	for (int i = 0; i < 64; i++) {
		tuning_params.push_back(Parameter(&PSQT::PawnTable[i], Value(0.002, 0.002), Value(3.0, 3.0), Score(200, 200), Score(-200, -200)));
	}
	for (int i = 0; i < 64; i++) {
		tuning_params.push_back(Parameter(&PSQT::KnightTable[i], Value(0.002, 0.002), Value(3.0, 3.0), Score(200, 200), Score(-200, -200)));
	}
	for (int i = 0; i < 64; i++) {
		tuning_params.push_back(Parameter(&PSQT::BishopTable[i], Value(0.002, 0.002), Value(3.0, 3.0), Score(200, 200), Score(-200, -200)));
	}
	for (int i = 0; i < 64; i++) {
		tuning_params.push_back(Parameter(&PSQT::RookTable[i], Value(0.002, 0.002), Value(3.0, 3.0), Score(200, 200), Score(-200, -200)));
	}
	for (int i = 0; i < 64; i++) {
		tuning_params.push_back(Parameter(&PSQT::QueenTable[i], Value(0.002, 0.002), Value(3.0, 3.0), Score(200, 200), Score(-200, -200)));
	}
	for (int i = 0; i < 64; i++) {
		tuning_params.push_back(Parameter(&PSQT::KingTable[i], Value(0.002, 0.002), Value(3.0, 3.0), Score(200, 200), Score(-200, -200)));
	}

	Texel::Tune(tuning_params, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 10000);
	//Texel::Parameters tuning_variables;
	//
	//using namespace Texel;
	//
	//tuning_variables.push_back(Parameter(&bishop_pair, Value(0.0015, 0.0015), Value(3.0, 3.0)));
	//tuning_variables.push_back(Parameter(&knight_pawn_penaly, Value(0.001, 0.001), Value(2.0, 2.0), Score(INF, INF), Score(1, 1)));
	//tuning_variables.push_back(Parameter(&rook_pawn_bonus, Value(0.001, 0.001), Value(2.0, 2.0), Score(INF, INF), Score(1, 1)));
	//
	//
	//Texel::Tune(tuning_variables, "C:\\Users\\abild\\Desktop\\quiet-labeled.epd", 1000);

	return 0;
}
