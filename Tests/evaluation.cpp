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
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace loki::tests
{
	TEST_CLASS(evaluator_tests), public fen_reader
	{
	private:
		position::position_t m_pos;
		std::unique_ptr<evaluation::evaluator> m_eval;

	public:
		evaluator_tests() : fen_reader("data/test_fens.fen")
		{
			m_pos = position::position::create_position(
				std::make_shared<position::game_state>(),
				std::make_shared<movegen::magics::slider_generator>());
			auto params = evaluation::make_params<evaluation::hardcoded_params>();
			m_eval = std::make_unique<evaluation::evaluator>(m_pos, params);
		}

		TEST_METHOD(eval_balance)
		{
			read_fen_file();

			for (const auto& fen : m_fens)
			{
				*m_pos << fen;
				auto original = m_eval->score_position();

				*m_pos << textutil::flip_fen(fen);
				auto flipped = m_eval->score_position();

				Assert::IsTrue(original == flipped);
			}
		}
	};
}