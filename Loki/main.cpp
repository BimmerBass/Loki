#include "uci.h"

const std::string FEN = "b2b1r1k/3R1ppp/4qP2/4p1PQ/4P3/5B2/4N1K1/8 w - -";

int main() {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();

	UCI::UCI_loop();

	return 0;
}