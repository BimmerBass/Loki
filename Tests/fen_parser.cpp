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
	TEST_CLASS(fen_parser_tests)
	{
	public:
		TEST_METHOD(test_fen_parser) {
			read_fen_file();
			loki::position::game_state_t pos = std::make_shared<loki::position::game_state>();
			std::string generated;
			size_t i = 0;

			for (auto fen : m_fens) {
				*pos << fen;
				*pos >> generated;

				Assert::AreEqual(fen, generated);
				i++;
			}

			std::cout << "Tested " << i << " FENs" << std::endl;
		}


	private:
		std::vector<std::string> m_fens;

		void read_fen_file() {
			auto fen_file = std::ifstream("data/test_fens.fen");
			Assert::IsTrue(fen_file.is_open());
			std::string current_fen;

			while (std::getline(fen_file, current_fen)) {
				m_fens.push_back(current_fen);
			}
			fen_file.close();
		}
	};
}
