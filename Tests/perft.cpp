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
#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace loki::tests {

	TEST_CLASS(perft_test) 
	{
	public:
		TEST_METHOD(run_perft_tests) {
			read_perft_file();
			utility::perft perft_tester("");
			auto log_file = std::ofstream("perft_log.txt");

			for (const auto& tp : m_test_points) {
				perft_tester.load(tp.fen);

				for (const auto& node_count : tp.node_counts) {
					size_t nodes = perft_tester.perform(node_count.first, log_file, false);
					Assert::IsTrue(nodes == node_count.second);

					m_running_nps.push_back(perft_tester.previous_nps());
				}
			}

			// Write the average NPS to the log.
			std::erase_if(m_running_nps, [](const size_t& x) {return x == static_cast<size_t>(-1); });
			double running_avg = 0.0;
			double count = static_cast<double>(m_running_nps.size());

			for (auto nps : m_running_nps) {
				//running_avg = ((count / (count + 1.0)) * (running_avg / (count + 1.0))) + (nps / (count + 1.0));
				running_avg += nps;
			}
			running_avg /= count;

			log_file << "\n\tRunning average NPS: " << running_avg << std::endl;
			log_file.close();
		}

	private:
		struct perft_test_point {
			std::string fen = "";
			std::vector<std::pair<DEPTH, size_t>> node_counts{};
		};
		std::vector<perft_test_point>	m_test_points;
		std::vector<size_t>				m_running_nps;

		void read_perft_file() {
			auto perft_file = std::ifstream("data/perft.fen");
			Assert::IsTrue(perft_file.is_open());
			std::string current_line;

			while (std::getline(perft_file, current_line)) {
				m_test_points.push_back(parse_perft_line(current_line));
			}
			perft_file.close();
		}

		perft_test_point parse_perft_line(std::string line) {
			auto semicolon_sep = split_string(line);
			Assert::IsTrue(semicolon_sep.size() > 1);

			perft_test_point tp;

			tp.fen = semicolon_sep[0];

			for (auto node_count_idx = 1; node_count_idx < semicolon_sep.size(); node_count_idx++) {
				auto node_count = semicolon_sep[node_count_idx];
				trim(node_count);
				auto depth_and_count = split_string(node_count, ' ');

				tp.node_counts.push_back(
					std::make_pair(
						static_cast<DEPTH>(std::stoi(depth_and_count[0].substr(1))),
						std::stoi(depth_and_count[1])));
			}

			return tp;
		}

		static std::vector<std::string> split_string(std::string str, char delim = ';') {
			std::vector<std::string> result;

			while (str.find_first_of(delim) != std::string::npos) {
				result.push_back(str.substr(0, str.find_first_of(delim)));
				str = str.substr(str.find_first_of(delim) + 1);
			}
			if (str != "") {
				result.push_back(str);
			}

			return result;
		}

		// trim from start (in place)
		static inline void ltrim(std::string& s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
				return !std::isspace(ch);
				}));
		}

		// trim from end (in place)
		static inline void rtrim(std::string& s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
				return !std::isspace(ch);
				}).base(), s.end());
		}

		// trim from both ends (in place)
		static inline void trim(std::string& s) {
			ltrim(s);
			rtrim(s);
		}
	};

}