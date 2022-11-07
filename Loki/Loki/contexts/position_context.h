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

	public:
		position_context(std::string parms) : context(POSITION, parms)
		{
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
			
			if ((pos = parms.find(" moves ")) != std::string::npos)
			{
				auto moves_start = pos + 7;
				moves = textutil::split(parms.substr(moves_start));
			}
		}

		/// <summary>
		/// Create N position objects. (One for each thread)
		/// </summary>
		std::vector<position::position_t> create_positions(size_t n)
		{
			if (n <= 0)
				throw e_positionContext(FORMAT_EXCEPTION_MESSAGE("create_position was called with an invalid amount of positions to create."));
			std::vector<position::position_t> positions;
			auto sg = std::make_shared<movegen::magics::slider_generator>();
			for (auto i = 0; i < n; i++)
			{
				auto game_state = std::make_unique<position::game_state>();
				*game_state << fen;
				
				positions.emplace_back(position::position::create_position(game_state, sg));

				move_t move;
				for (auto& strMove : moves)
				{
					auto& movelist = positions.back()->generate_moves();
					if ((move = movelist.find(strMove)) != MOVE_NULL)
						positions.back()->make_move(move);
				}
			}
			return positions;
		}
	};
}