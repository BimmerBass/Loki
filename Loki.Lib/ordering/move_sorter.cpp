#include "loki.pch.hpp"
using namespace loki::movegen;

namespace loki::ordering
{
	move_sorter::move_sorter(
		const position::position_t&		pos,
		const std::shared_ptr<stats_t>&	stats,
		bool							isQuiescence,
		bool							performScoring)
		: m_pos(pos), m_stats(stats), m_is_quiescence(isQuiescence), m_perform_scoring(performScoring), m_moveList{nullptr}, m_currentInx{0}
	{}

	/// <summary>
	/// Generate the moves and score them.
	/// TODO: Perhaps the template argument should be moved up to cover the entire class?
	/// </summary>
	template<eMoveType _T>
	void move_sorter::generate()
	{
		// NOTE: Although const_cast is generally considered bad practice, it is deemed okay to do here
		// This is because position::generate_moves is a public method and we do not want anyone (and their grandma) being able to change the generated moves seeing as the returned reference is part of
		// position's internal structure, but we trust the move_sorter class since its only objective is to score and return the moves.

		// If we are in quiescence, but are in check, we still want to generate all moves in order to not miss a checkmate/stalemate.
		if (!m_is_quiescence || m_pos->in_check())
			m_moveList = const_cast<move_list_t*>(&m_pos->generate_moves());
		else
			m_moveList = const_cast<move_list_t*>(&m_pos->generate_moves<movegen::ACTIVE>());
		m_currentInx = 0;

		if (m_perform_scoring)
			scoreMoves();
	}

	move_t move_sorter::get_next(eValue* score)
	{
		if (m_moveList == nullptr)
			generate();
		if (m_currentInx >= m_moveList->size())
			return MOVE_NULL;

		// If we don't score, we don't need to sort.
		if (m_perform_scoring)
			bringBestMoveFront();
		
		if (score != nullptr)
			*score = m_moveList->at(m_currentInx).score;
		return m_moveList->at(m_currentInx++).move;
	}

	/// <summary>
	/// Find the best move after our current index and swap with the data in said slot.
	/// </summary>
	void move_sorter::bringBestMoveFront()
	{
		size_t bestInx = m_currentInx;
		auto bestScore = m_moveList->at(m_currentInx).score;

		for (auto inx = m_currentInx; inx < m_moveList->size(); inx++)
		{
			if (m_moveList->at(inx).score > bestScore)
			{
				bestInx = inx;
				bestScore = m_moveList->at(inx).score;
			}
		}

		// Now that we know the best index, swap it with the current index, and increment the latter.
		if (bestInx != m_currentInx)
			m_moveList->swap(m_currentInx, bestInx);
	}

	/// <summary>
	/// Score the moves generated. Right now this only consists of Mvv/Lva for captures.
	/// </summary>
	void move_sorter::scoreMoves()
	{
		if (m_moveList == nullptr)
			throw e_moveSorter("scoreMoves was called before moves were generated");
		const static eValue victimValues[PIECE_NB] = { (eValue)100, (eValue)200, (eValue)300, (eValue)400, (eValue)500, (eValue)600 };

		for (auto i = 0; i < m_moveList->size(); i++)
		{
			auto& sm = m_moveList->at(i);
			if (m_pos->type_of(sm.move) == ACTIVE)
			{
				auto victim = m_pos->piece_on_sq(to_sq(sm.move), !m_pos->side_to_move());
				auto attacker = m_pos->piece_on_sq(from_sq(sm.move), m_pos->side_to_move());

				if (special(sm.move) == ENPASSANT)
					sm.score = victimValues[PAWN] - (eValue)PAWN;
				else if (victim != PIECE_NB)
				{
					sm.score = victimValues[victim] - (eValue)attacker;
					// For promotions we score it as the capture itself, along with the material gained from the new piece while losing the pawn.
					if (special(sm.move) == PROMOTION)
						sm.score += victimValues[promotion_piece(sm.move)] - victimValues[PAWN];
				}
				sm.score += CaptureScale;
			}
			else
			{
				// Apply killers.
				if (sm.move == m_stats->ply_stats(m_pos->ply())->killers.first)
					sm.score = CaptureScale - 1;
				else if (sm.move == m_stats->ply_stats(m_pos->ply())->killers.second)
					sm.score = CaptureScale - 2;
				else
				{
					// If the move wasn't a killer, score it with the history table.
					auto fromSq = from_sq(sm.move);
					auto toSq = to_sq(sm.move);
					sm.score = m_stats->history_score(m_pos->side_to_move(), fromSq, toSq);
				}
			}
		}
	}

#pragma region Template instantiations
	template void move_sorter::generate<ALL>();
	template void move_sorter::generate<QUIET>();
	template void move_sorter::generate<ACTIVE>();
#pragma endregion
}