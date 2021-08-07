/*
	Loki, a UCI-compliant chess playing software
	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

	Loki is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Loki is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "texel.h"


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


	using namespace Texel;
	
	Parameters vs;
	
	
	//for (int i = 0; i < 8; i++) {
	//	for (int j = 0; j < 7; j++) {
	//		vs.push_back(Parameter(&king_pawn_shelter[i][j]));
	//	}
	//}
	//for (int i = 0; i < 8; i++) {
	//	for (int j = 0; j < 7; j++) {
	//		vs.push_back(Parameter(&king_pawn_storm[i][j]));
	//	}
	//}
	
	//for (int i = 0; i < 4; i++) {
	//	for (int j = 0; j < 3; j++) {
	//		for (int w = 0; w < 3; w++) {
	//			vs.push_back(Parameter(&defending_minors[i][j][w], Value(0.004, 0.004), Value(16.0, 16.0)));
	//		}
	//	}
	//}
	//
	//Tune(vs, "C:\\Users\\abild\\Desktop\\data\\text\\10MB\\quiet-labeled.epd", 250);

	return 0;
}
