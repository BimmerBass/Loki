#include "move_parsing.hpp"
#include "uci_parser.hpp"

using namespace loki;
using namespace loki::uci;

namespace
{
	using loki::movegen::move;
	using loki::movegen::move_attr;

	move_attr infer_move_attr(const position::game_state& state, const move& candidate)
	{
		side mover = NUM_SIDES;
		const auto moved_piece = state.get_piece(position::square(candidate.from()), &mover);
		if (moved_piece == NO_PIECE || mover == NUM_SIDES)
			throw_msg<uci_parser::uci_error>("invalid move: no piece exists on '{}'", position::square(candidate.from()).to_algebraic());
		if (mover != state.side_to_move)
			throw_msg<uci_parser::uci_error>("invalid move: no {} piece exists on '{}'", enum_to_string(state.side_to_move), position::square(candidate.from()).to_algebraic());

		if (candidate.type() == loki::movegen::PROMOTION)
			return loki::movegen::PROMOTION;

		if (moved_piece == PAWN && state.en_passant_sq == candidate.to())
		{
			const auto dest_piece = state.get_piece(position::square(candidate.to()));
			if (dest_piece == NO_PIECE)
				return loki::movegen::ENPASSANT;
		}

		if (moved_piece == KING
			&& position::rank_of(candidate.from()) == position::rank_of(candidate.to())
			&& std::abs(static_cast<int>(position::file_of(candidate.to())) - static_cast<int>(position::file_of(candidate.from()))) == 2)
			return loki::movegen::CASTLING;

		return loki::movegen::NORMAL;
	}
}

move loki::uci::parse_move_token(const std::string& token, const position::game_state& state)
{
	if (token.size() < 4 || token.size() > 5)
		throw_msg<uci_parser::uci_error>("invalid move token: '{}'", token);

	const position::square from_sq(token.substr(0, 2));
	const position::square to_sq(token.substr(2, 2));

	auto type = loki::movegen::NORMAL;
	auto promotion_piece = KNIGHT;
	if (token.size() == 5)
	{
		type = loki::movegen::PROMOTION;
		switch (std::tolower(static_cast<unsigned char>(token[4])))
		{
		case 'n': promotion_piece = KNIGHT; break;
		case 'b': promotion_piece = BISHOP; break;
		case 'r': promotion_piece = ROOK; break;
		case 'q': promotion_piece = QUEEN; break;
		default:
			throw_msg<uci_parser::uci_error>("invalid promotion piece in move token: '{}'", token);
		}
	}

	move candidate(from_sq.value(), to_sq.value(), type, promotion_piece);
	candidate.type(infer_move_attr(state, candidate));
	return candidate;
}