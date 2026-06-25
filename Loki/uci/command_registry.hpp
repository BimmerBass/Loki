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

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "command.hpp"

namespace loki::uci
{
	/// <summary>
	/// Singleton registry over all commands, that the engine supports.
	/// </summary>
	class command_registry final
	{
		using command_t = std::unique_ptr<i_uci_command>;
		using factory_t = std::function<command_t()>;
		using commands_t = std::vector<command_t>;

		command_registry() = default;
	public:
		command_registry(const command_registry&) = delete;
		command_registry(command_registry&&) = delete;

		command_registry& operator=(const command_registry&) = delete;
		command_registry& operator=(command_registry&&) = delete;

		static command_registry& instance()
		{
			static command_registry instance{};
			return instance;
		}

		void add(std::string name, factory_t f)
		{
			factories.emplace(std::move(name), std::move(f));
		}

		command_t create(const std::string& name) const
		{
			const auto it = factories.find(name);
			if (it == factories.end())
				return nullptr;
			return it->second();
		}

		commands_t commands() const
		{
			commands_t commands;
			for (const auto& [_, factory] : factories)
				commands.push_back(factory());
			return commands;
		}

	private:
		std::map<std::string, factory_t> factories;
	};

	template<typename T>
	concept uci_command_type =
		std::is_base_of_v<i_uci_command, T>
		&& requires
	{
		{ T::name() } -> std::convertible_to<std::string_view>;
	};

	template<uci_command_type T>
	struct command_registration
	{
		command_registration()
		{
			command_registry::instance().add(std::string(T::name()), []
				{
					return std::make_unique<T>();
				});
		}
	};

}
