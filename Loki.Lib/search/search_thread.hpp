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
	/// <summary>
	/// search_thread simply holds all thread-local resources required for a search, and is responsible for actually executing a search.
	/// </summary>
	class search_thread
	{
	protected:
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
		inline static const _search_info ZeroInfo{ static_cast<eDepth>(0), 0, 0, 0 };
	protected:
		position::position_t					m_pos;
		std::unique_ptr<evaluation::evaluator>	m_eval;
		std::unique_ptr<search_limits>			m_limits;
		_search_info							m_info;
		bool									m_stop;

		std::shared_ptr<std::atomic_bool> stop_searching;
	protected:
		search_thread() = delete;
		search_thread(
			const position::game_state&					state, 
			const movegen::magics::slider_generator_t	slider_generator,
			evaluation::evaluation_params_t&			eval_params);

		virtual eCentipawnValue search() { throw e_notImplementedError("Multithreaded search is not implemented yet! Only main_thread can perform a search."); }
		
		/// <summary>
		/// Check if we should stop the search. This is a simple no-op for worker threads but the main thread should implement this and check if the GUI has requested a stop.
		/// </summary>
		virtual void check_stopped_search();
	private:
		eValue alpha_beta(eDepth depth, eValue alpha, eValue beta);

		/// <summary>
		/// Clears data collected during previous searches.
		/// </summary>
		void preprocess_search();

		inline static constexpr double InternalToCpConversionFactor = 512.0 / 100.0;
		inline static eCentipawnValue to_centipawns(eValue valueInternal) noexcept
		{
			return static_cast<eCentipawnValue>(InternalToCpConversionFactor * static_cast<double>(valueInternal));
		}
	};

	class main_thread : public search_thread
	{
	protected:
		eCentipawnValue search() override;
		void check_stopped_search() override;
	};
}