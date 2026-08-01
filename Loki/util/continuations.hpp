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

#include <coroutine>
#include <future>
#include <memory>
#include <thread>
#include <utility>

namespace loki::util
{
	template<typename T>
	class future_awaiter
	{
		struct state
		{
			explicit state(std::future<T>&& f)
				: future{ std::move(f) }
			{}

			std::future<T> future;
		};

	public:
		explicit future_awaiter(std::future<T>&& future)
			: state_{ std::make_shared<state>(std::move(future)) }
		{}

		bool await_ready() const
		{
			using namespace std::chrono_literals;

			return state_->future.wait_for(0s)
				== std::future_status::ready;
		}

		void await_suspend(std::coroutine_handle<> continuation)
		{
			auto state = state_;

			std::thread(
				[state = std::move(state), continuation]() mutable
				{
					state->future.wait();
					continuation.resume();
				})
				.detach();
		}

		T await_resume()
		{
			return state_->future.get();
		}

	private:
		std::shared_ptr<state> state_;
	};

	template<typename T>
	auto operator co_await(std::future<T>&& future)
	{
		return future_awaiter<T>{std::move(future)};
	}

	class detached_task
	{
	public:
		struct promise_type
		{
			detached_task get_return_object() noexcept
			{
				return {};
			}

			std::suspend_never initial_suspend() noexcept
			{
				return {};
			}

			std::suspend_never final_suspend() noexcept
			{
				return {};
			}

			void return_void() noexcept
			{}

			void unhandled_exception() noexcept
			{
				std::terminate();
			}
		};
	};

	/// <summary>Allows the caller to register a continuation and then continue execution immediately.</summary>
	template<typename T, typename F>
	detached_task then(std::future<T> future, F continuation)
	{
		if constexpr (std::is_void_v<T>)
		{
			co_await std::move(future);
			std::invoke(std::move(continuation));
		}
		else
		{
			T result = co_await std::move(future);

			std::invoke(
				std::move(continuation),
				std::move(result));
		}
	}
}