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


namespace loki::search::util
{
	/// <summary>
	/// Modern implementation of a Triangular PV table (ref: https://www.chessprogramming.org/Triangular_PV-Table)
	/// </summary>
	template<size_t _Depth>
	class tri_pv_table
	{
	public:
		inline static constexpr size_t MyDepth = _Depth;
		inline static constexpr size_t pv_length = MAX_DEPTH - MyDepth;
	private:
		// Next ply's PV array.
		tri_pv_table<MyDepth + 1> m_successor;

		// Current PV array.
		std::array<move_t, pv_length> m_pv;
		size_t m_current_size;

		using const_it_t = std::array<move_t, pv_length>::const_iterator;
	public:
		tri_pv_table() : m_successor{}, m_pv { MOVE_NULL }, m_current_size(0)
		{
		}

		inline void update_pv(eDepth depth, move_t best_move)
		{
			// If we're not the right element to handle this for the current depth, propgagate the call down.
			if (depth == MyDepth)
			{
				// Update best move for this depth, and add the next depth's PV.
				m_pv[0] = best_move;
				m_current_size = 1;

				for (auto pvMove = m_successor.cbegin(); pvMove != m_successor.cend(); pvMove++)
				{
					assert(m_current_size > 0 && m_current_size < m_pv.size());
					m_pv[m_current_size++] = (*pvMove);
				}
			}
			else
				m_successor.update_pv(depth, best_move);
		}

		/// <summary>
		/// Will clear the entire triangular PV table.
		/// </summary>
		inline void clear()
		{
			m_current_size = 0;
			m_pv.fill(MOVE_NULL);
			m_successor.clear();
		}

		/// <summary>
		/// Will just reset the PV for the given depth byt setting size = 0
		/// </summary>
		inline void reset_for_ply(eDepth depth)
		{
			if (depth == MyDepth)
				m_current_size = 0;
			else
				m_successor.reset_for_ply(depth);
		}

		// Simple accessors
		inline const move_t* get() const noexcept { return m_pv.data(); }
		inline size_t size() const noexcept { return m_current_size; }
		inline const_it_t cbegin() const noexcept { return m_pv.cbegin(); }
		inline const_it_t cend() const noexcept { return m_pv.begin() + m_current_size; }

		inline std::pair<size_t, const move_t*> get_for_depth(eDepth d) const
		{
			if (d == MyDepth)
				return std::make_pair(m_current_size, m_pv.data());
			return m_successor.get_for_depth(d);
		}
	};

	template<>
	class tri_pv_table<MAX_DEPTH>
	{
	public:
		inline static constexpr size_t pv_length = 1;
	private:
		// dummy pv-variable so we can return something from cbegin and cend.
		std::array<move_t, pv_length> m_pv;

		using const_it_t = std::array<move_t, pv_length>::const_iterator;
	public:
		tri_pv_table() : m_pv{ MOVE_NULL } {}

		inline void update_pv(eDepth /* unused*/, move_t /* unused*/)
		{}
		inline void clear()
		{}
		inline void reset_for_ply(eDepth /* unused*/)
		{}

		// Simple accessors
		inline const move_t* get() const noexcept { return nullptr; }
		inline size_t size() const noexcept { return 0; }
		inline const_it_t cbegin() const noexcept { return m_pv.cend(); }
		inline const_it_t cend() const noexcept { return m_pv.cend(); }
		inline std::pair<size_t, const move_t*> get_for_depth(eDepth /* unused */) const { return std::make_pair(0, nullptr); }
	};
}