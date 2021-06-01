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
	// learn dataset C:\\Users\\abild\\Desktop\\texel_data.lgd epoch 5 batchsize 14500 loss aae threads 8 eta 0.001 save_frequency 1 batch_load 250 output C:\\Users\\abild\\Desktop\\eval.lnn
	//DataGeneration::generate_training_data("C:\\Users\\abild\\Desktop\\texel-set-clean.epd", "C:\\Users\\abild\\Desktop\\texel_data.lgd");

	return 0;
}
