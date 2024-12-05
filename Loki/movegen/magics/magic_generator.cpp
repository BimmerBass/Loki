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
			auto fail = false;
			magic.magic_number = generate_valid_random(magic.mask);

			std::fill_n(
				magic.attacks.begin(),
				magic.attacks.size(), 0ULL);

			for (auto i = 0; i < occupancies.size() && !fail; i++)
			{
				auto index = magic.get_index(occupancies[i]);

				if (magic.attacks[index] == 0ULL)
					magic.attacks[index] = attacks[i];
				else if (magic.attacks[index] != attacks[i])
					fail = true;
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