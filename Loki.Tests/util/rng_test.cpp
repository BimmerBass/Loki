#include "pch.hpp"
#include "Loki/position/bitboard.hpp"
#include "Loki/util/rng.hpp"

namespace util_tests
{
	using namespace loki::util;
	using namespace loki::position;

	template<typename T>
	double average_popcount(const std::vector<T>& v)
	{
		size_t total = 0;
		for (auto x : v)
			total += popcount(x);
		return static_cast<double>(total) / static_cast<double>(v.size());
	}

	template<typename T>
	class rng_test : public ::testing::Test
	{
	public:
	};
	TYPED_TEST_SUITE_P(rng_test);

	TYPED_TEST_P(rng_test, test_sparse_lower_popcount)
	{
		// On average, generate() will have half of the bits set to 1
		// => generate() & generate() will have a fourth of the bits set
		// => generate() & generate() & generate() will have 1/8 set.
		// we will just check that the popcount is strictly lower than 1/4
		double bit_width = sizeof(TypeParam) * 8;
		std::vector<TypeParam> regular_rands;
		std::vector<TypeParam> sparse_rands;
		for (int i = 0; i < 10000; i++)
		{
			regular_rands.push_back(rng::instance()->generate<TypeParam>());
			sparse_rands.push_back(rng::instance()->generate_sparse<TypeParam>());
		}
		double avg_regular_popcnt = average_popcount<TypeParam>(regular_rands);
		double avg_sparse_popcnt = average_popcount<TypeParam>(sparse_rands);

		EXPECT_THAT(avg_regular_popcnt, IsInRange(bit_width * 1.0 / 4.0, bit_width * 3.0 / 4.0))
			<< "Regular popcount is not between 1/4 and 3/4 (i.e. not centered around 1/2)";
		EXPECT_THAT(avg_sparse_popcnt, IsInRange(bit_width * 0.0, bit_width * 1 / 4.0))
			<< "Sparse popcount is not between 0 and 1/4 (i.e. not centered around 1/8)";
		EXPECT_LT(avg_sparse_popcnt, avg_regular_popcnt / 2.0)
			<< "Sparse popcount isn't significantly lower than regular.";
	}
	REGISTER_TYPED_TEST_SUITE_P(rng_test,
		test_sparse_lower_popcount);
	using rng_types = ::testing::Types<
		uint64_t, uint32_t, uint16_t, uint8_t>;
	INSTANTIATE_TYPED_TEST_SUITE_P(rng, rng_test, rng_types);
}