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
	TEST_CLASS(misc_tests)
	{
	private:
		const wchar_t* GetWC(const char* c)
		{
			const size_t cSize = strlen(c) + 1;
			wchar_t* wc = new wchar_t[cSize];
			mbstowcs(wc, c, cSize);

			return wc;
		}
	public:
		TEST_METHOD(square_mirroring)
		{
			for (auto sq = A1; sq <= H8; sq++)
				Assert::IsTrue(
					make_side_relative<WHITE>(make_side_relative<WHITE>(sq)) == sq &&
					make_side_relative<BLACK>(make_side_relative<BLACK>(sq)) == sq);
		}
	};
}