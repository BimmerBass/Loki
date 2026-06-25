#include "uci/command_registry.hpp"
#include "uci/uci_parser.hpp"

#ifdef LOKI_ENABLE_DEV_COMMANDS

using namespace loki;
using namespace loki::uci;

class undo : public uci_command<undo>
{
public:
	static std::string name() { return "undo"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string>, context* ctx) override
	{
		if (!ctx->engine.position()->has_made_move())
			throw_msg<uci_parser::uci_error>("no move has been made in the current position object");
		ctx->engine.position()->undo_last_move();
	}
};

static command_registration<undo> reg;

#endif