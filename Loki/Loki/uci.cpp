#include "uci.h"

namespace loki::uci
{

	engine_manager::engine_manager()
	{
		REGISTER_CALLBACK(parse_uci);
		REGISTER_CALLBACK(parse_debug);
		REGISTER_CALLBACK(parse_isready);
		REGISTER_CALLBACK(parse_setoption);
		REGISTER_CALLBACK(parse_register);
		REGISTER_CALLBACK(parse_ucinewgame);
		REGISTER_CALLBACK(parse_position);
		REGISTER_CALLBACK(parse_go);
		REGISTER_CALLBACK(parse_stop);
		REGISTER_CALLBACK(parse_ponderhit);
		REGISTER_CALLBACK(parse_quit);
	}

	bool engine_manager::run() noexcept
	{
		try
		{
			std::string command;

			while (std::getline(std::cin, command))
			{
				//parse_cmd(command);
				auto token_length = command.find_first_of(' ');
				auto first_token = textutil::lowercase(token_length == std::string::npos ?
					command : command.substr(0, token_length));

				if (m_callbackMap.find(first_token) != m_callbackMap.end())
					m_callbackMap[first_token](command);
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

	void engine_manager::parse_uci(const std::string& /* unused */)
	{
		std::cout << "id name " << NAME << " " << VERSION << "\n";
		std::cout << "id author " << AUTHOR << "\n";
		std::cout << "uciok\n";
	}

	void engine_manager::parse_isready(const std::string& uci)
	{
		std::cout << "readyok\n";
	}
}