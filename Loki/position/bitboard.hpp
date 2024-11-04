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

#define rt_assert(_cond) \
	if (!std::is_constant_evaluated()){ \
		assert(_cond); \
	}

namespace loki::position
{
	using bitboard_t = uint64_t;

	/// <summary>
	/// NOTE: All index-based operations are zero-indexed and i < 0 || i > 63 is undefined behaviour.
	/// </summary>
	class bitboard final
	{
	private:
		bitboard_t x;
	public:
		inline constexpr bitboard(bitboard_t num) : x(num){}

		inline constexpr bitboard(const bitboard& _other) = delete;
		inline constexpr bitboard(bitboard&& _other) = delete;

#pragma region const methods
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
		inline constexpr std::optional<size_t> scan_lsb() const noexcept
		{
			return ls1b();
		}

		/// <summary>
		/// Find the index of the most significant 1-bit.
		/// </summary>
		/// <returns>index.</returns>
		inline constexpr std::optional<size_t> scan_msb() const noexcept
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
		/// <returns>a reference to the current object.</returns>
		inline constexpr bitboard& pop_lsb() noexcept
		{
			// scan_lsb guarantees the bit is 1 so we can XOR without worrying about setting a 0 to 1..
			auto i = scan_lsb();
			if (i.has_value())
				x ^= bitboard_t(1) << i.value();
			return *this;
		}
#pragma endregion
#pragma region operators
		inline constexpr bool operator==(const bitboard_t& rhs) const
		{
			return x == rhs;
		}
		inline constexpr bool operator==(const bitboard& rhs) const
		{
			return *this == rhs.x;
		}
#pragma endregion
	private:
		constexpr std::optional<size_t> ms1b() const noexcept;
		constexpr std::optional<size_t> ls1b() const noexcept;
	};

}