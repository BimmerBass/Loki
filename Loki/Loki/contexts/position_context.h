#pragma once
#include "uci_context.h"

namespace loki::uci
{
	class position_context : public context
	{
		EXCEPTION_CLASS(e_positionContext, e_uciError);
	public:
		static constexpr const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
	private:
		std::string fen;
		std::vector<std::string> moves;
		position::position_t position;

	public:
		position_context(std::string parms) : context(POSITION, parms)
		{
			auto game_state = std::make_shared<position::game_state>();

			size_t pos = std::string::npos;
			if (parms.find("startpos") != std::string::npos)
				fen = START_FEN;
			else if ((pos = parms.find("fen ")) != std::string::npos)
			{
				auto fen_start = pos + 4;
				auto fen_end = parms.find(" moves ");
				fen = parms.substr(fen_start, fen_end - fen_start);
			}
			else
				throw e_positionContext(FORMAT_EXCEPTION_MESSAGE("Neither 'startpos' nor 'fen' was specified in 'position' command."));
			(*game_state) << fen;
			position = position::position::create_position(game_state, std::make_shared<movegen::magics::slider_generator>());

			if ((pos = parms.find(" moves ")) != std::string::npos)
			{
				auto moves_start = pos + 7;
				moves = textutil::split(parms.substr(moves_start));
			}

			for (auto& m : moves)
			{
				auto& pseudoMoves = position->generate_moves<movegen::ALL>();
				move_t move;
				if ((move = pseudoMoves.find(m)) != MOVE_NULL)
					position->make_move(move);
			}

			std::cout << position << std::endl;
		}
	};
}