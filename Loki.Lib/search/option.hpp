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

namespace loki::search
{

	class options_manager
	{
		EXCEPTION_CLASS(e_optionsManager, e_lokiError);
	private:
		
		class _IOption
		{
		protected:
			EXCEPTION_CLASS(e_optionError, e_optionsManager);
		public:
			virtual ~_IOption() = default;
			virtual void executeCommand(const std::string& cmd) const = 0;
			virtual std::string get_typeString() const = 0;
		};
		class _SpinOption : public _IOption
		{
		private:
			std::function<void(int)> m_callback;
			int m_defaultVal, m_min, m_max;
		public:
			_SpinOption() = delete;
			_SpinOption(int defaultVal, int min, int max, std::function<void(int)>&& callback)
				:  m_callback(callback), m_defaultVal(defaultVal), m_min(min), m_max(max)
			{}

			void executeCommand(const std::string& cmd) const override
			{
				try
				{
					auto val = std::stoi(cmd);
					if (val < m_min)
						throw std::out_of_range("Value specified was lower than the minimum value the application supports");
					if (val > m_max)
						throw std::out_of_range("Value specified was higher than the maximum value the application supports");

					m_callback(val);
				}
				catch (std::invalid_argument& e)
				{
					throw e_optionError(std::string("Error while executing spin option (std::invalid_argument): ") + e.what());
				}
				catch (std::out_of_range& e)
				{
					throw e_optionError(std::string("Error while executing spin option (std::out_of_range): ") + e.what());
				}
			}
			std::string get_typeString() const override
			{
				return std::format("type spin default {} min {} max {}", m_defaultVal, m_min, m_max);
			}
		};

	private:
		// Since _IOption is abstract, we need to store pointers instead of instances.
		using option_t = std::unique_ptr<_IOption>;
		std::map<std::string, option_t> m_optionsMap;
	public:
		void register_option(const std::string& optionName, int defaultVal, int min, int max, std::function<void(int)>&& func)
		{
			if (m_optionsMap.find(optionName) != m_optionsMap.end())
				throw e_optionsManager(std::format("Key '{}' was already registered as an option!", optionName));
			m_optionsMap[optionName] = std::make_unique<_SpinOption>(defaultVal, min, max, std::move(func));
		}

		void perform(const std::string& name, const std::string& value) const
		{
			if (m_optionsMap.find(name) != m_optionsMap.end())
				m_optionsMap.at(name)->executeCommand(value);
		}

		std::vector<std::string> option_strings_for_gui() const
		{
			std::vector<std::string> obj;
			for (auto i = m_optionsMap.cbegin(); i != m_optionsMap.cend(); i++)
				obj.push_back(std::format("option name {} {}", (*i).first, (*i).second->get_typeString()));
			return obj;
		}

	};
}