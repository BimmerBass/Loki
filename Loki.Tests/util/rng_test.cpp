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
#include "Loki/position/bitboard.hpp"
#include "Loki/util/rng.hpp"

namespace util_tests
{
	using namespace loki::util;
	using namespace loki::position;

	template<typename T>
	double average_popcount(const std::vector<T>& values)
	{
		size_t total = 0;
		for (auto value : values)
		{
			total += popcount(value);
		}
		return static_cast<double>(total) / static_cast<double>(values.size());
	}

	template<typename T>
	void run_rng_distribution_check()
	{
		const double bit_width = static_cast<double>(sizeof(T) * 8);
		std::vector<T> regular_rands;
		std::vector<T> sparse_rands;
		regular_rands.reserve(10000);
		sparse_rands.reserve(10000);

		for (int i = 0; i < 10000; ++i)
		{
			regular_rands.push_back(rng::instance()->generate<T>());
			sparse_rands.push_back(rng::instance()->generate_sparse<T>());
		}

		const auto avg_regular_popcnt = average_popcount(regular_rands);
		const auto avg_sparse_popcnt = average_popcount(sparse_rands);

		REQUIRE(avg_regular_popcnt >= bit_width * 0.25);
		REQUIRE(avg_regular_popcnt <= bit_width * 0.75);
		REQUIRE(avg_sparse_popcnt >= 0.0);
		REQUIRE(avg_sparse_popcnt <= bit_width * 0.25);
		REQUIRE(avg_sparse_popcnt < avg_regular_popcnt / 2.0);
	}

	TEMPLATE_TEST_CASE("rng_sparse_values_have_lower_popcount",
		"[util][rng]",
		uint64_t, uint32_t, uint16_t, uint8_t)
	{
		run_rng_distribution_check<TestType>();
	}
}
