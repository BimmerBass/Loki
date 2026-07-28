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
	public:
		CHILD_EXCEPTION(option_error, loki_exception);
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

		void apply(std::string name, std::optional<std::string> value) const
		{
			try
			{
				const auto& option = _options.at(name);
				const auto visitor = overloads
				{
					[value](const checkbox&) -> option_value_t
					{
						if (!value)
							throw_msg<option_error>("a value is required for a check-type option");
						auto lowercased = util::lowercase(*value);
						std::istringstream is(lowercased);
						bool b{};
						if (!(is >> std::boolalpha >> b))
							throw_msg<option_error>("value '{}' is invalid for a check-type option", *value);
						is >> std::ws;
						if (!is.eof())
							throw_msg<option_error>("value '{}' is invalid for a check-type option", *value);
						return b;
					},
					[&](const spin& opt) -> option_value_t
					{
						if (!value)
							throw_msg<option_error>("a value is required for a spin-type option");
						int64_t ivalue;
						std::size_t parsed_characters;
						try
						{
							ivalue = std::stoll(*value, &parsed_characters);
						}
						catch (const std::invalid_argument&)
						{
							throw_msg<option_error>("value '{}' is invalid for a spin-type option", *value);
						}
						catch (const std::out_of_range&)
						{
							throw_msg<option_error>("value '{}' is invalid for a spin-type option", *value);
						}
						if (parsed_characters != value->size())
							throw_msg<option_error>("value '{}' is invalid for a spin-type option", *value);
						if (!opt.is_valid(ivalue))
							throw_msg<option_error>("value {} is invalid for option '{}' (range=[{}, {}])", ivalue, option.name, opt.min, opt.max);
						return ivalue;
					},
					[&](const combo& opt)->option_value_t
					{
						if (!value)
							throw_msg<option_error>("a value is required for a combo-type option");
						if (!opt.is_valid(*value))
							throw_msg<option_error>("value '{}' is invalid for option '{}' (allowed={})", *value, option.name, opt.values);
						return *value;
					},
					[value](const string& opt) -> option_value_t
					{
						if (!value)
							throw_msg<option_error>("a value is required for a string-type option");
						return *value;
					},
					[value](const button&) -> option_value_t
					{
						if (value)
							throw_msg<option_error>("a button-type option does not accept a value");
						return std::monostate{};
					}
				};

				const auto internal_value = std::visit(visitor, option.type);
				option.apply_callback(internal_value);
			}
			catch (std::out_of_range)
			{
				throw_msg<option_error>("attempted to set non-existent option '{}'", name);
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
