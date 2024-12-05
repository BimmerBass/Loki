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
#include "magic_index.hpp"

namespace loki::movegen::magics
{
	using namespace loki::position;

	bitboard magic_index::attacks(e_square sq, bitboard_t occupancy) const
	{
		assert(m_initialized);
		auto ptr = m_blocks[sq].attacks;
		return ptr[calculate_index(sq, occupancy)];
	}

	size_t magic_index::calculate_index(e_square sq, bitboard_t occupancy) const
	{
		auto& block = m_blocks[sq];
		occupancy &= block.mask;
		occupancy *= block.magic;
		occupancy >>= 64 - block.shift;
		return static_cast<size_t>(occupancy);
	}

	void magic_index::initialize(const position::bitboard_t* magics, const generation::sliding_generator* gen)
	{
		// init everything except for the attacks-pointers
		prefill_blocks(magics, gen);
		size_t index_size = std::accumulate(
			m_blocks.begin(), m_blocks.end(), 0ULL,
			[](size_t acc, const M& b) { return acc + (1ULL << b.shift); });

		m_index = new bitboard[index_size];
		partition_blocks(index_size);

		for (auto sq = A1; sq <= H8; sq++)
		{
			auto occupancies = gen->relevant_occupancies(sq);
			for (auto& occ : occupancies)
			{
				m_blocks[sq].attacks[calculate_index(sq, occ.get_raw())] = gen->attack(sq, occ);
			}
		}

		m_initialized = true;
	}

	void magic_index::prefill_blocks(const position::bitboard_t* magics, const generation::sliding_generator* gen)
	{
		for (auto sq = A1; sq <= H8; sq++)
		{
			auto m = gen->occupancy_mask(sq);
			m_blocks[sq].magic = magics[sq];
			m_blocks[sq].mask = m.get_raw();
			m_blocks[sq].shift = m.num_one_bits();
		}
	}

	void magic_index::partition_blocks(size_t index_size)
	{
		size_t offset = 0;
		for (auto sq = A1; sq <= H8; sq++)
		{
			auto& block = m_blocks[sq];
			auto block_size = 1ULL << block.shift;
			if (offset + block_size > index_size)
				throw_msg<index_exception>("accumulated block size exceeded predicted index size.");

			block.attacks = m_index + offset;

			offset += block_size;
		}
	}
}