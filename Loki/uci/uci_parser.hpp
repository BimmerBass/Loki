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
#include <functional>
#include <map>
#include <memory>

#include "util/exception.hpp"
#include "command.hpp"
#include "context.hpp"

namespace loki::uci
{
	class uci_parser final : public context
	{
	public:
		CHILD_EXCEPTION(uci_error, loki_exception);
	private:
		using tokens_t = std::vector<std::string>;
		
		std::map<std::string, std::unique_ptr<i_uci_command>> m_handlers;
	public:
		uci_parser();

		/// <summary>
		/// Main loop which implements the UCI protocol.
		/// </summary>
		/// <returns>zero if no error occurs. non-zero otherwise.</returns>
		int uci_loop() noexcept;

	private:
		std::vector<std::string> tokenize(std::string command);
	};
}