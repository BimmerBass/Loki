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
	evaluator::evaluator(const position::position_t& pos, const evaluation_params_t& params) : m_pos(pos), m_params(params)
	{
		m_currentVal = VALUE_ZERO;
	}

	eValue evaluator::score_position()
	{
		clear();

		m_currentVal = material<WHITE>() - material<BLACK>();
		m_currentVal += psqt<WHITE>() - psqt<BLACK>();

		// Now add tempo and return as an stm-relative score.
		m_currentVal += (m_pos->side_to_move() == WHITE) ? m_params->tempo : -m_params->tempo;
		return m_pos->side_to_move() == WHITE ? m_currentVal : -m_currentVal;
	}

	/// <summary>
	/// Evaluate the total material value of all pieces, a given side has.
	/// </summary>
	template<eSide _S> requires (_S == WHITE || _S == BLACK)
	eValue evaluator::material() const
	{
		return
			m_pos->piece_count<_S, PAWN>()* m_params->pawn +
			m_pos->piece_count<_S, KNIGHT>()* m_params->knight +
			m_pos->piece_count<_S, BISHOP>()* m_params->bishop +
			m_pos->piece_count<_S, ROOK>()* m_params->rook +
			m_pos->piece_count<_S, QUEEN>()* m_params->queen;
	}

	/// <summary>
	/// Evaluate the placement of the pieces for side _S
	/// </summary>
	template<eSide _S> requires (_S == WHITE || _S == BLACK)
	eValue evaluator::psqt() const
	{
		auto score = VALUE_ZERO;

		psqt<_S, PAWN>(score);
		psqt<_S, KNIGHT>(score);
		psqt<_S, BISHOP>(score);
		psqt<_S, ROOK>(score);
		psqt<_S, QUEEN>(score);

		// Handle the king separately since we know his square already.
		score += m_params->piece_square_tables.get<_S, KING>(m_pos->king_square<_S>());

		return score;
	}

	/// <summary>
	/// Evaluate the placement of the pieces with type _Pce for side _S
	/// </summary>
	template<eSide _S, ePiece _Pce> requires (_S == WHITE || _S == BLACK) && (_Pce >= PAWN && _Pce <= QUEEN)
	void evaluator::psqt(eValue& v) const
	{
		auto bb = m_pos->get_piece_bb<_S, _Pce>();

		while (bb)
		{
			auto sq = pop_bit(bb);
			v += m_params->piece_square_tables.get<_S, _Pce>((eSquare)sq);
		}
	}

	/// <summary>
	/// Prepare the evaluation object before executing a new evaluation.
	/// </summary>
	void evaluator::clear()
	{
		m_currentVal = VALUE_ZERO;
	}
}