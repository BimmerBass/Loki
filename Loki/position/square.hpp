#pragma once
#include "bitboard.hpp"
#include "util/exception.hpp"
#include "util/operators.hpp"
#include "util/stringops.hpp"

namespace loki::position
{
	enum e_square : uint64_t
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
	ENABLE_BASE_OPERATORS_ON(e_file);
	ENABLE_INCR_OPERATORS_ON(e_file);

	class square final
	{
	private:
		e_square m_value;
	public:
		inline constexpr square() : square(NO_SQ) {}
		inline constexpr square(e_square s) : m_value{ s }
		{}
		inline constexpr square(e_rank r, e_file f) : m_value{ static_cast<e_square>(r * 8 + f) }
		{}
		inline constexpr square(std::string algebraic)
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
		
		inline constexpr e_square value() const { return m_value; }
		inline constexpr e_rank rank() const { return static_cast<e_rank>(m_value / static_cast<e_square>(NUM_RANKS)); }
		inline constexpr e_file file() const { return static_cast<e_file>(m_value % static_cast<e_square>(NUM_FILES)); }

		inline constexpr void operator++(int) // post-increment
		{
			rt_assert(m_value <= H8);
			m_value++;
		}

		inline std::string to_algebraic() const
		{
			std::string cf = "a";
			switch (file())
			{
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


	inline constexpr bool operator==(square sq1, square sq2) { return sq1.value() == sq2.value(); }
	inline constexpr bool operator<(square sq1, square sq2) { return sq1.value() < sq2.value(); }
	inline constexpr bool operator<=(square sq1, square sq2) { return sq1.value() <= sq2.value(); }
	inline constexpr bool operator>(square sq1, square sq2) { return sq1.value() > sq2.value(); }
	inline constexpr bool operator>=(square sq1, square sq2) { return sq1.value() >= sq2.value(); }
}