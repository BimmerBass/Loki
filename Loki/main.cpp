//#include "uci.h"
//#include "texel.h"

#include "data/generate.h"

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

	//DataGeneration::generate_training_data("C:\\Users\\abild\\Desktop\\texel-set-clean.epd", "C:\\Users\\abild\\Desktop\\training_data.csv");
	DataGeneration::generate_training_data("C:\\Users\\abild\\Desktop\\quiet-labeled.epd", "C:\\Users\\abild\\Desktop\\training_data.csv");

	//GameState_t* pos = new GameState_t;
	//
	//pos->net.load_net<LNN::CSV>("C:\\Users\\abild\\Desktop\\trained_csv.csv");
	//pos->use_lnn = true;
	//
	//uint64_t avg = 0;
	//
	//for (int p = 0; p < test_positions.size(); p++) {
	//	pos->parseFen(test_positions[p]);
	//	pos->use_lnn = false;
	//	int hce = Eval::evaluate(pos);
	//	pos->use_lnn = true;
	//	int lnn = Eval::evaluate(pos);
	//
	//	std::cout << "Position " << p + 1 << ": HCE: " << hce << ", LNN: " << lnn << std::endl;
	//	avg += abs(lnn - hce);
	//}
	//
	//std::cout << "Average absolute error: " << avg / test_positions.size() << std::endl;
	//
	//delete pos;


	return 0;
}
