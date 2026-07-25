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
#include <mutex>
#include <type_traits>
#include <random>

namespace loki::util
{
	/// <summary>
	/// Singleton class for all of Loki's random number generation.
	/// Creation and destruction is controlled using a static mutex while actual operations are controlled
	/// with an instance-mutex.
	/// </summary>
	class rng final
	{
		// Empirically tested seed to optimize magic bitboard generation time. Halved from original, but still needs improving.
		static constexpr uint64_t SEED = 0x3A8F05C53A8EFE12ULL;
	private:
		constexpr explicit rng(std::uint64_t seed = SEED) noexcept
			: _state{ seed }
		{}
		std::uint64_t _state;

		inline static rng* _instance = nullptr;
		inline static std::once_flag _init_flag;

		static void destroy()
		{
			if (_instance != nullptr)
				delete _instance;
			_instance = nullptr;
		}

	public:
		rng(const rng&) = delete;
		rng& operator=(const rng&) = delete;
		
		template<std::integral T>
		[[nodiscard]]
		constexpr T generate() noexcept
		{
			std::uint64_t z = (_state += 0x9E3779B97F4A7C15ULL);

			z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
			z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
			return static_cast<T>(z ^ (z >> 31));
		}

		template<std::integral T>
		[[nodiscard]]
		constexpr T generate_sparse() noexcept
		{
			return (generate<T>() & generate<T>() & generate<T>());
		}

		/// <summary>
		/// Get the (runtime) instance and optionally create it.
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

		static consteval rng create_consteval() noexcept
		{
			return rng{};
		}
	};
}