//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#include "Loki.Lib/loki.pch.hpp"

inline std::string info()
{
	const std::string version = "4.0";
	std::stringstream ss, date(__DATE__);
	std::string day, month, yr;
	date >> month >> day >> yr;
	ss << "Loki v" << version << " (build: "
		<< day << " "
		<< month << " "
		<< yr << ") by Niels Abildskov (github: BimmerBass)";
	return ss.str();
}

int main()
{
	try
	{
		std::cout << info() << std::endl;
		loki::uci::engine_manager manager;
		manager.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception caugh in main(): " << e.what() << std::endl;
	}
}