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
#include "generation/sliding_generator.hpp"
#include "position/bitboard.hpp"
#include "position/square.hpp"
#include "util/exception.hpp"

#include <numeric>

namespace loki::movegen::magics
{
	class magic_index;
	using magic_index_t = std::shared_ptr<magic_index>;


	/// <summary>
	/// Interface for a magic attack tablebase.
	/// Manages pointers into a continuous area of memory.
	/// </summary>
	class magic_index
	{
	public:
		CHILD_EXCEPTION(index_exception, loki_exception);
	protected:
		struct M
		{
			position::bitboard_t* attacks;
			position::bitboard_t magic;
			position::bitboard_t mask;
			position::bitboard_t shift;
		};

	private:
		bool m_initialized;
		position::bitboard_t* m_index = nullptr;
		std::array<M, position::NUM_SQUARES> m_blocks;

	protected:
		magic_index()
			: m_index{ nullptr }, m_blocks{}, m_initialized{false}
		{}

		void initialize(const position::bitboard_t* magics, const generation::sliding_generator* gen);
	public:
		virtual ~magic_index()
		{
			if (m_index != nullptr)
				delete[] m_index;
		}

		/// <summary>
		/// Get the attack bitboard of a square and an occupancy mask.
		/// </summary>
		/// <param name="sq">The square</param>
		/// <param name="occupancy">Mask with occupied squares set to 1</param>
		/// <returns>A bitboard object with the attacks the piece can make.</returns>
		position::bitboard_t attacks(position::e_square sq, position::bitboard_t occupancy) const;

		magic_index(const magic_index&) = delete;
		magic_index(magic_index&&) = delete;
	private:
		size_t calculate_index(position::e_square sq, position::bitboard_t occupancy) const;
		void prefill_blocks(const position::bitboard_t* magics, const generation::sliding_generator* gen);
		void partition_blocks(size_t index_size);
	};
}
