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

namespace loki::search
{

	class searcher
	{
	protected:
		EXCEPTION_CLASS(e_searcherError, e_lokiError);

	private:
		/// <summary>
		/// Information gathered during the search.
		/// </summary>
		struct _search_info
		{
			eDepth selective_depth;		/* The actual depth that the thread has searched due to reductions, pruning and extensions */
			uint64_t nodes;				/* The total amount of nodes, the thread has analyzed */
			uint64_t fail_high;			/* The amount of nodes where the thread has found a beta-cutoff */
			uint64_t fail_high_first;	/* The amount of nodes where the thread has found a beta-cutoff on the first tested move. fail_high_first / fail_high gives an indication on the quality of the move ordering */
		};
		inline static constexpr _search_info ZeroInfo{ static_cast<eDepth>(0), 0, 0, 0 };

		// Ctor params
	protected:
		const eThreadId								m_threadId;
	private:
		const movegen::magics::slider_generator_t	m_sliderGenerator;
		const evaluation::evaluation_params_t		m_evalParams;
		std::shared_ptr<std::atomic_bool>			m_stop; /* Controlled by the main-thread */

		// The thread's own params
		position::position_t						m_pos;
		std::unique_ptr<evaluation::evaluator>		m_eval;
		std::shared_ptr<const search_limits>		m_limits;
		_search_info								m_info;
		util::tri_pv_table<>						m_pvTable;

	protected:
		searcher() = delete;
		searcher(
			eThreadId threadId,
			const movegen::magics::slider_generator_t& sliderGen,
			const evaluation::evaluation_params_t& params,
			std::shared_ptr<std::atomic_bool> stop);

		void search(
			const position::game_state& state,
			std::shared_ptr<const search_limits>& limits);
	private:
		void check_stopped_search();	
		void preprocess_search(
			const position::game_state& state,
			std::shared_ptr<const search_limits>& limits);
		eValue root_search(eDepth depth, eValue alpha, eValue beta);
		eValue alpha_beta(eDepth depth, eValue alpha, eValue beta);
		eValue quiescence(eValue alpha, eValue beta);
		const movegen::move_list_t& generate_quiescence_moves();
		eDepth depth_for_thread(eDepth d) const;

	public:
		inline static constexpr double InternalToCpConversionFactor = 100.0 / 512.0;
		inline static eCentipawnValue to_centipawns(eValue valueInternal) noexcept
		{
			return static_cast<eCentipawnValue>(std::round(InternalToCpConversionFactor * static_cast<double>(valueInternal)));
		}
		inline static long long to_mate(eValue valueInternal) noexcept
		{
			auto addition = (std::abs(valueInternal) % 2 != 0) ? 1 : 0;
			auto score = (((long)VALUE_INF - std::abs(valueInternal)) / 2) + addition;

			return valueInternal > VALUE_MATE ? score : -score;
		}

		virtual ~searcher() {}
	};

}