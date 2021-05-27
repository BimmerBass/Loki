//#include "uci.h"
#include "texel.h"
#include "lnn/trainer/train_lnn.h"


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
	//DataGeneration::generate_training_data("C:\\Users\\abild\\Desktop\\quiet-labeled.epd", "C:\\Users\\abild\\Desktop\\quiet_test.csv");
	Training::Trainer tr("C:\\Users\\abild\\Desktop\\quiet_test.csv", 1000, 1, LOSS_F::AAE, 1, 0.001);
	//Training::Trainer tr("C:\\Users\\abild\\Desktop\\quiet.csv", 1500, 14500, LOSS_F::AAE, 4, 0.1);
	tr.run();


	return 0;
}
