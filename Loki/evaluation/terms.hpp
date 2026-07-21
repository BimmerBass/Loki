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
#include "position/square.hpp"
#include "defs.hpp"

namespace loki::evaluation
{
	using id_t = size_t;

	enum class evaluation_term
	{
		MATERIAL,
		PSQT,

		TERM_NB
	};
	ENABLE_BASE_OPERATORS_ON(evaluation_term);

	// No default implementation.
	template<evaluation_term T>
	struct term_layout;

	// MATERIAL
	template<>
	struct term_layout<evaluation_term::MATERIAL>
	{
		static constexpr id_t offset = 0;
		static constexpr id_t num_features = static_cast<id_t>(KING);
		
		template<piece P> requires (P < KING)
		inline static constexpr id_t id() noexcept
		{
			auto relative_id = static_cast<size_t>(P);
			return static_cast<id_t>(offset + relative_id);
		}
	};

	template<>
	struct term_layout<evaluation_term::PSQT>
	{
		using prev_term = term_layout<evaluation_term::PSQT - 1>;

		static constexpr id_t offset = prev_term::offset + prev_term::num_features;
		static constexpr id_t num_features = static_cast<id_t>(NUM_PIECES * static_cast<size_t>(position::NUM_SQUARES));

		template<side S, piece P> requires (S < NUM_SIDES && P < NUM_PIECES)
		inline static constexpr id_t id(position::e_square sq) noexcept
		{
			const auto relative_sq = mirror<S>(sq);
			const auto relative_id = static_cast<size_t>(P * position::NUM_SQUARES + relative_sq);
			return static_cast<id_t>(offset + relative_id);
		}

		template<side S>
		inline static constexpr position::e_square mirror(position::e_square sq) noexcept
		{
			if constexpr (S == WHITE)
				return sq;
			else
				return position::vertical_flip(sq);
		}
	};


	/// <summary>
	/// Get the number of features in the entire evaluation function, computed by the terms.
	/// </summary>
	/// <returns>The highest, valid feature ID for the current evaluation function.</returns> 
	constexpr id_t feature_count() noexcept
	{
		constexpr auto term_count = (size_t)evaluation_term::TERM_NB;
		constexpr auto highest_term = evaluation_term(term_count - 1);

		return term_layout<highest_term>::offset + term_layout<highest_term>::num_features;
	}
}