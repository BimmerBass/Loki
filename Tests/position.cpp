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
	TEST_CLASS(position_tests)
	{
	public:
		/// <summary>
		/// Make sure the internal move history of position can actually make MAX_GAME_MOVES without throwing an exception or accessing invalid memory
		/// </summary>
		TEST_METHOD(max_game_plies)
		{
			auto stack = std::make_unique<move_stack<MAX_GAME_MOVES>>();
			try
			{
				size_t ply = 0;
				while (ply != MAX_GAME_MOVES)
				{
					stack->insert((move_t)0, std::make_tuple((ePiece)0, (ePiece)0, 0, 0, (eSquare)0, 0));
					ply++;
				}
			}
			catch (const e_lokiError& e)
			{
				Assert::IsTrue(false, LK_WCHAR_STR("Exception thrown: {}", e.what()));
			}
			// Now try to add one more element and make sure it throws
			try
			{
				stack->insert((move_t)0, std::make_tuple((ePiece)0, (ePiece)0, 0, 0, (eSquare)0, 0));
			}
			catch (const e_lokiError& e)
			{
				LOG("insert method threw exception at correct size");
				return;
			}
			Assert::IsTrue(false, L"insert method didn't throw when it exceeded max size");
		}
	};
}