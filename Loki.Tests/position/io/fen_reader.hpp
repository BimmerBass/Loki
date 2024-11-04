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