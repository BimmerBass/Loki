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
#include <type_traits>
#include <assert.h>
#include "bitboard.hpp"


namespace loki::position
{
	namespace
	{
		std::optional<size_t> runtime_ms1b(bitboard_t x)
		{
			assert(x != 0);
			size_t n = 0;

#if defined(__GNUC__) || defined(__GNUG__) // g++/gcc and clang
			n = size_t(63 ^ __builtin_clzll(bb)));
#elif defined(_MSC_VER) // windows
#ifdef _WIN64
			unsigned long idx;
			_BitScanReverse64(&idx, x);
			n = (size_t)idx;
#else
			// 32-bit
			unsigned long idx;
			if (bb >> 32)
			{
				_BitScanReverse(&idx, int32_t(bb >> 32));
				n = size_t(idx + 32);
			}
			else
			{
				_BitScanReverse(&idx, int32_t(bb));
				n = size_t(idx);
			}
#endif
#else
			n = bit_scan_reverse_fallback(x);
#endif
			return std::optional<size_t>(n);
		}

		std::optional<size_t> runtime_ls1b(bitboard_t x)
		{
			assert(x != 0);
			size_t n = 0;

#if defined(__GNUC__) || defined(__GNUG__) // g++/gcc and clang
			n = size_t(__builtin_ctzll(bb));
#elif defined(_MSC_VER) // windows
			unsigned long idx;
#ifdef _WIN64
			_BitScanForward64(&idx, x);
			n = (size_t)idx;
#else
			// 32-bit
			if (bb & 0xffffffff)
			{
				_BitScanForward(&idx, int32_t(bb));
				n = size_t(idx);
			}
			else
			{
				_BitScanForward(&idx, int32_t(bb >> 32));
				n = size_t(idx + 32);
			}
#endif
#else
			n = bit_scan_forward_fallback(x);
#endif
			return std::optional<size_t>(n);
		}

		static constexpr int index64[64] = {
			0, 47,  1, 56, 48, 27,  2, 60,
			57, 49, 41, 37, 28, 16,  3, 61,
			54, 58, 35, 52, 50, 42, 21, 44,
			38, 32, 29, 23, 17, 11,  4, 62,
			46, 55, 26, 59, 40, 36, 15, 53,
			34, 51, 20, 43, 31, 22, 10, 45,
			25, 39, 14, 33, 19, 30,  9, 24,
			13, 18,  8, 12,  7,  6,  5, 63
		};

		constexpr size_t bit_scan_reverse_fallback(bitboard_t x)
		{
			const bitboard_t debruijn64 = 0x03f79d71b4cb0a89;
			x |= x >> 1;
			x |= x >> 2;
			x |= x >> 4;
			x |= x >> 8;
			x |= x >> 16;
			x |= x >> 32;
			return index64[(x * debruijn64) >> 58];
		}

		constexpr size_t bit_scan_forward_fallback(bitboard_t x)
		{
			const bitboard_t debruijn64 = 0x03f79d71b4cb0a89;
			return index64[((x ^ (x - 1)) * debruijn64) >> 58];
		}
	}


	constexpr std::optional<size_t> bitboard::ms1b() const noexcept
	{
		if (x == 0)
			return std::nullopt;

		if (std::is_constant_evaluated())
		{
			return std::optional<size_t>(bit_scan_reverse_fallback(x));
		}
		return runtime_ms1b(x);
	}
	constexpr std::optional<size_t> bitboard::ls1b() const noexcept
	{
		if (x == 0)
			return std::nullopt;

		if (std::is_constant_evaluated())
		{
			return std::optional<size_t>(bit_scan_forward_fallback(x));
		}
		return runtime_ls1b(x);
	}
}