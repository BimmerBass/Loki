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

// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021 Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#include "pch.hpp"
#include "Loki/search/perft.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"
#include "Loki/util/stringops.hpp"

namespace search_tests
{
	using namespace loki;
	using namespace loki::movegen::magics;
	using namespace loki::search;

	namespace
	{
		struct perft_case
		{
			std::string fen;
			size_t depth;
			size_t expected_nodes;
		};

		std::vector<perft_case> read_perft_cases(size_t maximum_depth)
		{
			const auto path = std::filesystem::path(get_project_dir()) / "perft.epd";
			std::ifstream file(path);
			if (!file)
				throw std::runtime_error("could not open " + path.string());

			std::vector<perft_case> cases;
			std::string line;
			while (std::getline(file, line))
			{
				const auto fields = loki::util::split(line, ';');
				if (fields.empty())
					continue;

				perft_case parsed{ fields.front(), 0, 0 };
				while (!parsed.fen.empty() && std::isspace(static_cast<unsigned char>(parsed.fen.back())))
					parsed.fen.pop_back();

				for (size_t i = 1; i < fields.size(); i++)
				{
					std::istringstream field(fields[i]);
					std::string depth_label;
					size_t expected_nodes = 0;
					if (!(field >> depth_label >> expected_nodes) ||
						depth_label.size() < 2 ||
						depth_label.front() != 'D')
					{
						continue;
					}

					const auto depth = static_cast<size_t>(std::stoull(depth_label.substr(1)));
					if (depth <= maximum_depth && depth > parsed.depth)
					{
						parsed.depth = depth;
						parsed.expected_nodes = expected_nodes;
					}
				}

				if (parsed.depth != 0)
					cases.push_back(std::move(parsed));
			}

			return cases;
		}
	}

	TEST_CASE("perft can verify incremental position hashes at every leaf", "[search][perft][hashing]")
	{
		constexpr size_t maximum_depth = 4;
		const auto test_cases = read_perft_cases(maximum_depth);
		REQUIRE(test_cases.size() == 127);

		auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
		auto rook_index = std::make_shared<hardcoded_index<ROOK>>();

		for (const auto& test : test_cases)
		{
			CAPTURE(test.fen, test.depth, test.expected_nodes);

			std::ostringstream output;
			perft runner(output, rook_index, bishop_index);

			const auto nodes = runner.run<true>(test.fen, test.depth);
			REQUIRE(nodes == test.expected_nodes);
		}
	}
}
