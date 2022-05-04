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
#include "tokenizer.h"

#include <regex>
#include <functional>
#include <unordered_map>

namespace loki::uci {
	
	/// <summary>
	/// Apply a regular expression matching words and FEN's, and return these tokens.
	/// </summary>
	/// <returns></returns>
	std::vector<std::string> tokenizer::apply_regex() const {
		std::regex regex(m_uci_regex, std::regex_constants::ECMAScript);
		auto tokens_begin = std::sregex_iterator(m_command.begin(), m_command.end(), regex);
		auto tokens_end = std::sregex_iterator();

		// Extract tokens.
		std::vector<std::string> str_tokens;
		std::transform(
			tokens_begin,
			tokens_end,
			std::back_inserter(str_tokens),
			[](const std::sregex_iterator& i) {
				auto match = *i;
				return match.str();
			});

		return str_tokens;
	}

	namespace {
		// All command types that Loki accepts.
		enum command_types_t {
			CT_uci = 0,
			CT_isready,
			CT_setoption,
			CT_ucinewgame,
			CT_position,
			CT_go,
			CT_stop,
			CT_ponderhit,	// Not implemented yet.
			CT_quit
		};

		std::unordered_map<std::string, command_tyå

		template<command_types_t CT>
		void tokenize(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {
			return; // Don't do anything by default.
		}

		template<>
		void tokenize<CT_uci>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
		template<>
		void tokenize<CT_isready>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
		template<>
		void tokenize<CT_setoption>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
		template<>
		void tokenize<CT_ucinewgame>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
		template<>
		void tokenize<CT_position>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
		template<>
		void tokenize<CT_go>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
		template<>
		void tokenize<CT_stop>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
		template<>
		void tokenize<CT_quit>(tokenizer::tokens_t& tokens, const std::vector<std::string>& str_tokens) {

		}
	}

	/// <summary>
	/// Parse a UCI command.
	/// </summary>
	/// <param name="uci_cmd"></param>
	/// <returns></returns>
	const tokenizer::tokens_t& tokenizer::operator<<(const std::string& uci_cmd) {
		m_command = uci_cmd;
		m_tokens.clear();

		apply_regex();
		
		return m_tokens;
	}
}