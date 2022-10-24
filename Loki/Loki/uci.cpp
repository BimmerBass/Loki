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

	engine_manager::command_type engine_manager::get_type(std::string cmd)
	{
		auto token_length = cmd.find_first_of(' ');
		auto token = token_length == std::string::npos ?
			cmd : cmd.substr(0, token_length);
	}
}