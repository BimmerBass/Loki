// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once
#include "position/search_position.hpp"
#include "limits.hpp"
#include "search_worker.hpp"
#include <thread>
#include <mutex>
#include <functional>

namespace loki::search
{

	class search_thread
	{
	public:
		CHILD_EXCEPTION(thread_exception, loki_exception);

		using callback_t = std::function<void(search_result_t)>;
	private:
		struct search_context
		{
			std::unique_ptr<position::search_position> position;
			limits limits;

			callback_t finished_callback;
			info_sink_t info_sink;
		};
	public:
		search_thread(size_t id);
		// The injected worker must outlive this search thread.
		search_thread(size_t id, std::unique_ptr<i_search_worker> worker);
		~search_thread() {
			join();
		}

		// make the thread search
		virtual void search(
			const position::search_position_t& position,
			const limits& limits,
			callback_t callback,
			info_sink_t sink = std::make_unique<null_sink>())
		{
			if (!position)
				throw_msg<thread_exception>("search requested with null position");

			std::unique_lock<std::mutex> lock(_mtx);
			if (_context.has_value())
				throw_msg<thread_exception>("search requested on active thread");
			_context.emplace(search_context
				{
					.position = position->clone(),
					.limits = limits,
					.finished_callback = std::move(callback),
					.info_sink = std::move(sink)
				});
			_context.value().limits.stop_source = std::stop_source{};

			lock.unlock();
			_cv.notify_one();
		}

		void ponderhit(){
			std::lock_guard<std::mutex> lock(_mtx);
			if (!_context || _callback_running || !_current_limits)
				throw_msg<thread_exception>("got ponderhit on idle thread.");
			auto limits = _current_limits.value()();

			limits->start_time = limits::clock_t::now();
			limits->pondering.store(false, std::memory_order_release);
		}

		// Stop the current search.
		void stop_search()
		{
			std::scoped_lock<std::mutex> lock(_mtx);
			if (!_context.has_value())
				return;
			_context.value().limits.stop_source.value().request_stop();
		}

		void wait_for_finished_search()
		{
			std::unique_lock<std::mutex> lock(_mtx);
			if (!_context && !_callback_running)
				return;
			_cv.wait(lock, [&]() { return _context == std::nullopt && !_callback_running; });
		}

		void newgame_clear() {
			std::lock_guard<std::mutex> lock(_mtx);
			_search_worker->newgame_clear();
		}
	private:
		// stop the search and join.
		void join() noexcept
		{
			std::unique_lock<std::mutex> lock(_mtx);

			if (_thread.joinable())
			{
				if (_context != std::nullopt)
					_context.value().limits.stop_source.value().request_stop();

				_thread.request_stop();
				lock.unlock();

				_thread.join();
			}
		}
		void start_thread();
		void thread_loop(std::stop_token token);

		size_t thread_id;
		std::condition_variable _cv;
		std::mutex _mtx;

		std::unique_ptr<i_search_worker> _search_worker;
		std::optional<search_context> _context;
		std::optional<std::function<limits*()>> _current_limits;
		bool _callback_running = false;

		// must be declared last!
		std::jthread _thread;
	};
}
