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
		ACTIVE, // Active moves. captures, promotions, en-passant
		QUIET, // Non-active moves.
		ALL
	};
	enum move_attr : uint8_t
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
		constexpr move(move_t m) : m_move{ m }
		{ }
		constexpr move(position::e_square from, position::e_square to, move_attr type, piece promotion_piece) noexcept
			: m_move{0}
		{
			this->from(from);
			this->to(to);
			this->type(type);
			this->promotion_piece(promotion_piece);
		}
		constexpr move(position::e_square from, position::e_square to)
			: move(from, to, NORMAL, KNIGHT)
		{ }
		move(const move& src) noexcept : m_move{ src.m_move } {}

#pragma region setters
		constexpr void from(position::e_square sq) noexcept
		{
			m_move = nmasked_or(sq, 0xFC00, 10);
		}
		constexpr void to(position::e_square sq) noexcept
		{
			m_move = nmasked_or(sq, 0x03F0, 4);
		}
		constexpr void type(move_attr t) noexcept
		{
			m_move = nmasked_or(t, 0x000C, 2);
		}
		constexpr void promotion_piece(piece p) noexcept
		{
			m_move = nmasked_or((int)p - 1, 0x0003, 0);
		}
#pragma endregion
#pragma region getters
		constexpr position::e_square from() const noexcept
		{
			return get<position::e_square>(0x003F, 10);
		}
		constexpr position::e_square to() const noexcept
		{
			return get<position::e_square>(0x003F, 4);
		}
		constexpr move_attr type() const noexcept
		{
			return get<move_attr>(0x0003, 2);
		}
		constexpr piece promotion_piece() const noexcept
		{
			return get<piece>(0x0003, 0) + 1;
		}

		constexpr move_t get_raw() const noexcept
		{
			return m_move;
		}
#pragma endregion

		template<typename T> requires std::convertible_to<T, move_t>
		constexpr bool operator==(const T& rhs) const noexcept
		{
			return m_move == static_cast<move_t>(rhs);
		}
		constexpr bool operator==(const move& rhs) const noexcept
		{
			return m_move == rhs.m_move;
		}

	private:

		template<typename T>
		constexpr T get(uint16_t mask, size_t shift) const noexcept
		{
			return static_cast<T>((m_move >> shift) & mask);
		}

		template<typename T>
		constexpr move_t nmasked_or(T value, uint16_t mask, size_t shift) const noexcept
		{
			return (m_move & ~mask) | ((uint16_t)value << shift);
		}
	};
}