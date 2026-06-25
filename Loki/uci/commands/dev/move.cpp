#include "uci/command_registry.hpp"
#include "util/exception.hpp"
#include "uci/uci_parser.hpp"
#include "uci/move_parsing.hpp"

#ifdef LOKI_ENABLE_DEV_COMMANDS

using namespace loki;
using namespace loki::uci;

class move_command final : public uci_command<move_command>
{
public:
	move_command()
	{
		position_command = command_registry::instance().create("position");

		if (!position_command)
			throw_msg<loki_exception>("ERROR: position command needs to be registered for move command to work!");
	}

	static std::string name() { return "move"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string> arguments, context* ctx) override
	{
		if (arguments.empty())
			throw_msg<uci_parser::uci_error>("move cannot have zero arguments");

		const auto& gs = ctx->engine.position()->make_view()->game_state();
		auto move = parse_move_token(arguments[0], *gs);
		if (!ctx->engine.position()->make_move(move))
			throw_msg<uci_parser::uci_error>("move '{}' is illegal in the current position", move.to_string());
	}
	
private:
	std::unique_ptr<i_uci_command> position_command;
};

static command_registration<move_command> reg;

#endif
