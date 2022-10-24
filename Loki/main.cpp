#include "Backend/loki.pch.h"
#include <regex>
#include <iostream>

int main()
{
	//std::string m_regex = "(\\s*([rnbqkpRNBQKP1-8]+\\/){7}([rnbqkpRNBQKP1-8]+)\\s[bw-]\\s(([a-hkqA-HKQ]{1,4})|(-))\\s(([a-h][36])|(-))\\s\\d+\\s\\d+\\s*)|([^\\s]+)";
	std::string m_regex = "(([rnbqkpRNBQKP1-8]+\\/){7}([rnbqkpRNBQKP1-8]+)\\s[bw-]\\s(([a-hkqA-HKQ]{1,4})|(-))\\s(([a-h][36])|(-))\\s\\d+\\s\\d+\\s*)|([^\\s]+)";
	std::regex self_regex(m_regex, std::regex_constants::ECMAScript);

	std::vector<std::string> commands = {
		"position startpos",
		"position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 moves e2e4 e7e5 g1f3",
		"uci",
		"setoption name Hash value 32",
		"setoption name NalimovCache value 1",
		"setoption name NalimovPath value d:\\tb;c\\tb",
		"ucinewgame",
		"setoption name UCI_AnalyseMode value true",
		"go infinite",
		"stop"
	};

	for (auto str : commands)
	{
		std::cout << "Matches for '" << str << "':\n";

		auto tokens_begin = std::sregex_iterator(str.begin(), str.end(), self_regex);
		auto tokens_end = std::sregex_iterator();

		int j = 1;
		for (std::sregex_iterator i = tokens_begin; i != tokens_end; ++i)
		{
			std::smatch match = *i;
			std::string m_str = match.str();

			std::cout << "[" << j << "]: " << m_str << "\n";
		}
		std::cout << std::endl;
	}

	/*std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	std::cout << "Starting perft\n";

	using namespace loki;
	auto m_pos = position::position::create_position(
		std::make_shared<position::game_state>(),
		std::make_shared<movegen::magics::slider_generator>());
	*m_pos << start_fen;
	std::cout << m_pos << std::endl;

	loki::utility::perft pft(start_fen);

	pft.perform(loki::DEPTH(6), std::cout, true);*/
}