//#include "uci.h"
#include "texel.h"


int main() {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();


	//UCI::UCI_loop();

	Texel::tuning_positions* p = Texel::load_epd("C:\\Users\\abild\\Desktop\\quiet-labeled.epd");
	
	std::cout << "Number of positions: " << p->size() << std::endl;
	
	
	Texel::optimal_k(p);
	
	delete p;

	return 0;
}