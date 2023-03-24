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
#ifndef PERFT_H
#define PERFT_H

namespace loki::utility
{
	/// <summary>
	/// class for performing a perft evaluation of a position. 
	/// </summary>
	class perft
	{
	public:
		perft() = delete;
		perft(const std::string& fen);

		perft(const perft&) = delete;
		perft& operator=(const perft&) = delete;
		perft(perft&&) = delete;
		perft& operator=(perft&&) = delete;

		size_t perform(eDepth d, std::ostream& os);
		void load(const std::string& new_fen);

		inline auto previous_nps() const noexcept
		{
			return m_nps;
		}
	private:
		std::string				m_initial_fen;
		position::position_t	m_pos;
		size_t					m_nodes;
		double					m_nps;

		size_t perft_test(eDepth d, std::ostream& os);
		void perft_internal(eDepth d);
	};
}

#endif