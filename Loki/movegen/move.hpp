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
#include <cstdint>
#include "defs.hpp"
#include "position/square.hpp"

namespace loki::movegen
{
	enum move_type : uint8_t
	{
		NORMAL = 0x0,
		CASTLING = 0x1,
		ENPASSANT = 0x2,
		PROMOTION = 0x3
	};

	using move_t = uint16_t;

	class move final
	{
	private:
		move_t m_move;

	public:
		move() noexcept = default;
		inline constexpr move(move_t m) : m_move{ m }
		{ }
		inline constexpr move(position::e_square from, position::e_square to, move_type type, piece promotion_piece) noexcept
			: m_move{0}
		{
			this->from(from);
			this->to(to);
			this->type(type);
			this->promotion_piece(promotion_piece);
		}
		move(const move& src) noexcept : m_move{ src.m_move } {}

#pragma region setters
		inline constexpr void from(position::e_square sq) noexcept
		{
			m_move = nmasked_or(sq, 0xFC00, 10);
		}
		inline constexpr void to(position::e_square sq) noexcept
		{
			m_move = nmasked_or(sq, 0x03F0, 4);
		}
		inline constexpr void type(move_type t) noexcept
		{
			m_move = nmasked_or(t, 0x000C, 2);
		}
		inline constexpr void promotion_piece(piece p) noexcept
		{
			m_move = nmasked_or((int)p - 1, 0x0003, 0);
		}
#pragma endregion
#pragma region getters
		inline constexpr position::e_square from() noexcept
		{
			return get<position::e_square>(0x003F, 10);
		}
		inline constexpr position::e_square to() noexcept
		{
			return get<position::e_square>(0x003F, 4);
		}
		inline constexpr move_type type() noexcept
		{
			return get<move_type>(0x0003, 2);
		}
		inline constexpr piece promotion_piece() noexcept
		{
			return get<piece>(0x0003, 0) + 1;
		}
#pragma endregion

		template<typename T> requires std::convertible_to<T, move_t>
		inline constexpr bool operator==(const T& rhs) const noexcept
		{
			return m_move == static_cast<move_t>(rhs);
		}
		inline constexpr bool operator==(const move& rhs) const noexcept
		{
			return m_move == rhs.m_move;
		}

	private:

		template<typename T>
		inline constexpr T get(uint16_t mask, size_t shift) noexcept
		{
			return static_cast<T>((m_move >> shift) & mask);
		}

		template<typename T>
		inline constexpr move_t nmasked_or(T value, uint16_t mask, size_t shift) const noexcept
		{
			return (m_move & ~mask) | ((uint16_t)value << shift);
		}
	};
}