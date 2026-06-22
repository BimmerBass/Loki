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

#include "pch.hpp"
#include "Loki/movegen/magics/generation/bishop_generator.hpp"

namespace movegen_tests::generation_tests::magics_tests
{
	using namespace loki::position;
	using namespace loki::movegen::magics::generation;

	TEST_CASE("sliding generators enumerate valid occupancies", "[movegen][magics][sliding]")
	{
		bishop_generator generator;
		for (auto sq = A1; sq <= H8; ++sq)
		{
			const auto base_mask = generator.occupancy_mask(sq);
			auto occupancies = generator.relevant_occupancies(sq);

			REQUIRE(occupancies.size() == (1ULL << popcount(base_mask)));
			for (const auto occupancy : occupancies)
				REQUIRE((occupancy & ~base_mask) == 0ULL);

			std::sort(occupancies.begin(), occupancies.end());
			REQUIRE(std::unique(occupancies.begin(), occupancies.end()) == occupancies.end());
		}
	}
}
