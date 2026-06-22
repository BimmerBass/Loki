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
#include "Loki/movegen/magics/generation/magic_generator.hpp"
#include "Loki/movegen/magics/generation/rook_generator.hpp"
#include "Loki/movegen/magics/generation/bishop_generator.hpp"
#include "Loki/defs.hpp"

namespace movegen_tests::generation_tests::magics_tests
{
	using namespace loki;
	using namespace loki::position;
	using namespace loki::movegen::magics;
	using namespace loki::movegen::magics::generation;

	TEST_CASE("magic generator produces valid lookup tables", "[movegen][magics][magic_generator]")
	{
		for (const auto piece_type : {ROOK, BISHOP})
		{
			SECTION(piece_type == ROOK ? "rook" : "bishop")
			{
				std::shared_ptr<sliding_generator> generator = (piece_type == ROOK)
					? std::static_pointer_cast<sliding_generator>(std::make_shared<rook_generator>())
					: std::static_pointer_cast<sliding_generator>(std::make_shared<bishop_generator>());
				magic_generator gen(generator);
				auto magics = gen.generate();

				for (auto sq = A1; sq <= H8; ++sq)
				{
					const auto& magic = magics[sq];
					const auto occupancies = generator->relevant_occupancies(sq);
					REQUIRE(magic.attacks.size() == (1ULL << magic.shift));

					for (const auto occupancy : occupancies)
					{
						const auto index = magic.get_index(occupancy);
						REQUIRE(index >= 0);
						REQUIRE(static_cast<size_t>(index) <= magic.attacks.size());
						REQUIRE(magic.attacks[static_cast<size_t>(index)].attack == generator->attack(sq, occupancy));
					}
				}
			}
		}
	}
}
