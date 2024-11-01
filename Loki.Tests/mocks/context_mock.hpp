#pragma once
#include <gmock/gmock.h>
#include "Loki/uci/context_interface.hpp"
#include "Loki/position/game_state.cpp"

namespace uci_tests
{
	class context_mock : public loki::uci::context_interface
	{
	public:
		MOCK_METHOD(void, uci, (), (const, override));
		MOCK_METHOD(void, debug, (), (const, override));
		MOCK_METHOD(void, isready, (), (const, override));
		MOCK_METHOD(void, setoption, (std::string name, std::optional<std::string> value), (override));
		MOCK_METHOD(void, register_, (), (const, override));
		MOCK_METHOD(void, ucinewgame, (), (override));
		MOCK_METHOD(void, position, (const std::string& newState, const std::vector<std::string>& moves), (override));
		MOCK_METHOD(void, go, (const loki::search::limits* limits), (override));
		MOCK_METHOD(void, stop, (), (override));
		MOCK_METHOD(void, ponderhit, (), (override));
	};
}