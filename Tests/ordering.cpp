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
	TEST_CLASS(move_sorter_tests), public fen_reader
	{
	private:
		movegen::magics::slider_generator_t m_sliderGen;
	public:
		move_sorter_tests() : fen_reader("data/test_fens.fen")
		{
			m_sliderGen = std::make_shared<movegen::magics::slider_generator>();
		}

		TEST_METHOD(move_sorter_no_scoring)
		{
			move_sorter(false);
		}

		TEST_METHOD(move_sorter_with_scoring)
		{
			move_sorter(true);
		}

	private:
		void move_sorter(bool scoring)
		{
			LOG("Starting move_sorter test {}", scoring ? "with scoring" : "without scoring");
			read_fen_file();
			auto pos = position::position::create_position(
				std::make_shared<position::game_state>(),
				m_sliderGen);
			std::unordered_set<move_t> moves_generated;
			for (auto& fen : m_fens)
			{
				eValue score = -VALUE_INF, best_score = VALUE_INF;
				LOG("Starting move_sorter test for position:\t{}", fen);
				(*pos) << fen;
				ordering::move_sorter sorter(pos, false, scoring);
				move_list_t moves(pos->generate_moves()); /* Copy the moves, so we're can compare with results from move_sorter */

				move_t move;
				while (move = sorter.get_next(&score))
				{
					Assert::IsTrue(score <= best_score, LK_WCHAR_STR("get_next returned higher scored move than the best one so far. score = '{}' while best_score = '{}'", (int)score, (int)best_score));
					Assert::IsTrue(moves_generated.find(move) == moves_generated.end(), L"'sorter' fetched same move twice!");
					best_score = score;
					moves_generated.insert(move);
				}

				// Now that we have fetched all moves, make sure the list is precisely the same length as the result from pos->generate_moves, and that each move fetched exists in the move_list.
				Assert::IsTrue(moves_generated.size() == moves.size(), L"List from 'sorter' didn't match the one originally generated!");

				for (auto& m : moves)
					Assert::IsTrue(moves_generated.find(m.move) != moves_generated.end(), L"'moves' contained an entry that didn't exist in 'sorter'!");

				// Clear before next position.
				moves_generated.clear();
			}
			LOG("move_sorter passed test");
		}
	};
}
