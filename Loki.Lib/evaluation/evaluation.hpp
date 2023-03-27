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

namespace loki::evaluation
{
	class evaluator
	{
	private:
		position::position_t m_pos;
		evaluation_params_t m_params;
		
		eValue m_currentVal;
	public:
		// Can't evaluate without a postion..
		evaluator() = delete;
		evaluator(const position::position_t& pos, const evaluation_params_t& params);

		eValue score_position();
	
	private:
		void clear();
		
		template<eSide _S> requires (_S == WHITE || _S == BLACK)
		eValue material() const;

		template<eSide _S> requires (_S == WHITE || _S == BLACK)
		eValue psqt() const;

		template<eSide _S, ePiece _Pce> requires (_S == WHITE || _S == BLACK) && (_Pce >= PAWN && _Pce <= QUEEN)
		void psqt(eValue& v) const;
	};

#pragma region Explicit template instantiations
	template eValue evaluator::material<WHITE>() const;
	template eValue evaluator::material<BLACK>() const;
#pragma endregion

}