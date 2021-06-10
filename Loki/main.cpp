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
	//if (argc > 1 && !strncmp(argv[1], "bench", 5)) {
	//	Bench::run_benchmark();
	//	return 0;
	//}
	//
	//UCI::loop();

	
	Texel::tuning_positions* ps = new Texel::tuning_positions;
	ps = Texel::load_epd("C:\\Users\\abild\\Desktop\\quiet-labeled.epd");
	std::vector<std::string> fens;
	
	for (int i = 0; i < ps->size(); i++) {
		fens.push_back((*ps)[i].fen);
	}

	DataGeneration::Analysis::ThreadAnalyzer ta(1, 800);

	std::cout << "Loaded " << fens.size() << " fens." << std::endl;

	ta.run(fens);

	std::cout << "Generated " << ta.generated_entries.size() << " positions" << std::endl;

	// learn dataset C:\\Users\\abild\\Desktop\\loki_data.lgd epoch 3 batchsize 14500 loss aae threads 8 eta 0.001 save_frequency 1 batch_load 100 output C:\\Users\\abild\\Desktop\\loki_games.lnn net C:\\Users\\abild\\Desktop\\Loki\\Loki\\lnn\\768x256x32x32x1.lnn
	//DataGeneration::generate_training_data("C:\\Users\\abild\\Desktop\\lichess_more_data\\trainingSet.epd", "C:\\Users\\abild\\Desktop\\lichess_data.lgd");

	//DataGeneration::parse_c_chess("C:\\Users\\abild\\Desktop\\positions.csv", "C:\\Users\\abild\\Desktop\\qtest.lgd");

	return 0;
}
