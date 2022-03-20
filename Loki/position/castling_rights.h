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
			return ((m_rights << _Ss) & uint8_t(1)) != 0;
		}

		/// <summary>
		/// Enable castling for a given side.
		/// </summary>
		/// <param name="_Ss"></param>
		void operator+=(CASTLING_RIGHTS _Ss) {
			m_rights |= (uint8_t(1) >> _Ss);
		}

		/// <summary>
		/// Disable castling for a given side.
		/// </summary>
		/// <param name="_Ss"></param>
		void operator-=(CASTLING_RIGHTS _Ss) {
			if (((m_rights << _Ss) & uint8_t(1)) != 0) // Only if we can castle.
				m_rights &= ~(uint8_t(1) >> _Ss);
		}
	};

}

#endif