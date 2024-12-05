#pragma once
#include <mutex>
#include <type_traits>
#include <random>

namespace loki::util
{
	/// <summary>
	/// Thread-Safe (perhaps redundant) Singleton class for all of Loki's random number generation.
	/// Creation and destruction is controlled using a static mutex while actual operations are controlled
	/// with an instance-mutex.
	/// </summary>
	class rng
	{
	public:
		// Empirically tested seed to optimize magic bitboard generation time. Halved from original, but still needs improving.
		inline static constexpr uint64_t SEED = 0x3A8F05C53A8EFE12ULL;
	private:
		inline static rng* _instance = nullptr;
		inline static std::once_flag _init_flag;

		static void destroy()
		{
			if (_instance != nullptr)
				delete _instance;
			_instance = nullptr;
		}
	private: // instance
		std::mt19937_64 m_engine;
		std::recursive_mutex m_mtx;

		rng() : m_engine(SEED), m_mtx{}
		{}
		~rng() = default;
	public:
		rng(const rng&) = delete;
		void operator=(const rng&) = delete;

		/// <summary>
		/// Get the instance and optionally create it.
		/// </summary>
		/// <returns>A pointer to the static rng object.</returns>
		static rng* instance()
		{
			std::call_once(_init_flag, []()
				{
					_instance = new rng();
					std::atexit(&destroy);
				});
			return _instance;
		}

		/// <summary>
		/// Generate a random number of the specified type.
		/// </summary>
		/// <typeparam name="T">The type to generate</typeparam>
		/// <param name="min">Minimum value of the generated number </param>
		/// <param name="max">Maximum value of the generated number</param>
		/// <returns>A newly generated, random number.</returns>
		template<typename T> requires std::is_integral_v<T> && (sizeof(T) <= sizeof(uint64_t))
		T generate(
			T min = std::numeric_limits<T>::min(),
			T max = std::numeric_limits<T>::max())
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			T result;

			if constexpr (sizeof(T) < sizeof(uint16_t)) // std::uniform_int_distribution doesn't support types smaller than short
			{
				using lt = std::conditional_t<std::is_signed_v<T>, int16_t, uint16_t>;
				lt larger_result = generate<lt>(static_cast<lt>(min), static_cast<lt>(max));
				result = static_cast<T>(larger_result);
			}
			else
			{
				std::uniform_int_distribution<T> dist(min, max);
				result = dist(m_engine);
			}
			return result;
		}

		/// <summary>
		/// Generate a random number with only a few high-valued bits.
		/// </summary>
		/// <typeparam name="T">Type of number to generate</typeparam>
		/// <returns>A newly generated, sparse random number</returns>
		template<typename T>
		T generate_sparse()
		{
			return (generate<T>() & generate<T>() & generate<T>());
		}

		void set_seed(uint64_t x)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtx);
			m_engine.seed(x);
		}
	};
}