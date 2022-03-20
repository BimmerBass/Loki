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
#include "misc.h"



// I have lost the original link to the source of this function, but I have it from Vice.
int InputWaiting()
{
#ifndef _WIN32
	fd_set readfds;
	struct timeval tv;
	FD_ZERO(&readfds);
	FD_SET(fileno(stdin), &readfds);
	tv.tv_sec = 0; tv.tv_usec = 0;
	select(16, &readfds, 0, 0, &tv);

	return (FD_ISSET(fileno(stdin), &readfds));
#else
	static int init = 0, pipe;
	static HANDLE inh;
	DWORD dw;

	if (!init) {
		init = 1;
		inh = GetStdHandle(STD_INPUT_HANDLE);
		pipe = !GetConsoleMode(inh, &dw);
		if (!pipe) {
			SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(inh);
		}
	}
	if (pipe) {
		if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
		return dw;
	}
	else {
		GetNumberOfConsoleInputEvents(inh, &dw);
		return dw <= 1 ? 0 : dw;
	}
#endif
}

void ReadInput(bool& isStop, bool& isQuit) {
	
	//if (InputWaiting()) {
	//	isStop = true;
	//	
	//	std::string input = "";
	//	std::getline(std::cin, input);
	//
	//	if (input.find("quit") != std::string::npos) {
	//		isQuit = true;
	//	}
	//}
	int             bytes = 0;
	char            input[256] = "", * endc;

	if (InputWaiting()) {
		isStop = true;
		do {
#ifndef _WIN32
			bytes = read(fileno(stdin), input, 256);
#else
			bytes = _read(_fileno(stdin), input, 256);
#endif
		} while (bytes < 0);
		endc = strchr(input, '\n');
		if (endc) *endc = 0;

		if (strlen(input) > 0) {
			if (!strncmp(input, "quit", 4)) {
				isQuit = true;
			}
		}
		return;
	}
}