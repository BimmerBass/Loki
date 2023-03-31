#include "loki.pch.hpp"
using namespace loki::movegen;

namespace loki::ordering
{
	move_sorter::move_sorter(const position::position_t& pos, bool isQuiescence, bool performScoring)
		: m_pos(pos), m_is_quiescence(isQuiescence), m_perform_scoring(performScoring), m_moveList{nullptr}, m_currentInx{0}
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
		m_moveList = const_cast<move_list_t*>(&m_pos->generate_moves());
		m_currentInx = 0;
	}

	move_t move_sorter::get_next()
	{
		if (m_moveList == nullptr)
			generate();
		if (m_currentInx >= m_moveList->size())
			return MOVE_NULL;

		// If we don't score, we don't need to sort.
		if (m_perform_scoring)
			bringBestMoveFront();
		
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
				bestScore = m_moveList->at(m_currentInx).score;
			}
		}

		// Now that we know the best index, swap it with the current index, and increment the latter.
		if (bestInx != m_currentInx)
			m_moveList->swap(m_currentInx, bestInx);
	}

#pragma region Template instantiations
	template void move_sorter::generate<ALL>();
	template void move_sorter::generate<QUIET>();
	template void move_sorter::generate<ACTIVES>();
#pragma endregion
}