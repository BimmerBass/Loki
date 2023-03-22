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


namespace loki::movegen::magics
{

	namespace
	{

		// Return blockers bitboard from a mask and an iteration number.
		bitboard_t blockers_permutation(size_t iteration, bitboard_t mask) noexcept
		{
			bitboard_t blockers = 0;

			while (iteration != 0)
			{
				if ((iteration & 1) != 0)
				{
					auto shift = scan_forward(mask);
					blockers |= (bitboard_t(1) << shift);
				}
				iteration >>= 1;
				mask &= (mask - 1); // <-- removes lsb.
			}
			return blockers;
		}

		inline size_t transform_to_key(bitboard_t blockers, bitboard_t magic, size_t shift) noexcept
		{
			return static_cast<size_t>((blockers * magic) >> (64 - shift));
		}

		/// <summary>
		/// Generate attacks from a specific square  with a given set of blockers.
		/// </summary>
		template<ePiece _Pce>
		bitboard_t generate_attack(eSquare sq, bitboard_t blockers) noexcept
		{
			return 0;
		}

		// For bishops
		template<>
		bitboard_t generate_attack<BISHOP>(eSquare sq, bitboard_t blockers) noexcept
		{
			bitboard_t attacks = 0;
			int cr = rank(sq);
			int cf = file(sq);

			for (int r = cr + 1, f = cf + 1; r <= RANK_8 && f <= FILE_H; r++, f++)
			{
				attacks |= (bitboard_t(1) << get_square(r, f));
				if (((bitboard_t(1) << get_square(r, f)) & blockers) != 0)
					break;
			}
			for (int r = cr + 1, f = cf - 1; r <= RANK_8 && f >= FILE_A; r++, f--)
			{
				attacks |= (bitboard_t(1) << get_square(r, f));
				if (((bitboard_t(1) << get_square(r, f)) & blockers) != 0)
					break;
			}
			for (int r = cr - 1, f = cf + 1; r >= RANK_1 && f <= FILE_H; r--, f++)
			{
				attacks |= (bitboard_t(1) << get_square(r, f));
				if (((bitboard_t(1) << get_square(r, f)) & blockers) != 0)
					break;
			}
			for (int r = cr - 1, f = cf - 1; r >= RANK_1 && f >= FILE_A; r--, f--)
			{
				attacks |= (bitboard_t(1) << get_square(r, f));
				if (((bitboard_t(1) << get_square(r, f)) & blockers) != 0)
					break;
			}

			return attacks;
		}

		// For rooks.
		template<>
		bitboard_t generate_attack<ROOK>(eSquare sq, bitboard_t blockers) noexcept
		{
			bitboard_t attacks = 0;
			int cr = rank(sq);
			int cf = file(sq);

			for (int r = cr + 1; r <= RANK_8; r++)
			{
				attacks |= (bitboard_t(1) << get_square(r, cf));
				if (((bitboard_t(1) << get_square(r, cf)) & blockers) != 0)
					break;
			}
			for (int r = cr - 1; r >= RANK_1; r--)
			{
				attacks |= (bitboard_t(1) << get_square(r, cf));
				if (((bitboard_t(1) << get_square(r, cf)) & blockers) != 0)
					break;
			}
			for (int f = cf + 1; f <= FILE_H; f++)
			{
				attacks |= (bitboard_t(1) << get_square(cr, f));
				if (((bitboard_t(1) << get_square(cr, f)) & blockers) != 0)
					break;
			}
			for (int f = cf - 1; f >= FILE_A; f--)
			{
				attacks |= (bitboard_t(1) << get_square(cr, f));
				if (((bitboard_t(1) << get_square(cr, f)) & blockers) != 0)
					break;
			}

			return attacks;
		}
	}

	// Explicit instantiations.
	template class magics_index<BISHOP>;
	template class magics_index<ROOK>;

	template<ePiece _Pce>
	magics_index<_Pce>::magics_index() : m_attack_index(nullptr), m_magic_entries{ 0 } {
		m_attack_index = std::make_unique<index_t>();
		init();
	}

	/// <summary>
	/// Get the attack bitboard for a given square and occupancy.
	/// </summary>
	/// <param name="sq"></param>
	/// <param name="occupancy"></param>
	/// <returns></returns>
	template<ePiece _Pce>
	bitboard_t magics_index<_Pce>::attacks_bb(eSquare sq, bitboard_t occupancy) const noexcept
	{
		occupancy &= m_magic_entries[sq].mask;
		occupancy *= m_magic_entries[sq].magic;
		occupancy >>= 64 - m_magic_entries[sq].shift;
		return (*m_attack_index)[sq][occupancy];
	}


	/// <summary>
	/// Initialize the magics lookup table.
	/// </summary>
	/// <returns></returns>
	template<ePiece _Pce>
	void magics_index<_Pce>::init() noexcept
	{
		initialize_entry_table();

		for (eSquare sq = A1; sq <= H8; sq++)
		{
			magic_entry& current_entry = m_magic_entries[sq];

			size_t permutations = bitboard_t(1) << count_bits(current_entry.mask);

			for (size_t i = 0; i < permutations; i++)
			{
				bitboard_t blockers = blockers_permutation(i, current_entry.mask);
				bitboard_t attacks = generate_attack<_Pce>(sq, blockers);

				auto key = transform_to_key(blockers, current_entry.magic, count_bits(current_entry.mask));
				(*m_attack_index)[sq][key] = attacks;
			}
		}
	}

	template<>
	void magics_index<BISHOP>::initialize_entry_table() noexcept
	{
		for (eSquare sq = A1; sq <= H8; sq++)
		{
			bitboard_t attacks = 0;
			int cr = rank(sq);
			int cf = file(sq);

			for (int r = cr + 1, f = cf + 1; r <= RANK_7 && f <= FILE_G; r++, f++)
				attacks |= (bitboard_t(1) << get_square(r, f));
			for (int r = cr + 1, f = cf - 1; r <= RANK_7 && f >= FILE_B; r++, f--)
				attacks |= (bitboard_t(1) << get_square(r, f));
			for (int r = cr - 1, f = cf + 1; r >= RANK_2 && f <= FILE_G; r--, f++)
				attacks |= (bitboard_t(1) << get_square(r, f));
			for (int r = cr - 1, f = cf - 1; r >= RANK_2 && f >= FILE_B; r--, f--)
				attacks |= (bitboard_t(1) << get_square(r, f));

			m_magic_entries[sq].mask = attacks;
			m_magic_entries[sq].magic = magics[sq];
			m_magic_entries[sq].shift = count_bits(m_magic_entries[sq].mask);
		}
	}

	template<>
	void magics_index<ROOK>::initialize_entry_table() noexcept
	{
		for (eSquare sq = A1; sq <= H8; sq++)
		{
			bitboard_t attacks = 0;
			int cr = rank(sq);
			int cf = file(sq);

			for (int r = cr + 1; r <= RANK_7; r++) attacks |= (bitboard_t(1) << get_square(r, cf));
			for (int r = cr - 1; r >= RANK_2; r--) attacks |= (bitboard_t(1) << get_square(r, cf));
			for (int f = cf + 1; f <= FILE_G; f++) attacks |= (bitboard_t(1) << get_square(cr, f));
			for (int f = cf - 1; f >= FILE_B; f--) attacks |= (bitboard_t(1) << get_square(cr, f));

			m_magic_entries[sq].mask = attacks;
			m_magic_entries[sq].magic = magics[sq];
			m_magic_entries[sq].shift = count_bits(m_magic_entries[sq].mask);
		}
	}
}