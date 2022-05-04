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
#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <vector>
#include <string>
#include <any>

namespace loki::uci {
	
	/// <summary>
	/// Tokenize a UCI command.
	/// </summary>
	class tokenizer {
	public:
		using token_t = std::pair<std::string, std::any>;
		using tokens_t = std::vector<token_t>;
	private:
		// The uci_regex will split a uci command into whitespace-separated parts, but keep an FEN intact.
		const std::string m_uci_regex
			= "(([rnbqkpRNBQKP1-8]+\\/){7}([rnbqkpRNBQKP1-8]+)\\s[bw-]\\s(([a-hkqA-HKQ]{1,4})|(-))\\s(([a-h][36])|(-))\\s\\d+\\s\\d+\\s*)|([^\\s]+)";
		std::string m_command;			/* The full command. */
		std::string m_command_type;		/* The type of command received (uci, isready, setoption, ucinewgame, position, go, stop, ponderhit, quit)*/
		tokens_t	m_tokens;			/* The tokens following the command type. */
	public:
		/// <summary>
		/// Extract all tokens from a UCI command.
		/// </summary>
		/// <param name="uci_cmd"></param>
		/// <returns></returns>
		const tokens_t& operator<<(const std::string& uci_cmd);

	private:
		/// <summary>
		/// Apply a regular expression that matches space-separated words and FEN's.
		/// </summary>
		std::vector<std::string> apply_regex() const;
	};
}


#endif