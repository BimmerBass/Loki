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
#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <chrono>
#include <iostream>
#include <sstream>

#if (defined(_WIN32) || defined(_WIN64))

#if defined(_MSC_VER)
#define NOMINMAX
#endif

#include <windows.h>
#include <io.h>
#else
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#endif


// InputWaiting is used to see if the GUI has sent information to Loki.
int InputWaiting();

// ReadInput will determine if that input is either a stop or quit command.
void ReadInput(bool& isStop, bool &isQuit);

// Gets the time in milliseconds since epoch
inline long long getTimeMs() {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}




#endif // ifndef MISC_H