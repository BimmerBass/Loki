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
	/// Initialize the table with sizeMB size (megabytes).
	/// </summary>
	transposition_table::transposition_table(size_t sizeMB) : m_currentAge(0)
	{
		resize(sizeMB);
	}

	/// <summary>
	/// Get the size of the table in megabytes.
	/// </summary>
	size_t transposition_table::size() const noexcept
	{
		return ((m_entryCount * sizeof(hash_entry)) >> 20) + 1;
	}

	/// <summary>
	/// Resize the table to sizeMB megabytes
	/// </summary>
	void transposition_table::resize(size_t sizeMB)
	{
		auto upperEntryBound = from_mb(sizeMB) / sizeof(hash_entry);
		m_entryCount = nearest_pow2(upperEntryBound);
		m_slotCount = m_entryCount / slot_size;
		m_table = std::make_unique<slot_t[]>(m_slotCount);
		
		clear();
	}

	/// <summary>
	/// Clear all entries in the table.
	/// </summary>
	void transposition_table::clear()
	{
		m_currentAge = 0;
		for (auto i = 0; i < m_slotCount; i++)
		{
			for (auto entryInx = 0; entryInx < slot_size; entryInx++)
				m_table[i].at(entryInx).clear();
		}
	}

	/// <summary>
	/// Increment the age, making sure to stay under max_age
	/// </summary>
	void transposition_table::increment_age() noexcept
	{
		if (m_currentAge < max_age)
			m_currentAge++;
	}

	/// <summary>
	/// Probe the table for entries matching the key.
	/// If none are found, return false
	/// </summary>
	bool transposition_table::probe(const hashkey_t& key, const eDepth& ply, move_t& move, eValue& score, eDepth& depth, ttFlag& flag) const
	{
		if (m_slotCount <= 0)
			return false;
		auto potentialMatch = &m_table[key & (m_slotCount - 1)];

		// Always attempt to match on the depth-preferred bucket first.
		for (auto entryInx = 0; entryInx < slot_size; entryInx++)
		{
			auto& current = potentialMatch->at(entryInx);
			if (current.key() == (key ^ current.hash()))
			{
				move = (move_t)current.move();
				score = value_from_tt((eValue)current.score(), ply);
				depth = (eDepth)current.depth();
				flag = (ttFlag)current.flag();
				return true;
			}
		}
		return false;
	}

	/// <summary>
	/// Store an entry in the table.
	/// </summary>
	void transposition_table::store(const hashkey_t& key, const eDepth& ply, move_t move, eValue score, eDepth depth, ttFlag flag)
	{
		if (m_slotCount <= 0)
			return;
		auto slot = &m_table[key & (m_slotCount - 1)];
		//entry->set(key, move, value_to_tt(score, ply), depth, flag, 0);

		// 1. Check if we can replace depth/age preferred entry.
		if (depth >= slot->at(0).depth() - 1 || m_currentAge > slot->at(0).age())
		{
			auto age = m_currentAge;
			if (m_currentAge >= max_age)
				age = max_age - (key & 1); // Subtract random number so we allow replacements even when we exceed the age limit.

			slot->at(0).set(key, move, value_to_tt(score, ply), depth, flag, static_cast<uint8_t>(age));
		}
		else
		{
			slot->at(1).set(key, move, value_to_tt(score, ply), depth, flag, m_currentAge);
		}
	}
}