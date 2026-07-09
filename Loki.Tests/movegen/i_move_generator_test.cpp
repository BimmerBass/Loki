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
#include "Loki/movegen/i_move_generator.hpp"
#include "Loki/movegen/magics/magic_index.hpp"

namespace movegen_tests
{
	using namespace loki;
	using namespace loki::movegen;
	using namespace loki::position;

	class position_view_fake final : public position::i_position_view
	{
	public:
		explicit position_view_fake(const position::game_state* state) : m_state(state) {}

		bitboard_t piece_bb(side, piece) const noexcept override { return 0ULL; }
		bitboard_t all_pieces(side) const noexcept override { return 0ULL; }
		bitboard_t all_pieces() const noexcept override { return 0ULL; }
		e_square king_square(side) const noexcept override { return NO_SQ; }
		const position::game_state* game_state() const noexcept override { return m_state; }

	private:
		const position::game_state* m_state;
	};

	class mock_move_generator final : public i_move_generator<position_view_fake>
	{
	public:
		MAKE_CONST_MOCK5(generate_internal, size_t(const position_view_fake*, move_list*, side, move_type, piece), override);
		MAKE_CONST_MOCK4(attackers_to_internal, bitboard_t(const position_view_fake*, e_square, side, piece), override);
		MAKE_CONST_MOCK0(rook_index, magics::magic_index_t(), noexcept override);
		MAKE_CONST_MOCK0(bishop_index, magics::magic_index_t(), noexcept override);
	};

	TEST_CASE("i_move_generator wrapper templates forward to the virtual implementation", "[movegen][interface][trompeloeil]")
	{
		game_state state;
		state.side_to_move = BLACK;
		position_view_fake view(&state);

		move_list moves;
		mock_move_generator generator;

		REQUIRE_CALL(generator, generate_internal(&view, &moves, BLACK, ALL, NUM_PIECES)).RETURN(17);
		REQUIRE(generator.generate(&view, &moves) == 17);

		REQUIRE_CALL(generator, generate_internal(&view, &moves, WHITE, ACTIVE, KNIGHT)).RETURN(4);
		REQUIRE(generator.generate<WHITE, ACTIVE, KNIGHT>(&view, &moves) == 4);
	}

	TEST_CASE("i_move_generator attack wrapper forwards to the virtual implementation", "[movegen][interface][trompeloeil]")
	{
		position_view_fake view(nullptr);

		mock_move_generator generator;
		REQUIRE_CALL(generator, attackers_to_internal(&view, E4, WHITE, ROOK)).RETURN(0x1234ULL);
		REQUIRE(generator.attackers_to<ROOK>(&view, E4, WHITE) == 0x1234ULL);
	}
}
