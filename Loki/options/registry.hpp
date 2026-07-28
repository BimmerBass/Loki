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
#include "option.hpp"
#include "util/exception.hpp"
#include <string>

namespace loki::options
{
	class registry final
	{
		CHILD_EXCEPTION(option_error, loki_exception);
	public:
		template<value_option_t T, typename F>
			requires std::invocable<F&, typename std::remove_cvref_t<T>::underlying_type_t>
		void add(std::string name, T definition, F handler)
		{
			using def_t = std::remove_cvref_t<T>;
			using value_t = typename def_t::underlying_type_t;

			auto callback =
				[handler = std::move(handler)](
					const option_value_t value) mutable
				{
					if (std::holds_alternative<std::monostate>(value))
					{
						throw_msg<option_error>(
							"value option requires a value");
					}

					std::invoke(
						handler,
						std::get<value_t>(value));
				};

			_add(
				std::move(name),
				std::move(definition),
				std::move(callback));
		}

		template<typename F> requires std::invocable<F&>
		void add(std::string name, F handler)
		{
			auto callback =
				[handler = std::move(handler)](
					const option_value_t& value) mutable
				{
					if (!std::holds_alternative<std::monostate>(value))
						throw option_error{ "button option received a value" };

					std::invoke(handler);
				};

			_add(
				std::move(name),
				button{},
				std::move(callback));
		}

		void print(std::ostream& os) const noexcept
		{
			for (const auto& [_, definition] : _options)
			{
				definition.print(os);
			}
		}
	private:
		template<option_t T, typename F>
		void _add(
            std::string name,
            T definition,
            F handler)
        {
			if (contains_reserved_token(name))
			{
				throw_msg<option_error>(
					"option name contains a reserved token: {}", name);
			}

            loki_option registered{
                .name = name,
                .type = std::move(definition),
                .apply_callback = std::move(handler)
            };

            auto [iterator, inserted] = _options.emplace(
                name,
                std::move(registered));

            if (!inserted)
            {
                throw_msg<option_error>(
                    "duplicate option registered: {}", name);
            }
        }

        std::unordered_map<std::string, loki_option> _options;
	};
}
