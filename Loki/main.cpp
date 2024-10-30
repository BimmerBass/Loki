#include "util/exception.hpp"
#include "position/game_state.hpp"
#include "versioninfo.hpp"
#include <iostream>

void foo()
{
	loki::throw_msg<loki::loki_exception>("Hello there, I am {}!", "Niels");
}


int main()
{
	try
	{
		std::printf("NAME: %s\n", NAME);
		std::printf("AUTHOR: %s\n", AUTHOR);
		std::printf("VERSION: %s\n", VERSION);
	}
	catch (const loki::loki_exception& e)
	{
		std::printf("Error!\nMessage: %s\n\nStack-trace:\n%s", e.what(), std::to_string(e.trace()).c_str());
	}
}