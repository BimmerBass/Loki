#include "pch.hpp"
#include "Loki/movegen/move.hpp"

using namespace loki::movegen;
using namespace loki::position;
using namespace loki;

namespace move_tests
{
	TEST(move_test, test_all_zero)
	{
		move_t intmove = 0;
		move smove(intmove);

		EXPECT_EQ(smove.from(), A1);
		EXPECT_EQ(smove.to(), A1);
		EXPECT_EQ(smove.type(), NORMAL);
		EXPECT_EQ(smove.promotion_piece(), KNIGHT);
	}
	
	TEST(move_test, test_all_rand)
	{
		move_t intmove = ((uint16_t)32 << 10) | ((uint16_t)31 << 4) | (CASTLING << 2) | (ROOK-1);
		move smove(intmove);

		EXPECT_EQ(smove.from(), A5);
		EXPECT_EQ(smove.to(), H4);
		EXPECT_EQ(smove.type(), CASTLING);
		EXPECT_EQ(smove.promotion_piece(), ROOK);
	}

	TEST(move_test, test_all_full)
	{
		move_t intmove = ((uint16_t)63 << 10) | ((uint16_t)63 << 4) | (PROMOTION << 2) | (QUEEN-1);
		move smove(intmove);

		EXPECT_EQ(smove.from(), H8);
		EXPECT_EQ(smove.to(), H8);
		EXPECT_EQ(smove.type(), PROMOTION);
		EXPECT_EQ(smove.promotion_piece(), QUEEN);
	}

	TEST(move_test, test_set_from)
	{
		move m(0);
		EXPECT_EQ(m.from(), A1);
		m.from(H8);
		EXPECT_EQ(m.from(), H8);
	}
	TEST(move_test, test_set_to)
	{
		move m(0);
		EXPECT_EQ(m.to(), A1);
		m.to(H8);
		EXPECT_EQ(m.to(), H8);
	}
	TEST(move_test, test_set_type)
	{
		move m(0);
		EXPECT_EQ(m.type(), NORMAL);
		m.type(CASTLING);
		EXPECT_EQ(m.type(), CASTLING);
	}
	TEST(move_test, test_set_promotion_piece)
	{
		move m(0);
		EXPECT_EQ(m.promotion_piece(), KNIGHT);
		m.promotion_piece(ROOK);
		EXPECT_EQ(m.promotion_piece(), ROOK);
	}
}