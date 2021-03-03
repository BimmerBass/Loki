#include "movegen.h"



namespace moveGen {
	template <MoveType type, SIDE me>
	void generate_pawn_moves(GameState_t* pos, MoveList* move_list) {

		constexpr DIRECTION Up = (me == WHITE) ? NORTH : SOUTH;
		constexpr DIRECTION upLeft = (me == WHITE) ? NORTHWEST : SOUTHWEST;
		constexpr DIRECTION upRight = (me == WHITE) ? NORTHEAST : SOUTHEAST;
		constexpr int right_attack_origin = (me == WHITE) ? 9 : -7;
		constexpr int left_attack_origin = (me == WHITE) ? 7 : -9;

		constexpr Bitboard NotRankEight = (me == WHITE) ? ~BBS::RankMasks8[RANK_8] : ~BBS::RankMasks8[RANK_1];
		constexpr Bitboard RankThree = (me == WHITE) ? BBS::RankMasks8[RANK_3] : BBS::RankMasks8[RANK_6];

		Bitboard pawnBrd = pos->pieceBBS[PAWN][me];

		// We don't want to create impossible moves.
		if (pawnBrd == 0) {
			return;
		}

		Bitboard OPPONENT_PIECES = (me == WHITE) ? pos->all_pieces[BLACK] : pos->all_pieces[WHITE];

		Bitboard OCCUPIED = (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]);

		int index = 0;

		// The only quiet moves pawns make are one square up or two squares up.
		if constexpr (type == QUIET) {
			/*
			We don't want to generate promotions here, so we'll remove the eighth and first rank from the toSq boards.
			*/
			Bitboard one_up = (shift<Up>(pawnBrd) & ~OCCUPIED) & NotRankEight;


			// For two up, we'll only use the pawns that have been shifted up one square to rank three since we know that they have then originated on rank two.
			Bitboard two_up = shift<Up>(one_up & RankThree) & ~OCCUPIED;

			while (one_up != 0) {
				index = PopBit(&one_up);

				move_list->add_move(index, (me == WHITE ? index - 8 : index + 8), 0, NOT_SPECIAL);
			}

			while (two_up != 0) {
				index = PopBit(&two_up);

				move_list->add_move(index, (me == WHITE ? index - 16 : index + 16), 0, NOT_SPECIAL);
			}

			return;
		}


		// Here we'll generate all captures and promotions.
		else if constexpr (type == CAPTURES) {

			Bitboard left_attacks = shift<upLeft>(pawnBrd) & OPPONENT_PIECES;
			Bitboard right_attacks = shift<upRight>(pawnBrd) & OPPONENT_PIECES;

			Bitboard regular_promotions = shift<Up>(pawnBrd) & ~NotRankEight & ~OCCUPIED;

			while (left_attacks) {
				index = PopBit(&left_attacks);

				// If we are on the respective eighth ranks (rank 1 for black), we'll have to insert promotions.
				if ((me == WHITE && (index >= 56 && index <= 63)) || (me == BLACK && (index >= 0 && index <= 7))) {
					move_list->add_move(index, index - left_attack_origin, 0, PROMOTION);
					move_list->add_move(index, index - left_attack_origin, 1, PROMOTION);
					move_list->add_move(index, index - left_attack_origin, 2, PROMOTION);
					move_list->add_move(index, index - left_attack_origin, 3, PROMOTION);
				}

				else {
					move_list->add_move(index, index - left_attack_origin, 0, NOT_SPECIAL);
				}

			}

			while (right_attacks) {
				index = PopBit(&right_attacks);

				// If we are on the respective eighth ranks (rank 1 for black), we'll have to insert promotions.
				if ((me == WHITE && (index >= 56 && index <= 63)) || (me == BLACK && (index >= 0 && index <= 7))) {
					move_list->add_move(index, index - right_attack_origin, 0, PROMOTION);
					move_list->add_move(index, index - right_attack_origin, 1, PROMOTION);
					move_list->add_move(index, index - right_attack_origin, 2, PROMOTION);
					move_list->add_move(index, index - right_attack_origin, 3, PROMOTION);
				}

				else {
					move_list->add_move(index, index - right_attack_origin, 0, NOT_SPECIAL);
				}
			}

			/*
			Now we have to resolve regular promotions. These are easy.
			*/
			while (regular_promotions) {
				index = PopBit(&regular_promotions);

				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 0, PROMOTION);
				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 1, PROMOTION);
				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 2, PROMOTION);
				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 3, PROMOTION);
			}


			// Now we'll have to add en-passant, if there is any:
			if (pos->enPasSq != NO_SQ) {
				Bitboard epBrd = uint64_t(1) << pos->enPasSq;
				
				if constexpr (me == WHITE) {
					if (((epBrd & ~BBS::FileMasks8[FILE_A]) >> 9) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq - 9, 0, ENPASSANT);
					}
					if (((epBrd & ~BBS::FileMasks8[FILE_H]) >> 7) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq - 7, 0, ENPASSANT);
					}
				}
				else {
					if (((epBrd & ~BBS::FileMasks8[FILE_A]) << 7) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq + 7, 0, ENPASSANT);
					}
					if (((epBrd & ~BBS::FileMasks8[FILE_H]) << 9) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq + 9, 0, ENPASSANT);
					}
				}
			}

			return;
		}

		else {
			Bitboard one_up = (shift<Up>(pawnBrd) & ~OCCUPIED) & NotRankEight;

			// For two up, we'll only use the pawns that have been shifted up one square to rank three since we know that they have then originated on rank two.
			Bitboard two_up = shift<Up>(one_up & RankThree) & ~OCCUPIED;

			while (one_up != 0) {
				index = PopBit(&one_up);

				move_list->add_move(index, (me == WHITE ? index - 8 : index + 8), 0, NOT_SPECIAL);
			}

			while (two_up != 0) {
				index = PopBit(&two_up);

				move_list->add_move(index, (me == WHITE ? index - 16 : index + 16), 0, NOT_SPECIAL);
			}
			


			Bitboard left_attacks = shift<upLeft>(pawnBrd) & OPPONENT_PIECES;
			Bitboard right_attacks = shift<upRight>(pawnBrd) & OPPONENT_PIECES;

			Bitboard regular_promotions = shift<Up>(pawnBrd) & ~NotRankEight & ~OCCUPIED;

			while (left_attacks) {
				index = PopBit(&left_attacks);

				// If we are on the respective eighth ranks (rank 1 for black), we'll have to insert promotions.
				if ((me == WHITE && (index >= 56 && index <= 63)) || (me == BLACK && (index >= 0 && index <= 7))) {
					move_list->add_move(index, index - left_attack_origin, 0, PROMOTION);
					move_list->add_move(index, index - left_attack_origin, 1, PROMOTION);
					move_list->add_move(index, index - left_attack_origin, 2, PROMOTION);
					move_list->add_move(index, index - left_attack_origin, 3, PROMOTION);
				}

				else {
					move_list->add_move(index, index - left_attack_origin, 0, NOT_SPECIAL);
				}

			}

			while (right_attacks) {
				index = PopBit(&right_attacks);

				// If we are on the respective eighth ranks (rank 1 for black), we'll have to insert promotions.
				if ((me == WHITE && (index >= 56 && index <= 63)) || (me == BLACK && (index >= 0 && index <= 7))) {
					move_list->add_move(index, index - right_attack_origin, 0, PROMOTION);
					move_list->add_move(index, index - right_attack_origin, 1, PROMOTION);
					move_list->add_move(index, index - right_attack_origin, 2, PROMOTION);
					move_list->add_move(index, index - right_attack_origin, 3, PROMOTION);
				}

				else {
					move_list->add_move(index, index - right_attack_origin, 0, NOT_SPECIAL);
				}
			}

			/*
			Now we have to resolve regular promotions. These are easy.
			*/
			while (regular_promotions) {
				index = PopBit(&regular_promotions);

				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 0, PROMOTION);
				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 1, PROMOTION);
				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 2, PROMOTION);
				move_list->add_move(index, ((me == WHITE) ? index - 8 : index + 8), 3, PROMOTION);
			}


			// Now we'll have to add en-passant, if there is any:
			if (pos->enPasSq != NO_SQ) {
				Bitboard epBrd = uint64_t(1) << pos->enPasSq;

				if constexpr (me == WHITE) {
					if (((epBrd & ~BBS::FileMasks8[FILE_A]) >> 9) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq - 9, 0, ENPASSANT);
					}
					if (((epBrd & ~BBS::FileMasks8[FILE_H]) >> 7) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq - 7, 0, ENPASSANT);
					}
				}
				else {
					if (((epBrd & ~BBS::FileMasks8[FILE_A]) << 7) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq + 7, 0, ENPASSANT);
					}
					if (((epBrd & ~BBS::FileMasks8[FILE_H]) << 9) & pawnBrd) {
						move_list->add_move(pos->enPasSq, pos->enPasSq + 9, 0, ENPASSANT);
					}
				}
			}

		}

		return;
	}


	template<MoveType type, SIDE me>
	void generate_knight_moves(GameState_t* pos, MoveList* move_list) {

		Bitboard knightBrd = pos->pieceBBS[KNIGHT][me];

		if (knightBrd == 0) {
			return;
		}

		Bitboard opponent_pieces = (me == WHITE) ? pos->all_pieces[BLACK] : pos->all_pieces[WHITE];
		Bitboard friendly_pieces = pos->all_pieces[me];

		int index = 0;
		int attack_sq = 0;
		Bitboard attack_board = 0;

		if constexpr (type == QUIET) {

			while (knightBrd) {
				index = PopBit(&knightBrd);

				attack_board = BBS::knight_attacks[index] & ~(friendly_pieces | opponent_pieces);

				while (attack_board) {
					attack_sq = PopBit(&attack_board);

					move_list->add_move(attack_sq, index, 0, NOT_SPECIAL);
				}
			}

			return;
		}

		
		else if constexpr (type == CAPTURES) {
			while (knightBrd) {
				index = PopBit(&knightBrd);

				// For captures, the only destinations we are interested in, are the ones where there are enemy pieces.
				attack_board = BBS::knight_attacks[index] & opponent_pieces;

				while (attack_board) {
					attack_sq = PopBit(&attack_board);

					move_list->add_move(attack_sq, index, 0, NOT_SPECIAL);
				}
			}
		}

		// For knights, if it isn't captures or quiets, it is ALL
		else {

			while (knightBrd) {
				index = PopBit(&knightBrd);

				// Here we only remove the friendly pieces, that the knight can attack
				attack_board = BBS::knight_attacks[index] & ~friendly_pieces;

				while (attack_board) {
					attack_sq = PopBit(&attack_board);

					move_list->add_move(attack_sq, index, 0, NOT_SPECIAL);
				}
			}
			
			return;
		}
	}


	template <piece pce, MoveType type, SIDE me>
	void generate_piece_moves(GameState_t* pos, MoveList* move_list) {
		//Bitboard sliderBrd = (me == WHITE) ? pos->pieceBBS[pce][WHITE] : pos->pieceBBS[pce][BLACK];
		Bitboard sliderBrd = pos->pieceBBS[pce][me];

		if (sliderBrd == 0) {
			return;
		}

		Bitboard opponent_pieces = (me == WHITE) ? pos->all_pieces[BLACK] : pos->all_pieces[WHITE];
		Bitboard friendly_pieces = pos->all_pieces[me];
		Bitboard occupied = opponent_pieces | friendly_pieces;

		int sq = 0;
		int dest = 0;
		Bitboard attacks = 0;

		if constexpr (type == QUIET) {
			while (sliderBrd) {
				sq = PopBit(&sliderBrd);

				// If we are looking for quiet moves, we don't want to land on any types of pieces.
				attacks = (Magics::attacks_bb<pce>(sq, occupied)) & ~occupied;

				while (attacks) {
					dest = PopBit(&attacks);

					move_list->add_move(dest, sq, 0, NOT_SPECIAL);
				}
			}
			return;
		}

		else if constexpr (type == CAPTURES) {
			while (sliderBrd) {
				sq = PopBit(&sliderBrd);

				// If we are looking for captures, we only want to land on opponent pieces.
				attacks = (Magics::attacks_bb<pce>(sq, occupied)) & opponent_pieces;

				while (attacks) {
					dest = PopBit(&attacks);

					move_list->add_move(dest, sq, 0, NOT_SPECIAL);
				}
			}
			return;
		}


		else { // ALL
			while (sliderBrd) {
				sq = PopBit(&sliderBrd);

				// If we are looking for all moves, we want to land on everything except our own pieces.
				attacks = (Magics::attacks_bb<pce>(sq, occupied)) & ~friendly_pieces;

				while (attacks) {
					dest = PopBit(&attacks);

					move_list->add_move(dest, sq, 0, NOT_SPECIAL);
				}
			}
			return;
		}
	}


	template <SIDE me>
	void gen_castle_moves(GameState_t* pos, MoveList* move_list) {

		constexpr Bitboard key_squares_kingside = (BBS::RankMasks8[(me == WHITE) ? RANK_1 : RANK_8] & (BBS::FileMasks8[FILE_G] | BBS::FileMasks8[FILE_F]));
		constexpr Bitboard key_squares_queenside = (BBS::RankMasks8[(me == WHITE) ? RANK_1 : RANK_8] & (BBS::FileMasks8[FILE_C] | BBS::FileMasks8[FILE_D]
			| BBS::FileMasks8[FILE_B]));

		Bitboard friendly_pieces = pos->all_pieces[me];
		Bitboard opponent_pieces = pos->all_pieces[(me == WHITE) ? BLACK : WHITE];

		if constexpr (me == WHITE) {
			if (pos->can_castle<WKCA>() && ((friendly_pieces | opponent_pieces) & key_squares_kingside) == 0
				&& !pos->square_attacked(E1, BLACK) && !pos->square_attacked(F1, BLACK) && !pos->square_attacked(G1, BLACK)) {
				move_list->add_move(G1, E1, 0, CASTLING);
			}

			if (pos->can_castle<WQCA>() && ((friendly_pieces | opponent_pieces) & key_squares_queenside) == 0
				&& !pos->square_attacked(E1, BLACK) && !pos->square_attacked(D1, BLACK) && !pos->square_attacked(C1, BLACK)) {
				move_list->add_move(C1, E1, 0, CASTLING);
			}
		}
		else {
			if (pos->can_castle<BKCA>() && ((friendly_pieces | opponent_pieces) & key_squares_kingside) == 0
				&& !pos->square_attacked(E8, WHITE) && !pos->square_attacked(F8, WHITE) && !pos->square_attacked(G8, WHITE)) {
				move_list->add_move(G8, E8, 0, CASTLING);
			}
			if (pos->can_castle<BQCA>() && ((friendly_pieces | opponent_pieces) & key_squares_queenside) == 0
				&& !pos->square_attacked(E8, WHITE) && !pos->square_attacked(D8, WHITE) && !pos->square_attacked(C8, WHITE)) {
				move_list->add_move(C8, E8, 0, CASTLING);
			}
		}

	}

	template <MoveType type, SIDE me>
	void generate_king_moves(GameState_t* pos, MoveList* move_list) {

		assert(pos->pieceBBS[KING][me] != 0);

		Bitboard attackBrd = 0;
		int sq = 0;
		int kingSq = (me == WHITE) ? pos->king_squares[WHITE] : pos->king_squares[BLACK];

		Bitboard opponent_pieces = pos->all_pieces[(me == WHITE) ? BLACK : WHITE];
		Bitboard friendly_pieces = pos->all_pieces[me];

		if constexpr (type == QUIET) {
			gen_castle_moves<me>(pos, move_list);
			attackBrd = BBS::king_attacks[pos->king_squares[me]] & ~(opponent_pieces | friendly_pieces);
			
			while (attackBrd) {
				sq = PopBit(&attackBrd);

				move_list->add_move(sq, kingSq, 0, NOT_SPECIAL);
			}
			return;
		}

		else if constexpr (type == CAPTURES) {
			attackBrd = BBS::king_attacks[pos->king_squares[me]] & opponent_pieces;

			while (attackBrd) {
				sq = PopBit(&attackBrd);

				move_list->add_move(sq, kingSq, 0, NOT_SPECIAL);
			}
			return;
		}

		else if constexpr (type == CASTLE) {
			gen_castle_moves<me>(pos, move_list);
			
			return;
		}

		else { // MoveType is ALL
			gen_castle_moves<me>(pos, move_list);
			attackBrd = BBS::king_attacks[pos->king_squares[me]] & ~friendly_pieces;

			while (attackBrd) {
				sq = PopBit(&attackBrd);

				move_list->add_move(sq, kingSq, 0, NOT_SPECIAL);
			}
			return;
		}
	}

	
	template <MoveType type, SIDE me>
	void generate_all(GameState_t* pos, MoveList* move_list) {		
		generate_pawn_moves<type, me>(pos, move_list);
		generate_knight_moves<type, me>(pos, move_list);

		generate_piece_moves<BISHOP, type, me>(pos, move_list);
		generate_piece_moves<ROOK, type, me>(pos, move_list);
		generate_piece_moves<QUEEN, type, me>(pos, move_list);

		generate_king_moves<type, me>(pos, move_list);

	}

	



	bool moveExists(GameState_t* pos, unsigned int move) {
		MoveList ml;

		if (pos->side_to_move == WHITE) { generate_all<ALL, WHITE>(pos, &ml); }
		else { generate_all<ALL, BLACK>(pos, &ml); }

		for (int i = 0; i < ml.size(); i++) {
			if (ml[i]->move == move) {
				return true;
			}
		}
		return false;
	}
};

template<MoveType T>
void moveGen::generate(GameState_t* pos, MoveList* move_list) {
	SIDE me = pos->side_to_move;
	
	//return (me == WHITE) ? generate_all<T, WHITE>(pos) : generate_all<T, BLACK>(pos);
	if (me == WHITE) {
		generate_all<T, WHITE>(pos);
	}
	else {
		generate_all<T, BLACK>(pos);
	}
}


template<>
void moveGen::generate<ALL>(GameState_t* pos, MoveList* move_list) {
	SIDE me = pos->side_to_move;

	//return (me == WHITE) ? generate_all<ALL, WHITE>(pos) : generate_all<ALL, BLACK>(pos);
	if (me == WHITE) {
		generate_all<ALL, WHITE>(pos, move_list);
	}
	else {
		generate_all<ALL, BLACK>(pos, move_list);
	}
}


template<>
void moveGen::generate<CAPTURES>(GameState_t* pos, MoveList* move_list) {
	SIDE me = pos->side_to_move;

	//return (me == WHITE) ? generate_all<CAPTURES, WHITE>(pos) : generate_all<CAPTURES, BLACK>(pos);
	if (me == WHITE) {
		generate_all<CAPTURES, WHITE>(pos, move_list);
	}
	else {
		generate_all<CAPTURES, BLACK>(pos, move_list);
	}
}