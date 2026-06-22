// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once
#include "pch.hpp"
#include "Loki/util/stringops.hpp"

namespace position_tests::io_tests
{
	namespace fs = std::filesystem;

	struct test_fen
	{
		std::string fen;
		std::string piece_placements;
		std::string side_to_move;
		std::string castling_abilities;
		std::string en_passant_sq;
		std::string halfmove_clock;
		std::string fullmove_clock;
	};

	class fen_reader
	{
	public:
		fen_reader()
		{
			auto path = fs::path(get_project_dir()) / "fens.epd";
			assert(fs::exists(path));

			auto file = std::ifstream(path);
			auto firstLine = true;
			std::string line;
			while (std::getline(file, line))
			{
				if (firstLine)
				{
					firstLine = false;
					continue;
				}
				auto splitted = loki::util::split(line, '\t');
				assert(splitted.size() == 7);
				fens.push_back(test_fen{
					splitted[0],
					splitted[1],
					splitted[2],
					splitted[3],
					splitted[4],
					splitted[5],
					splitted[6]
					});
			}
		}

		std::vector<test_fen> fens;
	};
}
