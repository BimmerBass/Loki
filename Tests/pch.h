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
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#define LOG(msg) Logger::WriteMessage(msg)
#define LOG(msg, ...) Logger::WriteMessage(std::format(msg, __VA_ARGS__).c_str())


#include "Loki.Lib/loki.pch.hpp"
#include <fstream>
#include <unordered_set>

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
		LOG("Loaded {} fens from {}", m_fens.size(), m_filename);
	}
};

#endif //PCH_H
