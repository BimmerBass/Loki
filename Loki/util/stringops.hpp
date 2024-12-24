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
#include <array>

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
	/// Join a vector of strings into a single string using a specified delimiter
	/// </summary>
	/// <param name="vec">The vector of strings to join</param>
	/// <param name="delim">Delimiter to use</param>
	/// <returns>A string of the accumulated vector</returns>
	std::string join(const std::vector<std::string>& vec, char delim);

	/// <summary>
	/// Convert an ASCII-string to lowercase.
	/// </summary>
	/// <param name="str">The string in question.</param>
	/// <returns>A lowercase copy of str.</returns>
	std::string lowercase(const std::string& str);

	/// <summary>
	/// Convert an ASCII-string to uppercase.
	/// </summary>
	/// <param name="str">The string in question.</param>
	/// <returns>A lowercase copy of str.</returns>
	std::string uppercase(const std::string& str);
}

template<typename EnumT>
struct enum_strings
{
	static constexpr std::array<const char*, 1> strings = { "<unused>" };
};

/// <summary>
/// Convert an enum to a string representation.
/// NOTE: To enable this function, use the ENABLE_STRINGIFY macro instead of direct template instantiation.
/// </summary>
/// <typeparam name="EnumT">The enum type</typeparam>
/// <param name="value">Value of the enum</param>
/// <returns>A string representing the enum</returns>
template<typename T>
constexpr const char* enum_to_string(T value)
{
	auto index = static_cast<size_t>(value);
	if (index < enum_strings<T>::strings.size())
	{
		return enum_strings<T>::strings[index];
	}
	return "<invalid>";
}

#define ENABLE_STRINGIFY(enumType, ...) \
	template<>\
	struct enum_strings<enumType>{\
		static constexpr std::array<const char*, std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value> strings = {{__VA_ARGS__}};\
	};	
	
