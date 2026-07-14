// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once
#include <array>
#include "defs.hpp"
#include "movegen/move.hpp"

namespace loki::search
{
	/// <summary>
	/// Implementation of a triangular PV table as described in (https://www.chessprogramming.org/Triangular_PV-Table)
	/// </summary>
	template<ply_t P = ROOT_PLY> requires (P <= constants::MAX_DEPTH)
	class pv_table
	{
	private:
		inline static constexpr size_t current_ply_pv_length = constants::MAX_DEPTH - P;

		pv_table<P + 1> _successor;
		std::array<movegen::move, current_ply_pv_length> _pv;
		size_t _size;

	public:
		pv_table()
			: _successor{}, _pv{}, _size{ 0 }
		{}

		inline void update_pv(ply_t ply, movegen::move bestmove) noexcept
		{
			if (ply == P)
			{
				// keep best move and copy successor's PV.
				_size = 0;
				_pv[_size++] = bestmove;

				for (const auto& pvMove : _successor)
				{
					if (_size >= _pv.size())
						throw_msg<loki_exception>("pv length of {} exceeded.", _pv.size());
					_pv[_size++] = pvMove;
				}
			}
			else
			{
				_successor.update_pv(ply, bestmove);
			}
		}

		inline void clear() noexcept
		{
			_size = 0;
			_pv.fill(movegen::move{});
			_successor.clear();
		}

		inline void reset_for_ply(depth_t ply)
		{
			if (ply == P)
			{
				_size = 0;
			}
			else
			{
				_successor.reset_for_ply(ply);
			}
		}

		std::vector<movegen::move> get_pv(ply_t ply) const noexcept
		{
			if (ply == P)
			{
				return std::vector<movegen::move>(_pv.begin(), _pv.begin() + _size);
			}
			return _successor.get_pv(ply);
		}

		inline auto begin() noexcept { return _pv.begin(); }
		inline auto end() noexcept { return _pv.begin() + _size; }
	};

	template<>
	class pv_table<(ply_t)constants::MAX_DEPTH>
	{
	private:
		inline static constexpr size_t pv_length_for_ply = 1;

		std::array<movegen::move, pv_length_for_ply> _pv;

	public:
		inline void update_pv(ply_t, movegen::move) noexcept {}
		inline void clear() noexcept {}
		inline void reset_for_ply(depth_t) {}

		inline auto begin() noexcept { return _pv.begin(); }
		inline auto end() noexcept { return _pv.begin(); }

		std::vector<movegen::move> get_pv(ply_t) const noexcept { return std::vector<movegen::move>{}; }
	};
}
