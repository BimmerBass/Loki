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
#include <ranges>
#include "stringops.hpp"

namespace loki::util
{
	std::vector<std::string> split(const std::string& str, char sep, bool keep_empty_entries)
	{
		auto split = str
			| std::views::split(sep)
			| std::views::filter([&](const auto& subr)
				{
					if (keep_empty_entries) return true;
					return std::distance(subr.begin(), subr.end()) > 0;
				})
			| std::views::transform([](const auto& sr) { return std::string(sr.begin(), sr.end()); });
		return std::vector(split.begin(), split.end());
	}

	std::string lowercase(const std::string& str)
	{
		return str 
			| std::views::transform([](const char c) { return (char)std::tolower(c); }) 
			| std::ranges::to<std::string>();
	}
}