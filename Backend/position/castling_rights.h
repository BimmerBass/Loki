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
#ifndef CASTLING_RIGHTS_H
#define CASTLING_RIGHTS_H

namespace loki::position {

	/// <summary>
	/// Holds data about the castling rights for both sides.
	/// </summary>
	class castle_rights {
	private:
		uint8_t m_rights;
	public:
		castle_rights() : m_rights(0) {
		}
		castle_rights(uint8_t state) : m_rights(state) {
		}

		/// <summary>
		/// Return whether or not the side to move can castle to the given side.
		/// </summary>
		template<CASTLING_RIGHTS _Ss>
		bool operator()() const noexcept {
			return ((m_rights >> _Ss) & uint8_t(1)) != 0;
		}

		/// <summary>
		/// Enable castling for a given side.
		/// </summary>
		/// <param name="_Ss"></param>
		void operator+=(CASTLING_RIGHTS _Ss) {
			m_rights |= (uint8_t(1) << _Ss);
		}

		/// <summary>
		/// Disable castling for a given side.
		/// </summary>
		/// <param name="_Ss"></param>
		void operator-=(CASTLING_RIGHTS _Ss) {
			if (((m_rights >> _Ss) & uint8_t(1)) != 0) // Only if we can castle.
				m_rights &= ~(uint8_t(1) << _Ss);
		}

		bool any() const {
			return m_rights != 0;
		}
		void clear() {
			m_rights = 0;
		}
	};

}

#endif