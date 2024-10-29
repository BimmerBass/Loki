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
#pragma once
#include <tuple>

#include "defs.hpp"

namespace loki::position
{
	class castle_rights
	{
	public:
		/// <summary>
		/// Initialize to no rights.
		/// </summary>
		castle_rights() : m_rights{ 0 } {}

		/// <summary>
		/// Check if castling is possible for a given side and direction.
		/// </summary>
		/// <typeparam name="_S">The side (w/b)</typeparam>
		/// <typeparam name="_D">Direction (ks/qs)</typeparam>
		/// <returns>true if it is possible, false otherwise</returns>
		template<side _S, castling_direction _D>
		inline bool can_castle() const noexcept
		{
			return m_rights & mask<_S,_D>() != 0;
		}

		/// <summary>
		/// Set a side's and direction's castling ability.
		/// </summary>
		/// <typeparam name="_S">Side (w/b)</typeparam>
		/// <typeparam name="_D">Direction (ks/qs)</typeparam>
		/// <param name="value">The value to set the ability</param>
		template<side _S, castling_direction _D>
		inline void set(bool value) noexcept
		{
			if (can_castle<_S, _D>() != value)
			{
				// toggle
				m_rights ^= mask<_S, _D>();
			}
		}

	private:
		uint8_t m_rights;

		template<side _S, castling_direction _D>
		inline uint8_t mask() const noexcept
		{
			return 0x02 * (uint8_t)_S + (uint8_t)_D;
		}
	};
}