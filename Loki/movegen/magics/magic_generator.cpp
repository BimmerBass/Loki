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
#include <assert.h>
#include "magic_generator.hpp"
#include "util/rng.hpp"

namespace loki::movegen::magics
{
	using namespace loki::util;
	using namespace loki::position;

	std::array<magic, NUM_SQUARES> magic_generator::generate()
	{
		std::array<magic, NUM_SQUARES> magics;
		for (auto sq = A1; sq <= H8; sq++)
		{
			magics[sq] = find_magic(sq);
		}
		return magics;
	}

	magic magic_generator::find_magic(square sq)
	{
		size_t iteration = 0;
		auto magic_found = false;
		std::vector<bitboard> occupancies = m_generator->relevant_occupancies(sq), attacks(occupancies.size());
		std::transform(
			occupancies.begin(), occupancies.end(),
			attacks.begin(), [&](auto& m) { return m_generator->attack(sq, m); });
		magic magic(occupancies.size());
		magic.mask = m_generator->occupancy_mask(sq);
		magic.shift = magic.mask.num_one_bits();

		while (!magic_found)
		{
			iteration++;
			auto fail = false;
			magic.magic_number = generate_valid_random(magic.mask);

			for (auto i = 0; i < occupancies.size() && !fail; i++)
			{
				auto index = magic.get_index(occupancies[i]);

				if (magic.attacks[index].age < iteration)
				{
					magic.attacks[index].age = iteration;
					magic.attacks[index].attack = attacks[i];
				}
				else if (magic.attacks[index].attack != attacks[i])
				{
					fail = true;
					break;
				}
			}
			if (!fail)
				magic_found = true;
		}
		return magic;
	}

	bitboard magic_generator::generate_valid_random(const bitboard& mask) const
	{
		bitboard num = rng::instance()->generate_sparse<uint64_t>();
		int i = 0;
		while (((num * mask) & 0xFF00000000000000ULL).num_one_bits() < 6)
		{
			num = rng::instance()->generate_sparse<uint64_t>();
			i++;
		}
		return num;
	}
}