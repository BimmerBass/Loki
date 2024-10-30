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
#include "__gitversion_autogen.hpp"

#define VERSION_MAJOR "4"
#define VERSION_MINOR "0"
#define VERSION_PATCH "0"
#ifndef NDEBUG
#define VERSION_BUILD "-" GIT_CUR_COMMIT "@" GIT_BRANCH
#else
#define VERSION_BUILD ""
#endif

// Loki information
constexpr const char* NAME = "Loki";
constexpr const char* AUTHOR = "Niels Abildskov (BimmerBass)";
constexpr const char* VERSION =
	VERSION_MAJOR "."
	VERSION_MINOR "."
	VERSION_PATCH
	VERSION_BUILD;


