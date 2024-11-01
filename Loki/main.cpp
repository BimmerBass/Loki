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
#include <iostream>
#include <memory>

#include "uci/loki_context.hpp"
#include "uci/uci_parser.hpp"

using namespace loki::uci;

int main()
{
	std::shared_ptr<context_interface> ctx = std::make_shared<loki_context>(std::cout);
	auto uci = std::make_unique<uci_parser>(ctx);
	return uci->uci_loop();
}