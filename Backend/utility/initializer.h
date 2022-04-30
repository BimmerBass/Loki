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
#ifndef INITIALIZER_H
#define INITIALIZER_H

namespace loki::utility {

	/// <summary>
	/// Initializer class is responsible for creating a static object upon startup (before main) which will call all initialization methods in its constructor.
	/// </summary>
	class initializer {
	public:
		initializer();
	};

	// Static object which will be constructed before main.
	static initializer m_object;
}

#endif