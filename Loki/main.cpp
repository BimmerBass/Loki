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
	GameState_t* pos = new GameState_t;

	pos->net.load_net<LNN::CSV>("C:\\Users\\abild\\Desktop\\trained_csv.csv");

	delete pos;


	return 0;
}
