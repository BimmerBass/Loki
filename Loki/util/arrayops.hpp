// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once
#include <array>
#include <type_traits>

namespace loki::util
{

    template <typename T, std::size_t... Ns>
    struct nested_array;

    template <typename T, std::size_t N>
    struct nested_array<T, N>
    {
        using type = std::array<T, N>;
    };

    template <typename T, std::size_t N, std::size_t... Ns>
    struct nested_array<T, N, Ns...>
    {
        using type = std::array<typename nested_array<T, Ns...>::type, N>;
    };

    template <typename T, std::size_t... Ns>
    using nested_array_t = typename nested_array<T, Ns...>::type;

    /// <summary>
    /// Fill an N-dimensional array with a given value.
    /// If it is a nested array (elements support fill()), then fill those as well.
    /// </summary>
    /// <typeparam name="T">Element type</typeparam>
    /// <typeparam name="N">Array size</typeparam>
    /// <param name="arr">the array</param>
    /// <param name="value">the value to fill</param>
    template <typename T, std::size_t N, typename U>
    constexpr void fill_all(std::array<T, N>& arr, const U& value)
    {
        if constexpr (std::is_same_v<T, std::remove_cvref_t<U>>)
        {
            arr.fill(value);
        }
        else
        {
            for (auto& inner : arr)
            {
                fill_all(inner, value);
            }
        }
    }

    /// <summary>
    /// Create a filled sizeof...(Ns)-dimensional array.
    /// </summary>
    /// <typeparam name="T">The type of the array elements.</typeparam>
    /// <typeparam name="...Ns">Dimensions of the array</typeparam>
    /// <param name="value">Value to fill</param>
    /// <returns>The constructed array</returns>
    template <typename T, std::size_t... Ns>
    constexpr auto construct_fill(const T& value) -> nested_array_t<T, Ns...>
    {
        static_assert(sizeof...(Ns) > 0);

        nested_array_t<T, Ns...> result{};
        fill_all(result, value);
        return result;
    }
}
