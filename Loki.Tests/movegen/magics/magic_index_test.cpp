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
#include "Loki/movegen/magics/magic_index.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"
#include "Loki/movegen/magics/generation/rook_generator.hpp"
#include "Loki/movegen/magics/generation/bishop_generator.hpp"

namespace movegen_tests::magics_tests
{
	using namespace loki;
	using namespace loki::position;
	using namespace loki::movegen::magics;
	using namespace loki::movegen::magics::generation;

	TEST_CASE("hardcoded magic indexes match generated attacks", "[movegen][magics][magic_index]")
	{
		for (const auto piece_type : {ROOK, BISHOP})
		{
			SECTION(piece_type == ROOK ? "rook" : "bishop")
			{
				std::unique_ptr<magic_index> index;
				if (piece_type == ROOK)
					index = std::make_unique<hardcoded_index<ROOK>>();
				else
					index = std::make_unique<hardcoded_index<BISHOP>>();

				std::unique_ptr<sliding_generator> generator;
				if (piece_type == ROOK)
					generator = std::make_unique<rook_generator>();
				else
					generator = std::make_unique<bishop_generator>();

				for (auto sq = A1; sq <= H8; ++sq)
				{
					const auto occupancies = generator->relevant_occupancies(sq);
					for (const auto occupancy : occupancies)
						REQUIRE(index->attacks(sq, occupancy) == generator->attack(sq, occupancy));
				}
			}
		}
	}
}
