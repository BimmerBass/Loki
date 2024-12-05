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
#include "util/exception.hpp"
#include "move.hpp"

namespace loki::movegen
{
	/// <summary>
	/// Simple "dynamic" collection which is stack-allocated for speed.
	/// </summary>
	class move_list
	{
	public:
		CHILD_EXCEPTION(move_list_error, loki_exception);

		static constexpr size_t max_size = constants::MAX_POSITION_MOVES;
	private:
		std::array<move, max_size> m_collection;
		size_t m_size;

		using cit = std::array<move, max_size>::const_iterator;
	public:
		move_list()
			: m_size{0}, m_collection{}
		{}

		constexpr void push_back(move move)
		{
			if (m_size >= max_size)
				throw_msg<move_list_error>("collection is full");
			m_collection[m_size++] = move;
		}
		constexpr void clear() noexcept { m_size = 0; }

		constexpr const move& operator[](size_t inx) const
		{
			if (inx >= m_size)
				throw_msg<move_list_error>("access to index {} was requested on a move_list of size {}", inx, m_size);
			return m_collection[inx];
		}
		constexpr size_t size() const noexcept { return m_size; }
		constexpr cit begin() const noexcept { return m_collection.begin(); }
		constexpr cit end() const noexcept { return m_collection.begin() + m_size; }

		move_list(const move_list&) = delete;
		move_list& operator=(const move_list&) = delete;
		move_list(move_list&&) = delete;
		move_list& operator=(move_list&&) = delete;

	};
}