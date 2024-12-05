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

#define rt_assert(_cond) \
	if (!std::is_constant_evaluated()){ \
		assert(_cond); \
	}

namespace loki::position
{
	using bitboard_t = uint64_t;

	enum e_direction : int
	{
		UP = 8,
		DOWN = -8,
		LEFT = -1,
		RIGHT = 1
	};

	consteval std::array<bitboard_t, 8> init_file_masks()
	{
		std::array<bitboard_t, 8> files;
		files[0] = 0x101010101010101ULL;

		for (int i = 1; i < 8; i++)
		{
			files[i] = files[i - 1] << 1;
		}
		return files;
	}

	consteval std::array<bitboard_t, 8> init_rank_masks()
	{
		std::array<bitboard_t, 8> ranks;
		ranks[0] = 0x00000000FFULL;

		for (int i = 1; i < 8; i++)
		{
			ranks[i] = ranks[i - 1] << 8;
		}
		return ranks;
	}

	constexpr std::array<bitboard_t, 8> FILE_MASKS = init_file_masks();
	constexpr std::array<bitboard_t, 8> RANK_MASKS = init_rank_masks();

	/// <summary>
	/// NOTE: All index-based operations are zero-indexed and i < 0 || i > 63 is undefined behaviour.
	/// </summary>
	class bitboard final
	{
	private:
		bitboard_t x;
	public:
		inline constexpr bitboard() : x{ 0 } {}
		inline constexpr bitboard(bitboard_t num) : x{ num } {}

#pragma region const methods
		/// <summary>
		/// Get the underlying bitboard_t value that this instance represents.
		/// </summary>
		/// <returns>A bitboard_t value</returns>
		inline constexpr bitboard_t get_raw() const noexcept
		{
			return x;
		}

		/// <summary>
		/// Determine if the i'th bit is 1.
		/// </summary>
		/// <typeparam name="tIdx">Numeric type</typeparam>
		/// <param name="i">The index at which to check</param>
		/// <returns>true if the i'th bit is 1, false otherwise.</returns>
		template<typename tIdx>
		inline constexpr bool is_one_at(tIdx i) const noexcept
		{
			rt_assert(i >= 0 && i <= 63);
			return ((x >> i) & 1) != 0;
		}

		/// <summary>
		/// Count the number of one-bits.
		/// </summary>
		/// <returns>The number of bits set to 1</returns>
		inline constexpr size_t num_one_bits() const noexcept
		{
			return std::popcount<uint64_t>(x);
		}

		/// <summary>
		/// Find the index of the least significant 1-bit.
		/// </summary>
		/// <returns>index.</returns>
		inline std::optional<size_t> scan_lsb() const noexcept
		{
			return ls1b();
		}

		/// <summary>
		/// Find the index of the most significant 1-bit.
		/// </summary>
		/// <returns>index.</returns>
		inline std::optional<size_t> scan_msb() const noexcept
		{
			return ms1b();
		}
#pragma endregion
#pragma region non-const methods
		/// <summary>
		/// Set the i'th bit to 1.
		/// </summary>
		/// <typeparam name="tIdx">Numeric type</typeparam>
		/// <param name="i">Index at which to set the bit to 1</param>
		/// <returns>a reference to the current object.</returns>
		template<typename tIdx>
		inline constexpr bitboard& set_one_at(tIdx i) noexcept
		{
			rt_assert(i >= 0 && i <= 63);
			x |= bitboard_t(1) << i;
			return *this;
		}

		/// <summary>
		/// Toggle the i'th bit.
		/// </summary>
		/// <typeparam name="tIdx">Numeric type.</typeparam>
		/// <param name="i">The index to toggle</param>
		/// <returns>a reference to the current object.</returns>
		template<typename tIdx>
		inline constexpr bitboard& toggle_at(tIdx i) noexcept
		{
			rt_assert(i >= 0 && i <= 63);
			x ^= bitboard_t(1) << i;
			return *this;
		}

		/// <summary>
		/// Find the least significant 1-bit and set it to zero.
		/// If all bits are zero, nothing is changed.
		/// </summary>
		/// <returns>An optional size_t, having a value if *this != 0ULL</returns>
		inline std::optional<size_t> pop_lsb() noexcept
		{
			// scan_lsb guarantees the bit is 1 so we can XOR without worrying about setting a 0 to 1..
			auto i = scan_lsb();
			if (i.has_value())
				x ^= bitboard_t(1) << i.value();
			return i;
		}
#pragma endregion
#pragma region operators
		inline constexpr bool operator==(const bitboard_t& rhs) const
		{
			return x == rhs;
		}
		inline constexpr bool operator==(const bitboard& rhs) const
		{
			return x == rhs.x;
		}
		template<typename T> requires std::is_integral_v<T>
		inline constexpr bitboard& operator<<(const T n)
		{
			x <<= n;
			return *this;
		}
		template<typename T> requires std::is_integral_v<T>
		inline constexpr bitboard& operator>>(const T n)
		{
			x >>= n;
			return *this;
		}
#pragma endregion
	private:
		std::optional<size_t> ms1b() const noexcept;
		std::optional<size_t> ls1b() const noexcept;
	};

	/// <summary>
	/// Print a bitboard to the console.
	/// </summary>
	/// <param name="bb">The bitboard to print</param>
	void print_bitboard(const bitboard& bb);

	inline constexpr bitboard operator*(bitboard l, bitboard r) noexcept
	{
		return l.get_raw() * r.get_raw();
	}
	inline constexpr bitboard operator&(bitboard l, bitboard r) noexcept
	{
		return l.get_raw() & r.get_raw();
	}
	inline constexpr bool operator<(const bitboard& b1, const bitboard& b2) noexcept
	{
		return b1.get_raw() < b2.get_raw();
	}

	/// <summary>
	/// Shift a bitboard in a given direction.
	/// </summary>
	/// <typeparam name="D">Direction to shift</typeparam>
	/// <returns>A new bitboard instance.</returns>
	template<e_direction D>
	inline bitboard shift(const bitboard& bb) noexcept
	{
		switch (D)
		{
		case UP:
			return (bb.get_raw() & ~RANK_MASKS[7]) << 8;
		case DOWN:
			return (bb.get_raw() & ~RANK_MASKS[0]) >> 8;
		case LEFT:
			return (bb.get_raw() & ~FILE_MASKS[0]) >> 1;
		}
		return (bb.get_raw() & ~FILE_MASKS[7]) << 1; // right
	}

	template<e_direction D1, e_direction D2>
	inline bitboard shift(const bitboard& bb) noexcept
	{
		return shift<D1>(shift<D2>(bb));
	}
}