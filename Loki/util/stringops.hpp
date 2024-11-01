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
#pragma once
#include <string>
#include <vector>

namespace loki::util
{
	/// <summary>
	/// Split a string on each occurence of a given separator.
	/// </summary>
	/// <param name="str">The string to split</param>
	/// <param name="sep">The separator</param>
	/// <param name="keep_empty_entries">Whether or not to include empty substrings (default = false)1</param>
	/// <returns>A vector with the splitted string.</returns>
	std::vector<std::string> split(const std::string& str, char sep, bool keep_empty_entries = false);

	/// <summary>
	/// Convert an ASCII-string to lowercase.
	/// </summary>
	/// <param name="str">The string in question.</param>
	/// <returns>A lowercase copy of str.</returns>
	std::string lowercase(const std::string& str);
}