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


namespace loki::search
{
	/// <summary>
	/// Make the score of a position relative to its depth instead of relative to root.
	/// </summary>
	inline static eValue value_from_tt(eValue ttVal, eDepth ply)
	{
		if (ttVal > VALUE_MATE)
			return ttVal - (eValue)ply;
		else if (ttVal < -VALUE_MATE)
			return ttVal + (eValue)ply;
		return ttVal;
	}

	/// <summary>
	/// Make the score of a position relative to the root in case of mate scores.
	/// </summary>
	inline  static eValue value_to_tt(eValue val, eDepth ply)
	{
		if (val > VALUE_MATE)
			return val + (eValue)ply;
		else if (val < -VALUE_MATE)
			return val - (eValue)ply;
		return val;
	}

	class hash_entry
	{
		// The data in an entry is arranged to fit in a single 64-bit field.
		//union __entryData
		//{
		//	struct
		//	{
		//		uint16_t move;
		//		int16_t score;
		//		uint16_t depth;
		//		int16_t flag : 8, age : 8;
		//	} data;
		//	uint64_t dataHash;
		//};

		struct __entryData
		{
			uint16_t move;
			int16_t score;
			uint16_t depth;
			int16_t flag : 8, age : 8;
		};
	private:
		__entryData m_data;
		uint64_t m_key;
	public:
		void set(hashkey_t key, uint16_t move, int16_t score, uint16_t depth, uint16_t flag, uint16_t age) noexcept;
		void clear() noexcept;

		auto key() const noexcept { return m_key; }
		auto move() const noexcept { return m_data.move; }
		auto score() const noexcept { return m_data.score; }
		auto depth() const noexcept { return m_data.depth; }
		auto flag() const noexcept { return (int8_t)m_data.flag; }
		auto age() const noexcept { return (int8_t)m_data.age; }

		auto hash() const noexcept
		{
			auto d = reinterpret_cast<const uint64_t*>(&m_data);
			return *d;
		}
	};
}