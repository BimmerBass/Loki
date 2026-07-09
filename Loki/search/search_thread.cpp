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

#include "search_thread.hpp"

using namespace loki;
using namespace loki::search;
using namespace loki::position;

search_thread::search_thread(size_t id) :
	_worker{},
	thread_id{id},
	_cv{},
	_mtx{},
	_context{std::nullopt}
{
	_thread = std::jthread([this](std::stop_token tk)
		{
			this->thread_loop(tk);
		});
}

void search_thread::thread_loop(std::stop_token token)
{
	std::stop_callback callback(token, [&]()
		{
			_cv.notify_all();
		});
	while (true)
	{
		std::unique_lock<std::mutex> lock(_mtx);
		_cv.wait(lock, [&]() { return _context.has_value() || token.stop_requested();});

		// stop requested on the thread's token -> quit
		if (token.stop_requested())
			break;

		// stop not requested, launch a search
		search_context& ctx = _context.value();
		std::unique_ptr<position::search_position> search_pos = ctx.position->clone();
		limits limits = ctx.limits;
		std::stop_token search_stop = ctx.stop_source.get_token();
		callback_t callback = std::move(ctx.finished_callback);

		// unlock while searching to allow for cancellation.
		lock.unlock();
		auto result = _worker.search(std::move(search_pos), limits, search_stop);

		if (callback)
			callback(std::move(result));

		// lock again to clear context.
		lock.lock();
		_context = std::nullopt;

		// notify threads waiting
		lock.unlock();
		_cv.notify_all();
	}
}