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
				lc += std::tolower(c);
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
			return splitted;
		}
	};
}