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
#include "loki.pch.hpp"

namespace loki::evaluation
{
	evaluator::evaluator(const position::position_t& pos, evaluation_params_t& params) : m_pos(pos), m_params(params)
	{
		m_currentVal = VALUE_ZERO;
	}

	eValue evaluator::score_position()
	{
		clear();

		m_currentVal = material<WHITE>() - material<BLACK>();

		// Now add tempo and return as an stm-relative score.
		m_currentVal += (m_pos->side_to_move() == WHITE) ? m_params->tempo : -m_params->tempo;
		return m_pos->side_to_move() == WHITE ? m_currentVal : -m_currentVal;
	}

	/// <summary>
	/// Evaluate the total material value of all pieces, a given side has.
	/// </summary>
	template<eSide S>
	eValue evaluator::material() const
	{
		return
			m_pos->piece_count<S, PAWN>()* m_params->pawn +
			m_pos->piece_count<S, KNIGHT>()* m_params->knight +
			m_pos->piece_count<S, BISHOP>()* m_params->bishop +
			m_pos->piece_count<S, ROOK>()* m_params->rook +
			m_pos->piece_count<S, QUEEN>()* m_params->queen;
	}

	void evaluator::clear()
	{
		m_currentVal = VALUE_ZERO;
	}
}