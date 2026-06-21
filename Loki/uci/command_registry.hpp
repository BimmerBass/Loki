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
#include <memory>
#include "command.hpp"

namespace loki::uci
{
	/// <summary>
	/// Singleton registry over all commands, that the engine supports.
	/// </summary>
	class command_registry final
	{
		using factory_t = std::function<std::unique_ptr<i_uci_command>()>;
		using commands_t = std::vector<std::unique_ptr<i_uci_command>>;

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

		void add(factory_t f)
		{
			factories.push_back(f);
		}

		commands_t commands() const
		{
			commands_t commands;
			for (const auto& factory : factories)
				commands.push_back(factory());
			return commands;
		}

	private:
		std::vector<factory_t> factories;
	};

	template<typename T> requires std::is_base_of_v<i_uci_command, T>
	struct command_registration
	{
		command_registration()
		{
			command_registry::instance().add([]
				{
					return std::make_unique<T>();
				});
		}
	};

}