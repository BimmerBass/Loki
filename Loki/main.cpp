//#include "uci.h"
#include "texel.h"
#include "network.h"


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

	Neural::Network myNet("");

	myNet.save_net();

	GameState_t* pos = new GameState_t;

	std::array<int16_t, Neural::INPUT_SIZE> input;
	for (int n = 0; n < test_positions.size(); n++) {
		
		pos->parseFen(test_positions[n]);

		
		for (int i = PAWN; i <= KING; i++) {

			for (int sq = 0; sq < 64; sq++) {
				input[64 * i + sq] = ((((pos->pieceBBS[i][WHITE] >> sq) & 1) == 1) ? 1 : 0);
			}

		}

		for (int i = PAWN; i <= KING; i++) {

			for (int sq = 0; sq < 64; sq++) {
				input[64 * i + sq] = ((((pos->pieceBBS[i][BLACK] >> sq) & 1) == 1) ? 1 : 0);
			}

		}

		myNet.load_position(input);

		std::cout << "Random eval for position " << n << ": " << myNet.evaluate() << std::endl;
	}

	delete pos;

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
