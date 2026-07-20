// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

#include "pch.hpp"
#include "Loki/search/search_thread.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"

#include <future>

namespace search_tests
{
	using namespace std::chrono_literals;
	using namespace loki;
	using namespace loki::movegen;
	using namespace loki::movegen::magics;
	using namespace loki::position;
	using namespace loki::search;

	namespace
	{
		search_position_t make_start_position()
		{
			auto state = game_state::from_fen(constants::START_FEN);
			auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
			auto rook_index = std::make_shared<hardcoded_index<ROOK>>();

			return make(state, bishop_index, rook_index);
		}

		search_result_t successful_result()
		{
			return search_score_t{ cp_score{ 0 } };
		}

		class marker_sink final : public info_sink
		{
		public:
			void info(
				size_t,
				search_score_t,
				size_t,
				std::chrono::milliseconds,
				size_t,
				size_t,
				std::vector<move>, size_t, size_t) override
			{
			}

			void bestmove(move, std::optional<move>) override
			{
			}
		};

		struct blocking_control
		{
			std::mutex mutex;
			std::condition_variable cv;
			bool entered = false;
			bool release = false;
			bool stop_observed = false;
		};

		class functional_worker final : public i_search_worker
		{
		public:
			using function_t = std::function<search_result_t(
				std::unique_ptr<search_position>,
				const limits&,
				std::stop_token,
				info_sink_t)>;

			explicit functional_worker(function_t function) : _function{ std::move(function) }
			{
			}

			search_result_t search(
				std::unique_ptr<search_position> position,
				const limits& search_limits,
				std::stop_token stop_token,
				info_sink_t sink) noexcept override
			{
				return _function(
					std::move(position),
					search_limits,
					stop_token,
					std::move(sink));
			}

			void newgame_clear() override {}
		private:
			function_t _function;
		};

		void require_entered(const std::shared_ptr<blocking_control>& control)
		{
			std::unique_lock lock{ control->mutex };
			REQUIRE(control->cv.wait_for(lock, 1s, [&]() { return control->entered; }));
		}

		std::unique_ptr<i_search_worker> make_blocking_worker(
			const std::shared_ptr<blocking_control>& control)
		{
			return std::unique_ptr<i_search_worker>(new functional_worker{ [control](
				std::unique_ptr<search_position>,
				const limits&,
				std::stop_token token,
				info_sink_t)
				{
					std::stop_callback notify_on_stop(token, [&]() { control->cv.notify_all(); });
					std::unique_lock lock{ control->mutex };
					control->entered = true;
					control->cv.notify_all();
					control->cv.wait(lock, [&]() { return control->release || token.stop_requested(); });
					control->stop_observed = token.stop_requested();
					return successful_result();
				} });
		}
	}

	TEST_CASE("search_thread clones and forwards a submitted search", "[search][search_thread]")
	{
		auto position = make_start_position();
		const auto* original = position.get();
		std::mutex mutex;
		size_t executor_calls = 0;
		size_t callback_calls = 0;
		bool received_clone = false;
		bool received_limits = false;
		bool received_sink = false;
		bool callback_received_value = false;

		auto worker = std::unique_ptr<i_search_worker>(new functional_worker{ [&](
			std::unique_ptr<search_position> submitted_position,
			const limits& submitted_limits,
			std::stop_token,
			info_sink_t sink)
			{
				std::scoped_lock lock{ mutex };
				++executor_calls;
				received_clone = submitted_position && submitted_position.get() != original
					&& submitted_position->to_fen() == position->to_fen();
				received_limits = submitted_limits.depth == 3;
				received_sink = dynamic_cast<marker_sink*>(sink.get()) != nullptr;
				return successful_result();
			} });
		search_thread thread{ 0, std::move(worker) };

		limits search_limits{};
		search_limits.depth = 3;
		thread.search(
			position,
			search_limits,
			[&](search_result_t result)
			{
				std::scoped_lock lock{ mutex };
				++callback_calls;
				callback_received_value = result.has_value();
			},
			std::make_unique<marker_sink>());
		thread.wait_for_finished_search();

		std::scoped_lock lock{ mutex };
		REQUIRE(executor_calls == 1);
		REQUIRE(callback_calls == 1);
		REQUIRE(received_clone);
		REQUIRE(received_limits);
		REQUIRE(received_sink);
		REQUIRE(callback_received_value);
	}

	TEST_CASE("search_thread rejects overlapping searches and remains reusable", "[search][search_thread]")
	{
		auto control = std::make_shared<blocking_control>();
		auto worker = make_blocking_worker(control);
		search_thread thread{ 0, std::move(worker) };
		auto position = make_start_position();
		limits search_limits{};

		thread.search(position, search_limits, nullptr);
		require_entered(control);
		REQUIRE_THROWS_AS(
			thread.search(position, search_limits, nullptr),
			search_thread::thread_exception);

		{
			std::scoped_lock lock{ control->mutex };
			control->release = true;
		}
		control->cv.notify_all();
		thread.wait_for_finished_search();

		REQUIRE_NOTHROW(thread.search(position, search_limits, nullptr));
		thread.wait_for_finished_search();
	}

	TEST_CASE("search_thread sends active stop requests to its executor", "[search][search_thread]")
	{
		auto control = std::make_shared<blocking_control>();
		auto worker = make_blocking_worker(control);
		search_thread thread{ 0, std::move(worker) };
		auto position = make_start_position();

		thread.search(position, limits{}, nullptr);
		require_entered(control);
		thread.stop_search();
		thread.wait_for_finished_search();

		std::scoped_lock lock{ control->mutex };
		REQUIRE(control->stop_observed);
	}

	TEST_CASE("search_thread ignores stop requests while idle", "[search][search_thread]")
	{
		auto worker = std::unique_ptr<i_search_worker>(new functional_worker{ [](
			std::unique_ptr<search_position>,
			const limits&,
			std::stop_token,
			info_sink_t)
			{
				return successful_result();
			} });
		search_thread thread{ 0, std::move(worker) };

		REQUIRE_NOTHROW(thread.stop_search());
	}

	TEST_CASE("search_thread wait includes callback completion", "[search][search_thread]")
	{
		auto worker = std::unique_ptr<i_search_worker>(new functional_worker{ [](
			std::unique_ptr<search_position>,
			const limits&,
			std::stop_token,
			info_sink_t)
			{
				return successful_result();
			} });
		search_thread thread{ 0, std::move(worker) };
		auto position = make_start_position();
		std::mutex mutex;
		std::condition_variable cv;
		bool callback_entered = false;
		bool release_callback = false;

		thread.search(position, limits{}, [&](search_result_t)
			{
				std::unique_lock lock{ mutex };
				callback_entered = true;
				cv.notify_all();
				cv.wait(lock, [&]() { return release_callback; });
			});
		{
			std::unique_lock lock{ mutex };
			REQUIRE(cv.wait_for(lock, 1s, [&]() { return callback_entered; }));
		}

		auto waiter = std::async(std::launch::async, [&]() { thread.wait_for_finished_search(); });
		REQUIRE(waiter.wait_for(50ms) == std::future_status::timeout);

		{
			std::scoped_lock lock{ mutex };
			release_callback = true;
		}
		cv.notify_all();
		REQUIRE(waiter.wait_for(1s) == std::future_status::ready);
	}

	TEST_CASE("search_thread callback can enqueue the next search", "[search][search_thread]")
	{
		std::atomic_size_t executor_calls = 0;
		auto worker = std::unique_ptr<i_search_worker>(new functional_worker{ [&] (
			std::unique_ptr<search_position>,
			const limits&,
			std::stop_token,
			info_sink_t)
			{
				++executor_calls;
				return successful_result();
			} });
		search_thread thread{ 0, std::move(worker) };
		auto position = make_start_position();
		std::atomic_size_t callback_calls = 0;

		thread.search(position, limits{}, [&](search_result_t)
			{
				++callback_calls;
				thread.search(position, limits{}, [&](search_result_t) { ++callback_calls; });
			});
		thread.wait_for_finished_search();

		REQUIRE(executor_calls == 2);
		REQUIRE(callback_calls == 2);
	}

	TEST_CASE("search_thread destructor cancels and joins an active executor", "[search][search_thread]")
	{
		auto control = std::make_shared<blocking_control>();
		auto position = make_start_position();

		{
			auto worker = make_blocking_worker(control);
			search_thread thread{ 0, std::move(worker) };
			thread.search(position, limits{}, nullptr);
			require_entered(control);
		}

		std::scoped_lock lock{ control->mutex };
		REQUIRE(control->stop_observed);
	}

	TEST_CASE("search_thread rejects a null position synchronously", "[search][search_thread]")
	{
		auto worker = std::unique_ptr<i_search_worker>(new functional_worker{ [](
			std::unique_ptr<search_position>,
			const limits&,
			std::stop_token,
			info_sink_t)
			{
				return successful_result();
			} });
		search_thread thread{ 0, std::move(worker) };
		search_position_t position{};

		REQUIRE_THROWS_AS(
			thread.search(position, limits{}, nullptr),
			search_thread::thread_exception);
	}

	TEST_CASE("ponderhit converts an active ponder search to timed search", "[search][search_thread][ponder]")
	{
		std::mutex mutex;
		std::condition_variable cv;
		bool entered = false;
		bool release = false;
		bool observed_pondering = true;
		limits::timepoint_t observed_start{};

		auto worker = std::unique_ptr<i_search_worker>(new functional_worker{ [&] (
			std::unique_ptr<search_position>,
			const limits& active_limits,
			std::stop_token,
			info_sink_t)
			{
				{
					std::unique_lock lock{ mutex };
					entered = true;
					cv.notify_all();
					cv.wait(lock, [&]() { return release; });
				}

				observed_pondering = active_limits.pondering.load(std::memory_order_acquire);
				observed_start = active_limits.start_time;
				return successful_result();
			} });
		search_thread thread{ 0, std::move(worker) };
		auto position = make_start_position();

		limits search_limits{};
		search_limits.pondering = true;
		search_limits.start_time = limits::timepoint_t{};
		thread.search(position, search_limits, nullptr);

		{
			std::unique_lock lock{ mutex };
			REQUIRE(cv.wait_for(lock, 1s, [&]() { return entered; }));
		}

		const auto before_hit = limits::clock_t::now();
		thread.ponderhit();

		{
			std::scoped_lock lock{ mutex };
			release = true;
		}
		cv.notify_all();
		thread.wait_for_finished_search();

		REQUIRE_FALSE(observed_pondering);
		REQUIRE(observed_start >= before_hit);
	}

	TEST_CASE("ponderhit rejects an idle search thread", "[search][search_thread][ponder]")
	{
		auto worker = std::unique_ptr<i_search_worker>(new functional_worker{ [] (
			std::unique_ptr<search_position>,
			const limits&,
			std::stop_token,
			info_sink_t)
			{
				return successful_result();
			} });
		search_thread thread{ 0, std::move(worker) };

		REQUIRE_THROWS_AS(thread.ponderhit(), search_thread::thread_exception);
	}
}
