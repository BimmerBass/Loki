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

#include "pch.hpp"
#include "Loki/evaluation/terms.hpp"

namespace evaluation_tests
{
	using namespace loki::evaluation;

	struct term_layout_data
	{
		id_t offset;
		id_t num_features;
	};

	template<size_t... Is>
	constexpr auto make_term_layouts(std::index_sequence<Is...>)
	{
		return std::array{
			term_layout_data{
				term_layout<static_cast<evaluation_term>(Is)>::offset,
				term_layout<static_cast<evaluation_term>(Is)>::num_features
			}...
		};
	}

	constexpr auto all_term_layouts()
	{
		constexpr auto term_count = static_cast<size_t>(evaluation_term::TERM_NB);

		return make_term_layouts(std::make_index_sequence<term_count>{});
	}

	TEST_CASE("evaluation term layouts are contiguous", "[evaluation][terms]")
	{
		constexpr auto layouts = all_term_layouts();

		REQUIRE_FALSE(layouts.empty());
		REQUIRE(layouts.front().offset == 0);

		for (size_t i = 1; i < layouts.size(); ++i)
		{
			const auto& previous = layouts[i - 1];
			const auto& current = layouts[i];

			REQUIRE(current.offset == previous.offset + previous.num_features);
		}
	}

	TEST_CASE("evaluation term layouts cover feature_count", "[evaluation][terms]")
	{
		constexpr auto layouts = all_term_layouts();
		const auto& highest = layouts.back();

		REQUIRE(highest.offset + highest.num_features == feature_count());
	}
}
