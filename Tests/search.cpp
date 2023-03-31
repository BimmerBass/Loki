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
	TEST_CLASS(search_tests)
	{
	public:
		
		/// <summary>
		/// Fill a vector with random numbers/"moves" and fill the PV-table from a max depth and then test that the root PV is the same
		/// as the original.
		/// </summary>
		TEST_METHOD(triangular_pv_table)
		{
			try
			{
				// Initialize a PV table and generate a list of random moves from root to MAX_DEPTH
				search::util::tri_pv_table pvTable;
				std::vector<move_t> moves;
				for (auto i = 0; i < MAX_DEPTH + 1; i++)
					moves.push_back((move_t)std::rand());

				// Emulate updating PV in search:
				// This is done by inserting the leaf move as if we were in alpha_beta(depth = MAX_DEPTH)
				// Then go up (actually down in depth) and update the PV with new moves, hopefully producing a complete principal variation
				// matching the one in moves
				for (int d = MAX_DEPTH; d >= 0; d--)
					pvTable.update_pv((eDepth)d, moves[d]);

				auto rootPv = pvTable.get_for_depth((eDepth)0);
				Assert::IsTrue(rootPv.first == moves.size() - 1, L"PV-table and original move list had differing lengths!");

				for (auto i = 0; i < rootPv.first; i++)
				{
					Assert::IsTrue(rootPv.second[i] == moves[i],
						std::format(L"Differing moves at depth '{}'. PV-table contained '{}' and original list contained '{}'",
							i,
							(uint16_t)rootPv.second[i],
							(uint16_t)moves[i])
						.c_str());
				}
			}
			catch (std::exception& e)
			{
				auto msg = e.what();
			}
		}
	};
}