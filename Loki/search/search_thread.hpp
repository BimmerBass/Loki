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
#include <future>

namespace loki::search
{
	struct search_command
	{
		search_command() = delete;
		search_command(const position::search_position_t& _position, const limits& _limits, info_sink_t _sink)
		{
			position = _position->clone();
			limits = _limits;
			sink = std::move(_sink);
		}
		std::unique_ptr<position::search_position> position;
		limits limits;
		info_sink_t sink;
		std::promise<search_result_t> promise;

		search_command(const search_command&) = delete;
		search_command& operator=(const search_command&) = delete;
		search_command(search_command&& cmd) noexcept :
			position{ std::move(cmd.position) },
			limits{ cmd.limits },
			sink{ std::move(cmd.sink) },
			promise{ std::move(cmd.promise) }
		{}
		search_command& operator=(search_command&& cmd) noexcept
		{
			position = std::move(cmd.position);
			limits = cmd.limits;
			sink = std::move(cmd.sink);
			promise = std::move(cmd.promise);

			return *this;
		}
	};

	class worker_thread
	{
		friend void thread_loop(std::stop_token, worker_thread&);

		using lock_t = std::unique_lock<std::mutex>;
		using guard_t = std::lock_guard<std::mutex>;
	public:
		CHILD_EXCEPTION(thread_exception, loki_exception);
	private:
		CHILD_EXCEPTION(stop_requested_exception, thread_exception);
	public:
		worker_thread(size_t id, std::unique_ptr<i_search_worker> worker);
		worker_thread(size_t id);
		virtual ~worker_thread()
		{
			join();
		}

		virtual std::future<search_result_t> search(
			const position::search_position_t& position,
			limits limits,
			info_sink_t sink);
		virtual void stop_search() noexcept;
		virtual void wait_for_finished_search();
		virtual void clear_context();

		virtual void ponderhit();

	protected:
		virtual void join() noexcept;
	private:
		search_command&& wait_for_work(std::stop_token tk);

		const size_t _id;
		std::mutex _mtx;
		std::condition_variable _cv;
		std::jthread _thread;

		std::optional<search_command> _task;

		std::unique_ptr<i_search_worker> _search_worker;
		limits* current_limits = nullptr;
	};
}
