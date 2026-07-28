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
#include <algorithm>
#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <format>
#include <regex>
#include <string_view>

namespace loki::options
{
	inline bool contains_reserved_token(
		const std::string_view value) noexcept
	{
		static const std::regex reserved_token{
			R"((^|\s)(name|value)(\s|$))"
		};
		return std::regex_search(
			value.cbegin(),
			value.cend(),
			reserved_token);
	}

	struct base_option
	{
		virtual ~base_option() = default;
		virtual void print(std::ostream& os) const noexcept = 0;
	};

	template<typename T>
	struct value_option : public base_option
	{
		using underlying_type_t = std::remove_cvref_t<T>;
		underlying_type_t default_value;

		value_option() = delete;
		value_option(underlying_type_t _default_value)
			: default_value{ _default_value }
		{}

		void print(std::ostream& os) const noexcept override
		{
			os << "default ";
			if constexpr (std::same_as<underlying_type_t, bool>)
			{
				os << (default_value ? "true" : "false");
			}
			else
			{
				os << default_value;
			}
		}

		virtual bool is_valid(
			const underlying_type_t& value) const noexcept = 0;
	};

	struct checkbox : public value_option<bool>
	{
		checkbox(bool _default) : value_option(_default) {}

		void print(std::ostream& os) const noexcept override
		{
			os << "type check ";
			value_option<bool>::print(os);
		}

		bool is_valid(const bool&) const noexcept override
		{
			return true;
		}
	};
	struct spin : public value_option<int64_t>
	{
		spin(int64_t _default, int64_t _min, int64_t _max)
			: value_option(_default), min{ _min }, max{ _max }
		{}
		int64_t min, max;

		void print(std::ostream& os) const noexcept override
		{
			os << "type spin ";
			value_option<int64_t>::print(os);
			os << std::format(" min {} max {}", min, max);
		}

		bool is_valid(const int64_t& value) const noexcept override
		{
			return value >= min && value <= max;
		}
	};
	struct combo : public value_option<std::string>
	{
		combo(std::string _default, std::vector<std::string> _values)
			: value_option(_default), values{ std::move(_values) }
		{}
		std::vector<std::string> values;

		void print(std::ostream& os) const noexcept override
		{
			os << "type combo ";
			value_option<std::string>::print(os);
			for (const auto& value : values)
			{
				os << " var " << value;
			}
		}

		bool is_valid(const std::string& value) const noexcept override
		{
			return !contains_reserved_token(value)
				&& std::ranges::find(values, value) != values.end();
		}
	};
	struct string : public value_option<std::string>
	{
		string(std::string _default) : value_option(_default) {}

		void print(std::ostream& os) const noexcept override
		{
			os << "type string ";
			value_option<std::string>::print(os);
		}

		bool is_valid(const std::string& value) const noexcept override
		{
			return !contains_reserved_token(value);
		}
	};

	struct button : public base_option
	{
		void print(std::ostream& os) const noexcept override
		{
			os << "type button";
		}
	};

	namespace types
	{
		template<typename T>
		struct value_option_t
		{
		private:
			template<typename U>
			static std::true_type test(const value_option<U>*);

			static std::false_type test(...);

			using candidate_t = std::remove_cvref_t<T>;

		public:
			static constexpr bool value =
				decltype(test(
					static_cast<const candidate_t*>(nullptr)
				))::value;
		};
	}

	template<typename T>
	concept value_option_t = types::value_option_t<T>::value;

	template<typename T>
	concept option_t =
		std::derived_from<
		std::remove_cvref_t<T>,
		base_option
		>;

	/// <summary>Option value type</summary>
	using option_value_t = std::variant <
		bool,			// checkbox
		int64_t,		// spin
		std::string,	// string and combo
		std::monostate	// button
	>;

	using option_type = std::variant<
		checkbox,
		spin,
		combo,
		string,
		button
	>;

	struct loki_option
	{
		std::string name;
		option_type type;
		std::function<void(const option_value_t)> apply_callback;

		void print(std::ostream& os) const noexcept
		{
			os << "option name " << name << " ";
			std::visit(
				[&](const auto& actual)
				{
					actual.print(os);
				}, type);
			os << std::endl;
		}
	};

}
