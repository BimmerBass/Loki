// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

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

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "Loki.Lib/loki.pch.hpp"
#include <fstream>

class fen_reader
{
public:
	fen_reader(std::string filename) : m_filename(filename) {}
private:
	std::string m_filename;
protected:
	std::vector<std::string> m_fens;

	void read_fen_file()
	{
		m_fens.clear();
		auto fen_file = std::ifstream(m_filename);
		std::string current_fen;

		while (std::getline(fen_file, current_fen))
		{
			m_fens.push_back(current_fen);
		}
		fen_file.close();
	}
};

#endif //PCH_H
