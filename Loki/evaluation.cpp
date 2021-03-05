#include "evaluation.h"



namespace Eval {


	int Evaluation::interpolate(GameState_t* pos) {
		int p = phase(pos);
		if (p > 24) { p = 24; }

		int w_mg = p; int w_eg = 24 - p;

		return (((w_mg * mg) + (w_eg * eg)) / 24);
	}


	namespace BM = BBS::EvalBitMasks;

	namespace {

		/*
		
		Material evaluations

		*/

		template<SIDE side, GamePhase phase>
		int material(GameState_t* pos) {
			int v = 0;

			v += ((phase == MG) ? pawnValMg : pawnValEg) * countBits(pos->pieceBBS[PAWN][side]);
			v += ((phase == MG) ? knightValMg : knightValEg) * countBits(pos->pieceBBS[KNIGHT][side]);
			v += ((phase == MG) ? bishopValMg : bishopValEg) * countBits(pos->pieceBBS[BISHOP][side]);
			v += ((phase == MG) ? rookValMg : rookValEg) * countBits(pos->pieceBBS[ROOK][side]);
			v += ((phase == MG) ? queenValMg : queenValEg) * countBits(pos->pieceBBS[QUEEN][side]);
			
			return v;
		}


		/*
		
		Piece Square tables

		*/

		const int* middlegameTables[6] = { &PSQT::PawnTableMg[0], &PSQT::KnightTableMg[0], &PSQT::BishopTableMg[0],
			&PSQT::RookTableMg[0], &PSQT::QueenTableMg[0], &PSQT::KingTableMg[0] };
		const int* endgameTables[6] = { &PSQT::PawnTableEg[0], &PSQT::KnightTableEg[0], &PSQT::BishopTableEg[0],
			&PSQT::RookTableEg[0], &PSQT::QueenTableEg[0], &PSQT::KingTableEg[0] };

		template<SIDE side, GamePhase p>
		int addPsqtVal(int pce, int sq) {
			int v = 0;

			assert(p == MG || p == EG);
			assert(side == WHITE || side == BLACK);
			assert(sq >= 0 && sq <= 63);
			assert(pce >= PAWN && pce < NO_TYPE);
			assert(PSQT::Mirror64[PSQT::Mirror64[sq]] == sq);

			v += (p == MG) ? middlegameTables[pce][(side == WHITE) ? sq : PSQT::Mirror64[sq]] : endgameTables[pce][(side == WHITE) ? sq : PSQT::Mirror64[sq]];

			return v;
		}


		template<SIDE side, GamePhase phase>
		int psqt(GameState_t* pos) {
			int v = 0;

			Bitboard pceBrd = 0;
			int sq = 0;

			for (int pce = PAWN; pce <= KING; pce++) {
				pceBrd = pos->pieceBBS[pce][side];

				while (pceBrd) {
					sq = PopBit(&pceBrd);

					v += addPsqtVal<side, phase>(pce, sq);
				}
			}

			return v;
		}


		/*
		
		King safety
		
		*/


		template<SIDE side, GamePhase p>
		int non_pawn_material(GameState_t* pos) {
			assert(side == WHITE || side == BLACK);
			assert(p == MG || p == EG);

			if (p == MG) {
				return knightValMg * countBits(pos->pieceBBS[KNIGHT][side]) + bishopValMg * countBits(pos->pieceBBS[BISHOP][side]) + rookValMg * countBits(pos->pieceBBS[ROOK][side])
					+ queenValMg * countBits(pos->pieceBBS[QUEEN][side]);
			}
			else {
				return knightValEg * countBits(pos->pieceBBS[KNIGHT][side]) + bishopValEg * countBits(pos->pieceBBS[BISHOP][side]) + rookValEg * countBits(pos->pieceBBS[ROOK][side])
					+ queenValEg * countBits(pos->pieceBBS[QUEEN][side]);
			}
		}


		template<SIDE side>
		void king_safety(GameState_t* pos, Evaluation& eval){
			constexpr SIDE Them = (side == WHITE) ? BLACK : WHITE;
			constexpr int relative_ranks[8] = { (side == WHITE) ? RANK_1 : RANK_8, (side == WHITE) ? RANK_2 : RANK_7,
				(side == WHITE) ? RANK_3 : RANK_6, (side == WHITE) ? RANK_4 : RANK_5, (side == WHITE) ? RANK_5 : RANK_4,
				(side == WHITE) ? RANK_6 : RANK_3, (side == WHITE) ? RANK_7 : RANK_2, (side == WHITE) ? RANK_8 : RANK_1 };
			
			int mg = 0;
			int eg = 0;

			int attack_units = 0; // Used to index safety table.
			int king_square = pos->king_squares[side];
			int king_file = king_square % 8;
			Bitboard kingRing = king_ring(king_square);

			// Penalize the side depending on the distance to the pawns on the king's flank. This doesn't apply for the center
			Bitboard king_pawns = pos->pieceBBS[PAWN][side] & king_flanks[king_square % 8];
			Bitboard enemy_king_pawns = pos->pieceBBS[PAWN][Them] & king_flanks[king_file];
			int sq;

			// Penalize missing pawns on the files of the flank
			for (int f = std::max(0, king_file - 1); f <= std::min(7, king_file + 1); f++) {
				if ((pos->pieceBBS[PAWN][side] & BBS::FileMasks8[f]) == 0) {
					mg -= 40;
					eg -= 15;
				}

				// Penalize an open/semi-open file in the endgame
				if (((pos->pieceBBS[PAWN][WHITE] | pos->pieceBBS[PAWN][BLACK]) & BBS::FileMasks8[f]) == 0) { // No pawns --> open
					mg -= king_open_file_penalty;
				}
				else if ((pos->pieceBBS[PAWN][Them] & BBS::FileMasks8[f]) == 0) { // Only pawns of this side --> semi-open
					mg -= king_semi_open_file_penalty;
				}
			}
			
			if (king_file != FILE_E && king_file != FILE_D) {
				while (king_pawns) {
					//sq = (side == WHITE) ? PopBit(&king_pawns) : PSQT::Mirror64[PopBit(&king_pawns)];
					sq = PopBit(&king_pawns);

					mg -= (side == WHITE) ? PSQT::castledPawnAdvancementMg[sq] : PSQT::castledPawnAdvancementMg[PSQT::Mirror64[sq]];
			
					// Penalize distance to pawns on the king flank in the endgame
					eg -= 2 * PSQT::ManhattanDistance[king_square][sq];
				}

				while (enemy_king_pawns) {
					sq = PopBit(&enemy_king_pawns);

					mg -= (side == WHITE) ? PSQT::pawnStormMg[sq] : PSQT::pawnStormMg[PSQT::Mirror64[sq]];
				}

			}

			// We'll scale the king's pawn structure based on the opponents material. This will encourage the engine to trade pieces if the king's pawn
			// structure is damaged, and not trade when the opponent's is worse.
			double scalar_mg = double((side == WHITE) ? non_pawn_material<BLACK, MG>(pos) : non_pawn_material<WHITE, MG>(pos)) / double(max_material[MG]);
			double scalar_eg = double((side == WHITE) ? non_pawn_material<BLACK, EG>(pos) : non_pawn_material<WHITE, EG>(pos)) / double(max_material[EG]);

			mg = std::round(mg * scalar_mg);
			eg = std::round(eg * scalar_eg);

			// Now we'll gather information on attack units.
			// We give each attack by a minor two attack units, rook attacks get 3 and queens get 5.
			int knight_bishop_attackCnt = countBits((kingRing & eval.attacks[KNIGHT][Them]) | (kingRing & eval.attacks[BISHOP][Them]));
			int rook_attackCnt = countBits(kingRing & eval.attacks[ROOK][Them]);
			int queen_attackCnt = countBits(kingRing & eval.attacks[QUEEN][Them]);

			attack_units += 2 * knight_bishop_attackCnt;
			attack_units += 3 * rook_attackCnt;
			attack_units += 5 * queen_attackCnt;

			attack_units += (3 - std::min(3, countBits(pos->pieceBBS[PAWN][side] & king_flanks[king_square % 8])));

			if (attack_units > 99) { // Prevent overflow when accessing safety table.
				attack_units = 99;
			}

			// Only use safety table when opponent has a queen and more than one attacker
			if ((knight_bishop_attackCnt + rook_attackCnt + queen_attackCnt) > 1 && pos->pieceBBS[QUEEN][Them] != 0) {
				mg -= PSQT::safety_table[attack_units].mg();
				eg -= PSQT::safety_table[attack_units].eg();
			}

			eval.mg += (side == WHITE) ? mg : -mg;
			eval.eg += (side == WHITE) ? eg : -eg;
		}


		/*
		
		Pawn evaluation.
		
		*/
		template<SIDE side>
		void pawns(GameState_t* pos, Evaluation& eval) {
			int mg = 0;
			int eg = 0;

			// Declare some side-relative constants
			constexpr Bitboard* passedBitmask = (side == WHITE) ? BM::passed_pawn_masks[WHITE] : BM::passed_pawn_masks[BLACK];

			constexpr int relative_ranks[8] = { (side == WHITE) ? RANK_1 : RANK_8, (side == WHITE) ? RANK_2 : RANK_7,
				(side == WHITE) ? RANK_3 : RANK_6, (side == WHITE) ? RANK_4 : RANK_5, (side == WHITE) ? RANK_5 : RANK_4,
				(side == WHITE) ? RANK_6 : RANK_3, (side == WHITE) ? RANK_7 : RANK_2, (side == WHITE) ? RANK_8 : RANK_1 };

			constexpr SIDE Them = (side == WHITE) ? BLACK : WHITE;

			constexpr DIRECTION downLeft = (side == WHITE) ? SOUTHWEST : NORTHWEST;
			constexpr DIRECTION downRight = (side == WHITE) ? SOUTHEAST : NORTHEAST;
			constexpr DIRECTION upLeft = (side == WHITE) ? NORTHWEST : SOUTHWEST;
			constexpr DIRECTION upRight = (side == WHITE) ? NORTHEAST : SOUTHEAST;

			Bitboard pawnBoard = (side == WHITE) ? pos->pieceBBS[PAWN][WHITE] : pos->pieceBBS[PAWN][BLACK];
			int sq = NO_SQ;
			int relative_sq = NO_SQ; // For black it is PSQT::Mirror64[sq] but for white it is just == sq


			// Before evaluating all pawns, we will score the amount of doubled pawns by file.
			int doubled_count = 0;
			for (int f = FILE_A; f <= FILE_H; f++) {
				doubled_count += (countBits(BBS::FileMasks8[f] & pos->pieceBBS[PAWN][side]) > 1) ? 1 : 0;
			}
			
			mg -= doubled_count * doubled_penalty[MG];
			eg -= doubled_count * doubled_penalty[EG];

			// Now evaluate each individual pawn
			while (pawnBoard) {
				sq = PopBit(&pawnBoard);
				relative_sq = (side == WHITE) ? sq : PSQT::Mirror64[sq];

				//int r = relative_ranks[sq / 8];
				int f = sq % 8;

				// Passed pawn bonus
				if ((passedBitmask[sq] & pos->pieceBBS[PAWN][Them]) == 0) { // No enemy pawns in front
					mg += PSQT::passedPawnTable[relative_sq];
					eg += PSQT::passedPawnTable[relative_sq];

					// Save the passed pawn's position such that we can give a bonus if it is defended by pieces later.
					eval.passed_pawns[side] |= (uint64_t(1) << sq);
				}

				// Isolated penalty and/or doubled
				bool doubled = (countBits(BBS::FileMasks8[f] & pos->pieceBBS[PAWN][side]) > 1) ? true : false;
				bool isolated = ((BM::isolated_bitmasks[f] & pos->pieceBBS[PAWN][side]) == 0) ? true : false;

				if (doubled && isolated) {
					mg -= doubled_isolated_penalty[MG];
					eg -= doubled_isolated_penalty[EG];
				}
				else if (isolated) {
					mg -= isolated_penalty[MG];
					eg -= isolated_penalty[EG];
				}
			}

			// Populate the attacks bitboard with pawn attacks. This will be used in the evaluation of pieces.
			eval.attacks[PAWN][side] = (shift<upRight>(pos->pieceBBS[PAWN][side]) | shift<upLeft>(pos->pieceBBS[PAWN][side]));


			// Make the scores stored side-relative
			eval.mg += (side == WHITE) ? mg : -mg;
			eval.eg += (side == WHITE) ? eg : -eg;
		}





		template<SIDE side>
		Bitboard outposts(Bitboard pawnAttacks, Bitboard opponent_pawns) {
			
			constexpr Bitboard their_side = (side == WHITE) ? (BBS::RankMasks8[RANK_5] | BBS::RankMasks8[RANK_6] | BBS::RankMasks8[RANK_7] | BBS::RankMasks8[RANK_8]) :
				(BBS::RankMasks8[RANK_4] | BBS::RankMasks8[RANK_3] | BBS::RankMasks8[RANK_2] | BBS::RankMasks8[RANK_1]);

			Bitboard relevant_pawnAttacks = pawnAttacks & their_side;
			Bitboard outpost_bb = 0;
			int sq = NO_SQ;

			while (relevant_pawnAttacks) {
				sq = PopBit(&relevant_pawnAttacks);

				// If the square can't be attacked by opponent pawns, it is an outpost.
				if ((BM::outpost_masks[side][sq] & opponent_pawns) == 0) {
					outpost_bb |= (uint64_t(1) << sq);
				}
			}
			return outpost_bb;
		}


		template<SIDE side, int pce>
		void pieces(GameState_t* pos, Evaluation& eval) {
			
			constexpr SIDE Them = (side == WHITE) ? BLACK : WHITE;
			constexpr int first_rank = (side == WHITE) ? RANK_1 : RANK_8;
			
			Bitboard occupied = pos->all_pieces[WHITE] | pos->all_pieces[BLACK];
			Bitboard enemy_kingRing = king_ring((side == WHITE) ? pos->king_squares[BLACK] : pos->king_squares[WHITE]);

			int mg = 0;
			int eg = 0;

			Bitboard pceBoard = pos->pieceBBS[pce][side];
			int sq = NO_SQ;

			// the outposts bitmask is only populated if we're evaluating bishops or knights.
			Bitboard outpost_mask = 0;
			if (pce == BISHOP || pce == KNIGHT) {
				outpost_mask = outposts<side>(eval.attacks[PAWN][side], pos->pieceBBS[PAWN][Them]);
			
				// Give bonuses for occupying the calculated outposts
				int num_outposts = countBits(pceBoard & outpost_mask);
				mg += outpost[MG] * num_outposts;
				eg += outpost[EG] * num_outposts;
			}


			// If the pieces being evaluated are rooks, we'll give bonuses for being doubled. They dont get bonuses for being doubled on the same rank.
			if (pce == ROOK) {
				for (int f = FILE_A; f <= FILE_H; f++) {

					if (countBits(pos->pieceBBS[ROOK][side] & BBS::FileMasks8[f]) > 1) {
						mg += doubled_rooks[MG];
						eg += doubled_rooks[EG];
					}
				}
			}


			while (pceBoard) {

				sq = PopBit(&pceBoard);
				int r = sq / 8;
				int f = sq % 8;

				if (pce == KNIGHT || pce == BISHOP) {
					// Save the squares that the piece attacks.
					Bitboard attacks = ((pce == KNIGHT) ? BBS::knight_attacks[sq] : Magics::attacks_bb<BISHOP>(sq, occupied));
					eval.attacks[pce][side] |= attacks;
				
					// Bonus for being able to reach an outpost on the next move.
					int num_reachable_outposts = countBits(attacks & outpost_mask);
					mg += reachable_outpost[MG] * num_reachable_outposts;
					eg += reachable_outpost[EG] * num_reachable_outposts;
				
					// Bishop specific eval.
					if (pce == BISHOP) {
				
						// Bonus for being on the same diagonal or anti-diagonal as the enemy queen.
						if (((BBS::diagonalMasks[7 + r - f] | BBS::antidiagonalMasks[r + f]) & pos->pieceBBS[QUEEN][Them]) != 0) {
							mg += bishop_on_queen[MG];
							eg += bishop_on_queen[EG];
						}
				
						// Bad bishop. Penalty for being blocked by our own pawns.
						int blocking_pawns_count = countBits(attacks & pos->pieceBBS[PAWN][side]);
						mg -= blocked_bishop_coefficient_penalty[MG] * blocking_pawns_count;
						eg -= blocked_bishop_coefficient_penalty[EG] * blocking_pawns_count;

						// Bonus for attacking the king ring
						if ((attacks & enemy_kingRing) != 0) {
							mg += bishop_on_kingring[MG];
							eg += bishop_on_kingring[EG];
						}
					}
				
				
					// Knight specific eval
					else {
				
						// On the wiki (https://www.chessprogramming.org/Evaluation_of_Pieces) it is adviced to penalize a knight placed on c3 or c6 (depending on side to move)
						//	if there are pawns on - for white - c2, d4, and not e4 or - for black - c7, d5 and not e5.
						//	This is probably because these openings usually use the c-pawn as a pawn-break.
				
				
						// Small bonus for being defended by a pawn.
						if ((eval.attacks[PAWN][side] & (uint64_t(1) << sq)) != 0) {
							mg += defended_knight[MG];
							eg += defended_knight[EG];
						}

						// Bonus for attacking the enemy king ring
						if ((attacks & enemy_kingRing) != 0) {
							mg += knight_on_kingring[MG];
							eg += knight_on_kingring[EG];
						}
					}
					continue;
				}

				if (pce == ROOK) {
					// Get the squares that the rook attacks.
					Bitboard attacks = Magics::attacks_bb<ROOK>(sq, occupied);
					eval.attacks[ROOK][side] |= attacks;
				
					// Give bonus for being aligned with the queen.
					if (((BBS::RankMasks8[r] | BBS::FileMasks8[f]) & pos->pieceBBS[QUEEN][Them]) != 0) {
						mg += rook_on_queen[MG];
						eg += rook_on_queen[EG];
					}
					
					// Give bonus for attacking the king ring
					if ((attacks & enemy_kingRing) != 0) {
						mg += rook_on_kingring[MG];
						eg += rook_on_kingring[EG];
					}
				
					// Give bonus for being on an open file.
					if (((pos->pieceBBS[PAWN][BLACK] | pos->pieceBBS[PAWN][WHITE]) & BBS::FileMasks8[f]) == 0) {
						mg += rook_open_file[MG];
						eg += rook_open_file[EG];
					}
					
					// If we're not on an open file, see if we're on a semi-open one and score accordingly.
					else if (((pos->pieceBBS[PAWN][Them]) & BBS::FileMasks8[f]) == 0) {
						mg += rook_semi_open_file[MG];
						eg += rook_semi_open_file[EG];
					}
				
					// Give a large bonus for the Tarrasch rule: In the endgame, rooks are best placed behind passed pawns.
					// If we're not directly defending it, but are instead on the same file, give half the bonus.
					if ((attacks & eval.passed_pawns[side]) != 0) {
						eg += rook_behind_passer;
					}
				
					else if ((BBS::FileMasks8[f] & eval.passed_pawns[side]) != 0) {
						eg += (rook_behind_passer / 2);
					}
				
					continue;
				}


				if (pce == QUEEN) {
					// Get the squares that the queen attacks
					Bitboard attacks = Magics::attacks_bb<QUEEN>(sq, occupied);
					eval.attacks[QUEEN][side] |= attacks;
				
					// Give a penalty for early queen development. This is done by multiplying a factor with the amount of minor pieces on the first rank.
					if (r != first_rank) {
						mg -= queen_development_penalty * countBits((pos->pieceBBS[BISHOP][side] | pos->pieceBBS[KNIGHT][side]) & BBS::RankMasks8[first_rank]);
					}

					// Give small bonus for attacking the enemy king ring
					if ((attacks & enemy_kingRing) != 0) {
						mg += queen_on_kingring[MG];
						eg += queen_on_kingring[EG];
					}
				
					continue;
				}

			}


			// Make the scores side-relative
			eval.mg += (side == WHITE) ? mg : -mg;
			eval.eg += (side == WHITE) ? eg : -eg;
		}



		/*
		
		Mobility
		
		*/

		template<SIDE side, int pce>
		void mobility(GameState_t* pos, Evaluation& Eval) {
			int mg = 0;
			int eg = 0;

			Bitboard attacks = 0; // All attacks from all pieces of type pce used to populate the bitmasks in Eval
			Bitboard piece_attacks = 0; // Individual piece attacks.

			Bitboard friends = pos->all_pieces[side];
			Bitboard pceBoard = pos->pieceBBS[pce][side];
			int sq = 0;


			if (pce < KNIGHT || pce > QUEEN || pceBoard == 0) {
				return;
			}

			while (pceBoard != 0) {
				sq = PopBit(&pceBoard);

				if constexpr (pce == KNIGHT) {
					piece_attacks = BBS::knight_attacks[sq];
					attacks |= piece_attacks;
					piece_attacks &= ~friends;
					
					int attack_cnt = countBits(piece_attacks);
					assert(attack_cnt < 9);
					mg += PSQT::mobilityBonus[pce - 1][attack_cnt].mg();
					eg += PSQT::mobilityBonus[pce - 1][attack_cnt].eg();
				}

				else if constexpr (pce == BISHOP) {
					piece_attacks = Magics::attacks_bb<BISHOP>(sq, (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]));
					attacks |= piece_attacks;
					piece_attacks &= ~friends;

					int attack_cnt = countBits(piece_attacks);
					assert(attack_cnt < 15);
					mg += PSQT::mobilityBonus[pce - 1][attack_cnt].mg();
					eg += PSQT::mobilityBonus[pce - 1][attack_cnt].eg();
				}

				else if constexpr (pce == ROOK) {
					piece_attacks = Magics::attacks_bb<ROOK>(sq, (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]));
					attacks |= piece_attacks;
					piece_attacks &= ~friends;

					int attack_cnt = countBits(piece_attacks);
					assert(attack_cnt < 15);
					mg += PSQT::mobilityBonus[pce - 1][attack_cnt].mg();
					eg += PSQT::mobilityBonus[pce - 1][attack_cnt].eg();
				}

				else if constexpr (pce == QUEEN) {
					piece_attacks = Magics::attacks_bb<QUEEN>(sq, (pos->all_pieces[WHITE] | pos->all_pieces[BLACK]));
					attacks |= piece_attacks;
					piece_attacks &= ~friends;

					int attack_cnt = countBits(piece_attacks);
					assert(attack_cnt < 29);
					mg += PSQT::mobilityBonus[pce - 1][attack_cnt].mg();
					eg += PSQT::mobilityBonus[pce - 1][attack_cnt].eg();
				}

				else { // Just in case we went into the loop without a proper piece-type.
					assert(pce >= KNIGHT && pce <= QUEEN); // Raise an error.
					return;
				}
			}

			// Populate attacks bitmask for the piece and side in Eval.
			Eval.attacks[pce][side] = attacks;

			// Make the scores side-relative.
			Eval.mg += (side == WHITE) ? mg : -mg;
			Eval.eg += (side == WHITE) ? eg : -eg;
		}
	}



	int evaluate(GameState_t* pos) {
		int v = 0;

		// Material draw check (~22 elo)
		if (material_draw(pos)) {
			return 0;
		}

		// Create an evaluation object holding the middlegame and endgamescore separately.
		Evaluation eval(mg_evaluate(pos), eg_evaluate(pos));

		// Pawn structure evaluation
		pawns<WHITE>(pos, eval); pawns<BLACK>(pos, eval);
		
		// Evaluate mobility (~56 elo)
		mobility<WHITE, KNIGHT>(pos, eval); mobility<BLACK, KNIGHT>(pos, eval);
		mobility<WHITE, BISHOP>(pos, eval); mobility<BLACK, BISHOP>(pos, eval);
		mobility<WHITE, ROOK>(pos, eval); mobility<BLACK, ROOK>(pos, eval);
		mobility<WHITE, QUEEN>(pos, eval); mobility<BLACK, QUEEN>(pos, eval);

		// Simple king safety evaluation (~47 elo)
		king_safety<WHITE>(pos, eval); king_safety<BLACK>(pos, eval);

		// Piece evaluations --> loses elo (~23) at the moment
		//pieces<WHITE, KNIGHT>(pos, eval);	pieces<BLACK, KNIGHT>(pos, eval);
		//pieces<WHITE, BISHOP>(pos, eval);	pieces<BLACK, BISHOP>(pos, eval);
		//pieces<WHITE, ROOK>(pos, eval);		pieces<BLACK, ROOK>(pos, eval);
		//pieces<WHITE, QUEEN>(pos, eval);	pieces<BLACK, QUEEN>(pos, eval);

		v = eval.interpolate(pos);

		v += (pos->side_to_move == WHITE) ? tempo : -tempo;

		v *= (pos->side_to_move == WHITE) ? 1 : -1;


		return v;
	}


	int mg_evaluate(GameState_t* pos) {
		int v = 0;

		v += (material<WHITE, MG>(pos) - material<BLACK, MG>(pos));
		v += (psqt<WHITE, MG>(pos) - psqt<BLACK, MG>(pos));

		return v;
	}

	int eg_evaluate(GameState_t* pos) {
		int v = 0;

		v += (material<WHITE, EG>(pos) - material<BLACK, EG>(pos));
		v += (psqt<WHITE, EG>(pos) - psqt<BLACK, EG>(pos));

		return v;
	}


	int phase(GameState_t* pos) {
		int p = 0;

		p += 1 * (countBits(pos->pieceBBS[KNIGHT][WHITE] | pos->pieceBBS[KNIGHT][BLACK]));
		p += 1 * (countBits(pos->pieceBBS[BISHOP][WHITE] | pos->pieceBBS[BISHOP][BLACK]));
		p += 2 * (countBits(pos->pieceBBS[ROOK][WHITE] | pos->pieceBBS[ROOK][BLACK]));
		p += 4 * (countBits(pos->pieceBBS[QUEEN][WHITE] | pos->pieceBBS[QUEEN][BLACK]));

		return p;
	}




	bool material_draw(GameState_t* pos) {

		// If there are only kings left, it is an immediate draw.
		if ((pos->all_pieces[WHITE] ^ pos->pieceBBS[KING][WHITE]) == 0 && (pos->all_pieces[BLACK] ^ pos->pieceBBS[KING][BLACK]) == 0) {
			return true;
		}

		// If there are still pawns on the board, it is not a material draw due to the possibility of promotions
		if (pos->pieceBBS[PAWN][WHITE] == 0 && pos->pieceBBS[PAWN][BLACK] == 0) {
			// We assume that any number of major pieces can force a checkmate.
			if (pos->pieceBBS[QUEEN][WHITE] == 0 && pos->pieceBBS[ROOK][WHITE] == 0 && pos->pieceBBS[QUEEN][BLACK] == 0 && pos->pieceBBS[ROOK][BLACK] == 0) {
				// If there are no bishops
				if (pos->pieceBBS[BISHOP][WHITE] == 0 && pos->pieceBBS[BISHOP][BLACK] == 0) {
					if (countBits(pos->pieceBBS[KNIGHT][WHITE]) <= 2 && countBits(pos->pieceBBS[KNIGHT][BLACK]) <= 2) { return true; }
				}

				// If there are no knights
				else if (pos->pieceBBS[KNIGHT][WHITE] == 0 && pos->pieceBBS[KNIGHT][BLACK] == 0) {
					if (abs(countBits(pos->pieceBBS[BISHOP][WHITE]) - countBits(pos->pieceBBS[BISHOP][BLACK])) < 2) { return true; }
				}

				else if ((countBits(pos->pieceBBS[KNIGHT][WHITE]) < 3 && pos->pieceBBS[BISHOP][WHITE] == 0) || (countBits(pos->pieceBBS[BISHOP][WHITE]) == 1 && pos->pieceBBS[KNIGHT][WHITE] == 0)) {
					if (((countBits(pos->pieceBBS[KNIGHT][BLACK]) < 3 && pos->pieceBBS[BISHOP][BLACK] == 0) || (countBits(pos->pieceBBS[BISHOP][BLACK]) == 1 && pos->pieceBBS[KNIGHT][BLACK] == 0))) {
						return true;
					}
				}
			}

			else if (pos->pieceBBS[QUEEN][WHITE] == 0 && pos->pieceBBS[QUEEN][BLACK] == 0) {
				if (countBits(pos->pieceBBS[ROOK][WHITE]) == 1 && countBits(pos->pieceBBS[ROOK][BLACK]) == 1) {
					if (countBits(pos->pieceBBS[KNIGHT][WHITE] | pos->pieceBBS[BISHOP][WHITE]) < 2 && countBits(pos->pieceBBS[KNIGHT][BLACK] | pos->pieceBBS[BISHOP][BLACK]) < 2) { return true; }
				}

				else if (countBits(pos->pieceBBS[ROOK][WHITE]) == 1 && pos->pieceBBS[ROOK][BLACK] == 0) {
					if ((pos->pieceBBS[KNIGHT][WHITE] | pos->pieceBBS[BISHOP][WHITE]) == 0 &&
						(countBits(pos->pieceBBS[KNIGHT][BLACK] | pos->pieceBBS[BISHOP][BLACK]) == 1 || countBits(pos->pieceBBS[KNIGHT][BLACK] | pos->pieceBBS[BISHOP][BLACK]) == 2)) {
						return true;
					}
				}

				else if (pos->pieceBBS[ROOK][WHITE] == 0 && countBits(pos->pieceBBS[ROOK][BLACK]) == 1) {
					if ((pos->pieceBBS[KNIGHT][BLACK] | pos->pieceBBS[BISHOP][BLACK]) == 0 &&
						(countBits(pos->pieceBBS[KNIGHT][WHITE] | pos->pieceBBS[BISHOP][WHITE]) == 1 || countBits(pos->pieceBBS[KNIGHT][WHITE] | pos->pieceBBS[BISHOP][WHITE]) == 2)) {
						return true;
					}
				}

			}
		}

		return false;
	}



	Bitboard king_flanks[8] = { 0 };

	void initKingFlanks() {
		Bitboard flank;

		for (int f = FILE_A; f <= FILE_H; f++) {
			if (f < FILE_D) {
				king_flanks[f] = (BBS::FileMasks8[FILE_A] | BBS::FileMasks8[FILE_B] | BBS::FileMasks8[FILE_C]);
			}
			else if (f == FILE_D) {
				king_flanks[f] = (BBS::FileMasks8[FILE_C] | BBS::FileMasks8[FILE_D] | BBS::FileMasks8[FILE_E]);
			}
			else if (f == FILE_E) {
				king_flanks[f] = (BBS::FileMasks8[FILE_D] | BBS::FileMasks8[FILE_E] | BBS::FileMasks8[FILE_F]);
			}
			else if (f > FILE_E) {
				king_flanks[f] = (BBS::FileMasks8[FILE_F] | BBS::FileMasks8[FILE_G] | BBS::FileMasks8[FILE_H]);
			}
		}
	}


	void INIT() {

		initKingFlanks();

	}
}




void Eval::Debug::eval_balance() {

	GameState_t* pos = new GameState_t;

	int total = 0;
	int passed = 0;
	int failed = 0;

	int w_ev = 0;
	int b_ev = 0;

	for (int p = 0; p < test_positions.size(); p++) {
		total++;
		pos->parseFen(test_positions[p]);

		w_ev = evaluate(pos);

		pos->mirror_board();

		b_ev = evaluate(pos);


		if (w_ev == b_ev) {
			std::cout << "Position " << (p + 1) << "	--->	" << "PASSED" << std::endl;
			passed++;
		}
		else {
			std::cout << "Position " << (p + 1) << "	--->	" << "FAILED: " << w_ev << " != " << b_ev << "		(FEN: " << test_positions[p] << ")" << std::endl;
			failed++;
		}
	}

	std::cout << total << " positions analyzed." << std::endl;
	std::cout << passed << " positions passed." << std::endl;
	std::cout << failed << " positions failed. (" << (double(failed) / double(total)) * 100.0 << "%)" << std::endl;

	delete pos;
}