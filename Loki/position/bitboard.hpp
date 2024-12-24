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
#include <bit>
#include <cstdint>
#include <optional>
#include <cassert>
#include <array>
#include "util/operators.hpp"

#define rt_assert(_cond) \
	if (!std::is_constant_evaluated()){ \
		assert(_cond); \
	}

namespace loki::position
{
	using bitboard_t = uint64_t;

	enum e_direction
	{
		UP = 0,
		DOWN,
		LEFT,
		RIGHT
	};
	ENABLE_INCR_OPERATORS_ON(e_direction);

	consteval std::array<bitboard_t, 8> init_file_masks()
	{
		std::array<bitboard_t, 8> files;
		files[0] = 0x101010101010101ULL;

		for (size_t i = 1; i < 8; i++)
		{
			files[i] = files[i - 1] << 1;
		}
		return files;
	}

	consteval std::array<bitboard_t, 8> init_rank_masks()
	{
		std::array<bitboard_t, 8> ranks;
		ranks[0] = 0x00000000FFULL;

		for (size_t i = 1; i < 8; i++)
		{
			ranks[i] = ranks[i - 1] << 8;
		}
		return ranks;
	}

	constexpr std::array<bitboard_t, 8> FILE_MASKS = init_file_masks();
	constexpr std::array<bitboard_t, 8> RANK_MASKS = init_rank_masks();

	/// <summary>
	/// Determine if the i'th bit is 1.
	/// </summary>
	/// <typeparam name="tIdx">Numeric type</typeparam>
	/// <param name="x">The bitboard</param>
	/// <param name="i">The index at which to check</param>
	/// <returns>true if the i'th bit is 1, false otherwise.</returns>
	template<typename tIdx>
	constexpr bool is_one_at(bitboard_t x, tIdx i) noexcept
	{
		rt_assert(i >= 0 && i <= 63);
		return ((x >> i) & 1) != 0;
	}

	/// <summary>
	/// Count the number of one-bits.
	/// </summary>
	/// <param name="x">The bitboard</param>
	/// <returns>The number of bits set to 1</returns>
	constexpr size_t popcount(bitboard_t x) noexcept
	{
		return static_cast<size_t>(std::popcount<uint64_t>(x));
	}

	/// <summary>
	/// Find the index of the least significant 1-bit.
	/// </summary>
	/// <param name="x">The bitboard</param>
	/// <returns>index.</returns>
	std::optional<size_t> scan_lsb(bitboard_t x) noexcept;

	/// <summary>
	/// Find the index of the most significant 1-bit.
	/// </summary>
	/// <param name="x">The bitboard</param>
	/// <returns>index.</returns>
	std::optional<size_t> scan_msb(bitboard_t x) noexcept;

	/// <summary>
	/// Shift a bitboard in a given direction.
	/// </summary>
	/// <typeparam name="D">Direction to shift</typeparam>
	/// <returns>A new bitboard instance.</returns>
	template<e_direction D>
	constexpr bitboard_t shift(bitboard_t bb) noexcept
	{
		switch (D)
		{
		case UP:
			return (bb & ~RANK_MASKS[7]) << 8;
		case DOWN:
			return (bb & ~RANK_MASKS[0]) >> 8;
		case LEFT:
			return (bb & ~FILE_MASKS[0]) >> 1;
		}
		return (bb & ~FILE_MASKS[7]) << 1; // right
	}

	template<e_direction D1, e_direction D2>
	constexpr bitboard_t shift(bitboard_t bb) noexcept
	{
		return shift<D1>(shift<D2>(bb));
	}

	template<e_direction D>
	constexpr bitboard_t shift_combine(bitboard_t bb) noexcept
	{
		return shift<D>(bb);
	}
	/// <summary>
	/// Shift the bitboard in a given set of directions, accumulating the result of each shift-operation
	/// </summary>
	/// <typeparam name="D1">The current direction</typeparam>
	/// <typeparam name="D2">The next direction</typeparam>
	/// <typeparam name="...Ds">The next directions</typeparam>
	/// <param name="bb">The bitboard</param>
	/// <returns>A bitboard with the "path" given by D and Ds set to ones.</returns>
	template<e_direction D1, e_direction D2, e_direction... Ds>
	constexpr bitboard_t shift_combine(bitboard_t bb) noexcept
	{
		auto shifted = shift_combine<D1>(bb);
		return shifted |  shift_combine<D2, Ds...>(shifted);
	}

	/// <summary>
	/// Set the i'th bit to 1.
	/// </summary>
	/// <typeparam name="tIdx">Numeric type</typeparam>
	/// <param name="x">The bitboard</param>
	/// <param name="i">Index at which to set the bit to 1</param>
	/// <returns>a reference to the current object.</returns>
	template<typename tIdx>
	constexpr bitboard_t set_one_at(bitboard_t x, tIdx i) noexcept
	{
		rt_assert(i >= 0 && i <= 63);
		x |= bitboard_t(1) << i;
		return x;
	}

	/// <summary>
	/// Toggle the i'th bit.
	/// </summary>
	/// <typeparam name="tIdx">Numeric type.</typeparam>
	/// <param name="x">The bitboard</param>
	/// <param name="i">The index to toggle</param>
	/// <returns>a reference to the current object.</returns>
	template<typename tIdx>
	constexpr bitboard_t toggle_at(bitboard_t x, tIdx i) noexcept
	{
		rt_assert(i >= 0 && i <= 63);
		x ^= bitboard_t(1) << i;
		return x;
	}

	/// <summary>
	/// Find the least significant 1-bit and set it to zero.
	/// If all bits are zero, nothing is changed.
	/// </summary>
	/// <param name="x">The bitboard</param>
	/// <returns>An optional size_t, having a value if *this != 0ULL</returns>
	inline std::optional<size_t> pop_lsb(bitboard_t& x) noexcept
	{
		// scan_lsb guarantees the bit is 1 so we can XOR without worrying about setting a 0 to 1..
		auto i = scan_lsb(x);
		if (i.has_value())
			x ^= bitboard_t(1) << i.value();
		return i;
	}


	/// <summary>
	/// Print a bitboard to the console.
	/// </summary>
	/// <param name="bb">The bitboard to print</param>
	void print_bitboard(bitboard_t bb);
}