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
#include "bitboard.hpp"
#include "util/exception.hpp"
#include "util/operators.hpp"
#include "util/stringops.hpp"

namespace loki::position
{
	enum e_square : uint8_t
	{
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		NUM_SQUARES = 64, NO_SQ = 65
	};
	ENABLE_INCR_OPERATORS_ON(e_square);
	ENABLE_FULL_OPERATORS_ON(e_square);
	enum e_rank{
		RANK_1 = 0,
		RANK_2,
		RANK_3,
		RANK_4,
		RANK_5,
		RANK_6,
		RANK_7,
		RANK_8,
		NUM_RANKS,
		NO_RANK
	};
	ENABLE_INCR_OPERATORS_ON(e_rank);
	ENABLE_FULL_OPERATORS_ON(e_rank);
	enum e_file{
		FILE_A = 0,
		FILE_B,
		FILE_C,
		FILE_D,
		FILE_E,
		FILE_F,
		FILE_G,
		FILE_H,
		NUM_FILES,
		NO_FILE
	};
	ENABLE_STRINGIFY(e_file, "A", "B", "C", "D", "E", "F", "G", "H", "NF", "NONE");
	ENABLE_FULL_OPERATORS_ON(e_file);
	ENABLE_INCR_OPERATORS_ON(e_file);

	constexpr e_rank rank_of(e_square sq) noexcept
	{
		return static_cast<e_rank>(sq / static_cast<e_square>(NUM_RANKS));
	}
	constexpr e_file file_of(e_square sq)
	{
		return static_cast<e_file>(sq % static_cast<e_square>(NUM_FILES));
	}

	constexpr e_square vertical_flip(e_square sq) noexcept
	{
		return static_cast<e_square>(sq ^ 56);
	}

	class square final
	{
	private:
		e_square m_value;
	public:
		constexpr square() : square(NO_SQ) {}
		constexpr square(size_t s) : square(static_cast<e_square>(s))
		{}
		constexpr square(e_square s) : m_value{ s }
		{}
		constexpr square(e_rank r, e_file f) : m_value{ static_cast<e_square>(r * 8 + f) }
		{}
		constexpr square(std::string algebraic)
		{
			rt_assert(algebraic.size() == 2);

			algebraic = loki::util::lowercase(algebraic);
			auto f = algebraic[0];
			auto r = algebraic[1];
			rt_assert(r >= '1' && r <= '8');
			rt_assert(f >= 'a' && f <= 'h');
			auto rank = static_cast<e_rank>(r - '1');
			auto file = static_cast<e_file>(f - 'a');

			m_value = static_cast<e_square>(rank * 8 + file);
		}
		
		constexpr e_square value() const noexcept { return m_value; }
		constexpr e_rank rank() const { return rank_of(m_value); }
		constexpr e_file file() const { return file_of(m_value); }

		constexpr square& operator++() // pre-increment
		{
			rt_assert(m_value <= H8);
			m_value++;
			return *this;
		}

		constexpr square operator++(int) // post-increment
		{
			auto old = *this;
			++(*this);
			return old;
		}

		inline std::string to_algebraic() const
		{
			if (m_value == NO_SQ)
				return "-";
			std::string cf;
			switch (file())
			{
			case FILE_A: cf = "a"; break;
			case FILE_B: cf = "b"; break;
			case FILE_C: cf = "c"; break;
			case FILE_D: cf = "d"; break;
			case FILE_E: cf = "e"; break;
			case FILE_F: cf = "f"; break;
			case FILE_G: cf = "g"; break;
			case FILE_H: cf = "h"; break;
			}
			return cf + std::to_string(rank() + 1);
		}
	};

	constexpr bool operator==(square sq1, square sq2) { return sq1.value() == sq2.value(); }
	constexpr bool operator<(square sq1, square sq2) { return sq1.value() < sq2.value(); }
	constexpr bool operator<=(square sq1, square sq2) { return sq1.value() <= sq2.value(); }
	constexpr bool operator>(square sq1, square sq2) { return sq1.value() > sq2.value(); }
	constexpr bool operator>=(square sq1, square sq2) { return sq1.value() >= sq2.value(); }
}
