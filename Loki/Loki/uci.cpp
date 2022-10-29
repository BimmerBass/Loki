#include "uci.h"

namespace loki::uci
{

	bool engine_manager::run() noexcept
	{
		try
		{
			std::string command;

			while (std::getline(std::cin, command))
			{
				parse_cmd(command);
			}
		}
		catch (std::exception& e)
		{
			// Output the exception message as a UCI info command.
			std::cout << "info string " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	void engine_manager::parse_cmd(const std::string& cmd)
	{
		// Extract command type.
		auto token_length = cmd.find_first_of(' ');
		auto first_token = textutil::lowercase(token_length == std::string::npos ?
			cmd : cmd.substr(0, token_length));
		
		command_type cmd_type = UCI_NONE;
		if (command_map.find(first_token) != command_map.end())
			cmd_type = command_map.at(first_token);
	}
}