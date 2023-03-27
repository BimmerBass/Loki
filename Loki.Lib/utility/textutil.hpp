#pragma once


namespace loki
{
	class textutil
	{
	public:

		inline static std::string lowercase(const std::string& caseSensitive)
		{
			std::string lc = "";
			for (auto c : caseSensitive)
				lc += (char)std::tolower(c);
			return lc;
		}

		inline static std::vector<std::string> split(const std::string& str, char delim = ' ')
		{
			std::vector<std::string> splitted;
			std::string tmp = "";

			for (auto c : str)
			{
				if (c != delim)
					tmp += c;
				else
				{
					if (!tmp.empty())
						splitted.push_back(tmp);
					tmp = "";
				}
			}
			if (!tmp.empty())
				splitted.push_back(tmp);

			return splitted;
		}

		inline static char toggle_case(char c)
		{
			return static_cast<char>(isupper(c) ? tolower(c) : toupper(c));
		}

		inline static std::string flip_fen(const std::string& fen)
		{
			std::string f, token;
			std::stringstream ss(fen);

			for (int i = 0; i < 8; i++)
			{
				std::getline(ss, token, i < 7 ? '/' : ' ');
				std::transform(token.begin(), token.end(), token.begin(), toggle_case);
				f.insert(0, token + (i ? "/" : " "));
			}

			ss >> token; // Side to move
			f += (token == "w" ? "b " : "w ");

			ss >> token; // Castling flags
			std::transform(token.begin(), token.end(), token.begin(), toggle_case);
			f += token + " ";

			ss >> token; // En-passant square
			f += (token == "-" ? token : token.replace(1, 1, token[1] == '3' ? "6" : "3"));

			std::getline(ss, token); // Full and half moves
			f += token;
			return f;
		}
	};
}
