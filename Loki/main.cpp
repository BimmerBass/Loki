//#include "uci.h"
#include "texel.h"



const std::string FAIL_FEN = "r4rk1/3nb3/1p1p4/p1pPpp1p/P1P2Pp1/2BP4/1P1N2PP/3RR1K1 b -";


int main() {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();


	UCI::UCI_loop();

	//Texel::tuning_positions* p = Texel::load_epd("C:\\Users\\abild\\Desktop\\quiet-labeled.epd");
	//
	//std::cout << "Number of positions: " << p->size() << std::endl;
	//
	//
	//Texel::optimal_k(p);
	//
	//delete p;

	return 0;
}