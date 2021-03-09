#include "uci.h"



int main() {
	BBS::INIT();
	Magics::INIT();
	Search::INIT();
	Eval::INIT();
	PSQT::INIT();

	UCI::UCI_loop();

	return 0;
}