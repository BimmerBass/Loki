//#include "uci.h"
#include "texel.h"
#include "lnn/trainer/train_lnn.h"
#include "data/generate.h"

int main(int argc, char* argv[]) {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();

	// If "bench" has been added as an argument, just run this and quit
	if (argc > 1 && !strncmp(argv[1], "bench", 5)) {
		Bench::run_benchmark();
		return 0;
	}
	
	UCI::loop();
	
	//DataGeneration::generate_training_data("C:\\Users\\abild\\Desktop\\texel-set-clean.epd", "C:\\Users\\abild\\Desktop\\texel_data.lgd");

	return 0;
}
