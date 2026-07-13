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
#include "terms.hpp"
#include <vector>

namespace loki::evaluation
{
	template<typename T, side S>
	concept side_feature_sink =
    	requires(T& sink, id_t id, size_t count)
    	{
        	{ sink.template add_feature<S>(id, count) } -> std::same_as<void>;
    	};

	template<typename T>
	concept feature_sink =
    	requires
    	{
        	{ T::enabled } -> std::convertible_to<bool>;
    	}
    	&& side_feature_sink<T, WHITE>
    	&& side_feature_sink<T, BLACK>;

	struct traced_feature
	{
		id_t id;
		std::array<size_t, NUM_SIDES> counts = {};

		template<side S> requires (S < NUM_SIDES)
		constexpr size_t& count() noexcept
		{
			return counts[static_cast<size_t>(S)];
		}

		template<side S> requires (S < NUM_SIDES)
		constexpr const size_t& count() const noexcept
		{
			return counts[static_cast<size_t>(S)];
		}
	};

	struct null_trace
	{
		constexpr static bool enabled = false;

		template<side S> requires (S < NUM_SIDES)
		inline void add_feature(id_t id, size_t count) noexcept {}
	};

	struct feature_trace
	{
		feature_trace() : features(feature_count())
		{
			for (id_t id = 0; id < feature_count(); ++id)
			{
				features[id].id = id;
			}
		}

		std::vector<traced_feature> features;
		side relative_to;
		score_t score;

		constexpr static bool enabled = true;

		template<side S> requires (S < NUM_SIDES)
		inline void add_feature(id_t id, size_t count) noexcept
		{
			features[id].count<S>() = count;
		}
	};
}