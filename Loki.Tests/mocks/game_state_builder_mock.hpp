#pragma once
#include <gmock/gmock.h>
#include "Loki/position/io/game_state_builder.hpp"

namespace position_tests
{
	class game_state_builder_mock : public loki::position::io::game_state_builder
	{
	public:
		MOCK_METHOD(base_builder&, piece_placements, (), (override));
		MOCK_METHOD(base_builder&, side_to_move, (), (override));
		MOCK_METHOD(base_builder&, castling_ability, (), (override));
		MOCK_METHOD(base_builder&, en_passant_square, (), (override));
		MOCK_METHOD(base_builder&, halfmove_clock, (), (override));
		MOCK_METHOD(base_builder&, fullmove_clock, (), (override));
	};
}