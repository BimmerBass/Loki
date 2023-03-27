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
		EXCEPTION_CLASS(e_searchThread, e_lokiError);

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
	protected:
		position::position_t					m_pos;
		std::unique_ptr<evaluation::evaluator>	m_eval;
		std::shared_ptr<const search_limits>	m_limits;
		_search_info							m_info;
		bool									m_stop;
		util::tri_pv_table<>					m_pvTable;

		// ctor parameters.
		const movegen::magics::slider_generator_t	m_sliderGenerator;
		const evaluation::evaluation_params_t		m_evalParams;
		std::shared_ptr<std::atomic_bool>			m_stop_searching;
	protected:
		search_thread() = delete;
		search_thread(
			const movegen::magics::slider_generator_t	slider_generator,
			evaluation::evaluation_params_t&			eval_params,
			std::shared_ptr<std::atomic_bool>			stopFlag);

		/// <summary>
		/// Perform a search on the given position.
		/// </summary>
		virtual void search(
			const position::game_state& /* unused */,
			std::shared_ptr<const search_limits>& /* unused */)
		{
			throw e_notImplementedError("Multithreaded search is not implemented yet! Only main_thread can perform a search.");
		}
		
		/// <summary>
		/// Check if we should stop the search. This is a simple no-op for worker threads but the main thread should implement this and check if the GUI has requested a stop.
		/// </summary>
		virtual void check_stopped_search();

		/// <summary>
		/// Clears data collected during previous searches.
		/// </summary>
		void preprocess_search(
			const position::game_state& state,
			std::shared_ptr<const search_limits>& limits);

		/// <summary>
		/// Basically just alpha_beta without any pruning or aggressive reductions
		/// </summary>
		eValue root_search(eDepth depth, eValue alpha, eValue beta);
	private:
		/// <summary>
		/// Internal fail-hard PVS search function
		/// </summary>
		eValue alpha_beta(eDepth depth, eValue alpha, eValue beta);

		/// <summary>
		/// Capture resolving to avoid the horizon effect.
		/// </summary>
		eValue quiescence(eValue alpha, eValue beta);

		const movegen::move_list_t& generate_quiescence_moves();
	};

	class main_thread : public search_thread
	{
		EXCEPTION_CLASS(e_mainThread, e_searchThread);

		using searchThread_t = std::unique_ptr<search_thread>;
	private:
		
		std::vector<searchThread_t> m_workerThreads;
	public:
		main_thread() = delete;
		main_thread(
			const movegen::magics::slider_generator_t	slider_generator,
			evaluation::evaluation_params_t& eval_params);

		/// <summary>
		/// Start the worker threads (if there are any), and then enter search.
		/// </summary>
		void search(
			const position::game_state& state,
			std::shared_ptr<const search_limits>& limits) override;
	private:
		void check_stopped_search() override;
	};
}