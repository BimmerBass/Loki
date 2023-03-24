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

// Neat way of defining operators taken by stockfish (see https://github.com/official-stockfish/Stockfish/blob/master/src/types.h line 307)
#define ENABLE_BASE_OPERATORS_ON(_Ty)																	\
template<typename T> requires(!std::is_same_v<T, _Ty>)													\
inline constexpr _Ty operator+(_Ty v1, T v2) noexcept { return static_cast<_Ty>((T)v1 + v2); }			\
inline constexpr _Ty operator+(_Ty v1, _Ty v2) noexcept {return static_cast<_Ty>((long)v1 + (long)v2);} \
template<typename T> requires(!std::is_same_v<T, _Ty>)													\
inline constexpr _Ty operator-(_Ty v1, T v2) noexcept { return static_cast<_Ty>((T)v1 - v2); }			\
inline constexpr _Ty operator-(_Ty v1, _Ty v2) noexcept {return static_cast<_Ty>((long)v1 - (long)v2);} \
inline constexpr _Ty operator-(_Ty rhs) noexcept { return static_cast<_Ty>(-long(rhs)); }				\
inline constexpr _Ty& operator+=(_Ty& v, long rhs) noexcept { return v = v + rhs; }						\
inline constexpr _Ty& operator-=(_Ty& v, long rhs) noexcept { return v = v - rhs; }

#define ENABLE_INCR_OPERATORS_ON(_Ty)																\
template<typename T> requires(!std::is_same_v<T, _Ty>)												\
inline constexpr _Ty& operator++(_Ty& v, T) noexcept { return v = static_cast<_Ty>((long)v + 1); }	\
template<typename T> requires(!std::is_same_v<T, _Ty>)												\
inline constexpr _Ty& operator--(_Ty& v, T) noexcept { return v = static_cast<_Ty>((long)v - 1); }

#define ENABLE_FULL_OPERATORS_ON(_Ty)															\
ENABLE_BASE_OPERATORS_ON(_Ty)																	\
template<typename T> requires(!std::is_same_v<T, _Ty>)											\
inline constexpr _Ty operator*(T i, _Ty v) { return static_cast<_Ty>(i * (T)v); }				\
template<typename T> requires(!std::is_same_v<T, _Ty>)											\
inline constexpr _Ty operator*(_Ty v, T i) { return static_cast<_Ty>((T)v * i); }				\
inline constexpr _Ty operator/(_Ty v, long i) { return static_cast<_Ty>((long)v / i); }			\
inline constexpr long operator/(_Ty v1, _Ty v2) {return (long)v1 / (long)v2; }					\
inline constexpr _Ty& operator*=(_Ty& v, long i) { return v = static_cast<_Ty>((long)v * i); }	\
inline constexpr _Ty& operator/=(_Ty& v, long i) { return v = static_cast<_Ty>((long)v / i); }
