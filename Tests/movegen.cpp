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
#include "pch.h"

namespace loki::tests
{
	TEST_CLASS(movegen_tests), public fen_reader
	{
	private:
		movegen::magics::slider_generator_t m_sliderGen;
	public:
		movegen_tests() : fen_reader("data/test_fens.fen")
		{
			m_sliderGen = std::make_shared<movegen::magics::slider_generator>();
		}

		/// <summary>
		/// Tests that all actives generated are in the list of all pseudo-legal moves, and that none are left out.
		/// </summary>
		TEST_METHOD(generate_actives)
		{
			LOG("Begginning test to ensure proper generation of active moves.");
			read_fen_file();

			auto pos = position::position::create_position(
				std::make_shared<position::game_state>(),
				m_sliderGen);

			for (auto& fen : m_fens)
			{
				(*pos) << fen;
				auto actives = move_list_t(pos->generate_moves<movegen::ACTIVES>()); // Ensure proper copy.
				auto all = pos->generate_moves();

				// First check that all active moves generated exist in the list of total moves
				for (auto& activeMove : actives)
					Assert::IsTrue(all.contains(activeMove.move), LK_WCHAR_STR("Active move '{}' did not exist in the list of all pseudo-legal moves!", to_string(activeMove.move)));
				// Next, check that no active moves are missing, by ensuring ALL actives in the list of total moves exist in the actives list.
				for (auto& genericMove : all)
				{
					if (is_active(pos, genericMove.move))
						Assert::IsTrue(actives.contains(genericMove.move),
							LK_WCHAR_STR("Active move '{}' did not exist in the list of all active moves!", to_string(genericMove.move)));
				}

				LOG("Actives generated correctly for FEN:\t\t'{}'", fen);
			}
		}

	private:
		bool is_active(position::position_t& pos, move_t move) const
		{
			auto spc = special(move);
			// Active moves are defined as changing the material on the board. Thus they are en-passant, promotions or captures.
			if (spc == ENPASSANT || spc == PROMOTION || pos->piece_on_sq(to_sq(move), !pos->side_to_move()) != PIECE_NB)
				return true;
			return false;
		}
	};
}