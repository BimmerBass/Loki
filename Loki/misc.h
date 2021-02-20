#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <chrono>

#if (defined(_WIN32) || defined(_WIN64))
#define NOMINMAX

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