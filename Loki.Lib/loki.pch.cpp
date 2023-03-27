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
#include "loki.pch.hpp"

namespace loki
{
	namespace
	{
		std::map<char, eFile> file_mappings = {
			{'a', FILE_A},
			{'b', FILE_B},
			{'c', FILE_C},
			{'d', FILE_D},
			{'e', FILE_E},
			{'f', FILE_F},
			{'g', FILE_G},
			{'h', FILE_H},
		};
		std::array<std::string, FILE_NB> file_names = { "a", "b", "c", "d", "e", "f", "g", "h" };

	}

	eSquare from_algebraic(std::string str)
	{
		auto file = str[0];
		auto rank = str[1];
		return get_square((rank - '0') - 1, file_mappings[(char)std::tolower(file)]);
	}
	std::string to_algebraic(eSquare sq)
	{
		size_t f = file(sq);
		size_t r = rank(sq) + 1;
		return file_names[f] + std::to_string(r);
	}
}