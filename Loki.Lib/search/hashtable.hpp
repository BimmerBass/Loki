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
	/// transposition_table holds a two-bucket hash table.
	/// The first bucket features a depth/age based replacement scheme, whereas the other one is always replace.
	/// </summary>
	class transposition_table
	{
		EXCEPTION_CLASS(e_transpositionTable, e_lokiError);
	private:
		inline static constexpr size_t max_age = static_cast<size_t>(std::numeric_limits<uint8_t>::max());
		inline static constexpr size_t slot_size = 2;
		using slot_t = std::array<hash_entry, slot_size>;
		std::unique_ptr<slot_t[]> m_table;
		size_t m_entryCount, m_slotCount, m_currentAge;
	public:
		transposition_table(size_t sizeMB = default_size);

		size_t size() const noexcept;
		void resize(size_t sizeMB);

		bool probe(const hashkey_t& key, const eDepth& ply, move_t& move, eValue& score, eDepth& depth, ttFlag& flag) const;
		void store(const hashkey_t& key, const eDepth& ply, move_t move, eValue score, eDepth depth, ttFlag flag);
		void clear();
		void increment_age() noexcept;

		inline static constexpr size_t min_size_mb = 1;
		inline static constexpr size_t max_size_mb = 8192;
		inline static constexpr size_t default_size = 16;
	private:
		/// <summary>
		/// Convert megabytes to bytes.
		/// </summary>
		inline static auto from_mb(size_t s) noexcept { return (s << 20); }

		/// <summary>
		/// Return the nearest power of two less than or equal to maxSize
		/// </summary>
		inline static auto nearest_pow2(size_t maxSize) noexcept
		{
			uint64_t x = 1;

			while (x <= maxSize)
				x <<= 1;
			x >>= 1;
			return x;
		}
	};

}