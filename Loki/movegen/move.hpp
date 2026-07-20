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
#include <map>
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

	enum move_t : uint16_t
	{
		MOVE_NULL = 0
	};
	ENABLE_COMP_OPERATORS_ON(move_t);

	// Encodes a move_t as the lower 16 bits, an activity flag and the remaining 15 bits as a signed score.
	enum scored_move_t : uint32_t
	{
	};
	using move_score_t = int16_t;

	/// <summary>Wrapper class for move_t with utility get/set functions</summary>
	class move final
	{
	private:
		scored_move_t m_move;

	public:
		move() noexcept : m_move{ MOVE_NULL }
		{}
		constexpr move(move_t m) : m_move{ m }
		{}
		constexpr move(
			position::e_square from,
			position::e_square to,
			move_attr type,
			piece promotion_piece,
			bool isactive,
			move_score_t score = 0)
			: m_move{ 0 }
		{
			this->from(from);
			this->to(to);
			this->type(type);
			this->promotion_piece(promotion_piece);
			this->active(isactive);
			this->score(score);
		}
		constexpr move(size_t from, size_t to, bool isactive)
			: move(static_cast<position::e_square>(from), static_cast<position::e_square>(to), isactive)
		{

		}
		constexpr move(position::e_square from, position::e_square to, bool isactive)
			: move(from, to, NORMAL, KNIGHT, isactive)
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
		constexpr void active(bool isactive) noexcept
		{
			uint32_t value = isactive ? 0x1 : 0x0;
			m_move = nmasked_or(value, 0x10000, 16);
		}
		constexpr void score(move_score_t score)
		{
			if (score < -0x4000 || score > 0x3FFF)
				throw loki_exception("score value out of range");
			m_move = nmasked_or(score, 0xFFFE0000, 17);
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
		constexpr bool is_active() const noexcept
		{
			return get<bool>(0x1, 16);
		}
		constexpr move_score_t score() const noexcept
		{
			const auto encoded_score = get<uint32_t>(0x7FFF, 17);
			return encoded_score & 0x4000
				? static_cast<move_score_t>(static_cast<int32_t>(encoded_score) - 0x8000)
				: static_cast<move_score_t>(encoded_score);
		}

		constexpr move_t get_move() const noexcept
		{
			return static_cast<move_t>(m_move & 0xFFFF);
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

		std::string to_string() const
		{
			constexpr std::array<char, 4> promotion_pieces = { 'n', 'b', 'r', 'q' };

			position::square from_sq = from();
			position::square to_sq = to();
			auto move_str = from_sq.to_algebraic() + to_sq.to_algebraic();
			if (type() == PROMOTION)
			{
				move_str += promotion_pieces[promotion_piece() - 1];
			}
			return move_str;
		}
	private:

		template<typename T>
		constexpr T get(uint32_t mask, size_t shift) const noexcept
		{
			return static_cast<T>((m_move >> shift) & mask);
		}

		template<typename T>
		constexpr scored_move_t nmasked_or(T value, uint32_t mask, size_t shift) const noexcept
		{
			const auto shifted_value = static_cast<uint32_t>(value) << shift;
			return static_cast<scored_move_t>((m_move & ~mask) | (shifted_value & mask));
		}
	};
}
