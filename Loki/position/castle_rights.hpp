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
#include "defs.hpp"

namespace loki::position
{
	enum castling_direction : uint8_t
	{
		KINGSIDE = 0,
		QUEENSIDE
	};

	class castle_rights
	{
	public:
		/// <summary>
		/// Initialize to no rights.
		/// </summary>
		constexpr castle_rights() : m_rights{ 0 } {}

		/// <summary>
		/// Check if castling is possible for a given side and direction.
		/// </summary>
		/// <typeparam name="_S">The side (w/b)</typeparam>
		/// <typeparam name="_D">Direction (ks/qs)</typeparam>
		/// <returns>true if it is possible, false otherwise</returns>
		template<side _S, castling_direction _D>
		constexpr inline bool can_castle() const noexcept
		{
			return (m_rights & mask<_S,_D>()) != 0;
		}

		/// <summary>
		/// Set a side's and direction's castling ability.
		/// </summary>
		/// <typeparam name="_S">Side (w/b)</typeparam>
		/// <typeparam name="_D">Direction (ks/qs)</typeparam>
		/// <param name="value">The value to set the ability</param>
		template<side _S, castling_direction _D>
		constexpr inline void set(bool value) noexcept
		{
			if (can_castle<_S, _D>() != value)
			{
				// toggle
				auto m = mask<_S, _D>();
				m_rights ^= m;
			}
		}

		/// <summary>
		/// Compute a string representing the current object.
		/// </summary>
		/// <returns>A FEN-valid string representing the castling rights.</returns>
		inline std::string to_string() const
		{
			std::string castle_rights = "";
			if (can_castle<WHITE, KINGSIDE>())
				castle_rights += "K";
			if (can_castle<WHITE, QUEENSIDE>())
				castle_rights += "Q";
			if (can_castle<BLACK, KINGSIDE>())
				castle_rights += "k";
			if (can_castle<BLACK, QUEENSIDE>())
				castle_rights += "q";
			return castle_rights.empty() ? "-" : castle_rights;
		}

	private:
		uint8_t m_rights;

		template<side _S, castling_direction _D>
		inline uint8_t mask() const noexcept
		{
			return 0x01 << (0x02 * (uint8_t)_S + (uint8_t)_D);
		}
	};
}