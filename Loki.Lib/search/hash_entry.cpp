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
#include "loki.pch.hpp"

namespace loki::search
{
	/// <summary>
	/// Set the data in the entry.
	/// </summary>
	void hash_entry::set(hashkey_t key, uint16_t move, int16_t score, uint16_t depth, uint8_t flag, uint8_t age) noexcept
	{
		m_data.move = move;
		m_data.score = score;
		m_data.depth = depth;
		m_data.flag = flag;
		m_data.age = age;

		m_key = key ^ hash();
	}

	/// <summary>
	/// Reset the data in the entry
	/// </summary>
	void hash_entry::clear() noexcept
	{
		m_key = 0;
		m_data.move = 0;
		m_data.score = 0;
		m_data.depth = 0;
		m_data.flag = 0;
		m_data.age = 0;
	}
}