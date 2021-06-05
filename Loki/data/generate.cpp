#include "generate.h"


namespace DataGeneration {
	

	namespace SelfPlay {
		// Setup of a saved position
		void SavedBoard::setup(const GameState_t* pos, int evaluation) {
			// Step 1. Copy the gamestate.
			pieces[PAWN][WHITE] = pos->pieceBBS[PAWN][WHITE];
			pieces[KNIGHT][WHITE] = pos->pieceBBS[KNIGHT][WHITE];
			pieces[BISHOP][WHITE] = pos->pieceBBS[BISHOP][WHITE];
			pieces[ROOK][WHITE] = pos->pieceBBS[ROOK][WHITE];
			pieces[QUEEN][WHITE] = pos->pieceBBS[QUEEN][WHITE];
			pieces[KING][WHITE] = pos->pieceBBS[KING][WHITE];

			pieces[PAWN][BLACK] = pos->pieceBBS[PAWN][BLACK];
			pieces[KNIGHT][BLACK] = pos->pieceBBS[KNIGHT][BLACK];
			pieces[BISHOP][BLACK] = pos->pieceBBS[BISHOP][BLACK];
			pieces[ROOK][BLACK] = pos->pieceBBS[ROOK][BLACK];
			pieces[QUEEN][BLACK] = pos->pieceBBS[QUEEN][BLACK];
			pieces[KING][BLACK] = pos->pieceBBS[KING][BLACK];

			stm = pos->side_to_move;
			en_passant = pos->enPasSq;
			castling_rights = pos->castleRights;
			move50 = pos->fiftyMove;

			// Step 2. Save the score
			score = evaluation;
		}


		// Constructors for SavedBoard
		SavedBoard::SavedBoard() {
			pieces[PAWN][WHITE] = 0;
			pieces[KNIGHT][WHITE] = 0;
			pieces[BISHOP][WHITE] = 0;
			pieces[ROOK][WHITE] = 0;
			pieces[QUEEN][WHITE] = 0;
			pieces[KING][WHITE] = 0;

			pieces[PAWN][BLACK] = 0;
			pieces[KNIGHT][BLACK] = 0;
			pieces[BISHOP][BLACK] = 0;
			pieces[ROOK][BLACK] = 0;
			pieces[QUEEN][BLACK] = 0;
			pieces[KING][BLACK] = 0;

			stm = WHITE; en_passant = NO_SQ; castling_rights = 0; move50 = 0; score = 0; game_result = 0.5;
		}

		SavedBoard::SavedBoard(const SavedBoard& sb) {
			// Step 1. Copy the gamestate
			pieces[PAWN][WHITE] = sb.pieces[PAWN][WHITE];
			pieces[KNIGHT][WHITE] = sb.pieces[KNIGHT][WHITE];
			pieces[BISHOP][WHITE] = sb.pieces[BISHOP][WHITE];
			pieces[ROOK][WHITE] = sb.pieces[ROOK][WHITE];
			pieces[QUEEN][WHITE] = sb.pieces[QUEEN][WHITE];
			pieces[KING][WHITE] = sb.pieces[KING][WHITE];

			pieces[PAWN][BLACK] = sb.pieces[PAWN][BLACK];
			pieces[KNIGHT][BLACK] = sb.pieces[KNIGHT][BLACK];
			pieces[BISHOP][BLACK] = sb.pieces[BISHOP][BLACK];
			pieces[ROOK][BLACK] = sb.pieces[ROOK][BLACK];
			pieces[QUEEN][BLACK] = sb.pieces[QUEEN][BLACK];
			pieces[KING][BLACK] = sb.pieces[KING][BLACK];

			stm = sb.stm; en_passant = sb.en_passant; castling_rights = sb.castling_rights; move50 = sb.move50; score = sb.score; game_result = sb.game_result;
		}


		/*

		The Arbiter class will be managed by each thread. It will be the one playing the games and saving the positions.

		*/
		Arbiter::Arbiter(int _depth, size_t _positions, bool _draws, int _eval_limit, bool _random, int _first_random, bool _vb) : depth(_depth), position_count(_positions),
			use_draws(_draws), eval_limit(_eval_limit), random_mover(_random), first_random_moves(_first_random), verbose(_vb) {

			// Step 1. Allocate memory for the search object
			searcher = new SearchThread_t;

			// Step 2. Make sure all paramters are correctly set. Abort if there are errors.
			try {
				if (depth < 0) { throw("Depth must be greater than or equal to zero."); }
				if (position_count <= 0) { throw("Positions to generate must be a positive number."); }
				if (eval_limit <= 0) { throw("Evaluation score limit must be a positive number."); }
			}
			catch (const char* msg) {
				std::cout << "[!] Encountered an error while setting up Arbiter object: " << msg << std::endl;
				exit(EXIT_FAILURE);
			}

		}

		Arbiter::~Arbiter() {
			if (searcher != nullptr) { delete searcher; }
		}


		/*

		When searching, we need to prepare our SearchThread_t object.

		*/
		void Arbiter::prepare_search() {
			// Step 1. Clear the info.
			searcher->info->clear();

			// Step 2. Set the parameters.
			searcher->info->depth = depth;
			searcher->info->nodes = 0;
			searcher->info->stopped = false;
			searcher->info->quit = false;
			searcher->info->timeset = false;
			searcher->info->seldepth = 0;
		}


		/*

		Get all legal moves for the position.

		*/
		MoveList Arbiter::legal_moves() {
			int original_ply = searcher->pos->ply;

			// Step 1. Get all pseudo-legal moves.
			MoveList pseudo;
			moveGen::generate<ALL>(searcher->pos, &pseudo);

			MoveList legal;

			// Step 2. Loop through all the moves and make them. If they are legal, append them to legal.
			for (int i = 0; i < pseudo.size(); i++) {
				if (!searcher->pos->make_move(pseudo[i])) {
					continue;
				}

				legal.append(pseudo[i]->move);

				searcher->pos->undo_move();
			}

			assert(searcher->pos->ply == original_ply); // We don't want to accidentally modify the position object.
			std::cout << "Legal size " << legal.size() << std::endl;
			return legal;
		}

		/*

		Helper method to check if the game has ended. The game can end for the following reasons:
			- White checkmates black --> White wins.
			- Black checkmates white --> Black wins.
			- Draw due to either 3-fold repetition, 50 move rule or stalemate.
		*/
		G_RESULT Arbiter::game_ended() {
			// Step 1. Check for draw due to either 3-fold repetition, 50 move rule or stalemate.
			if (searcher->pos->three_fold_draw() || searcher->pos->fiftyMove >= 100) { return DRAW; }

			// Step 2. Generate all legal moves. They are used to check for checkmate and stalemate.
			MoveList moves = legal_moves();

			// Step 3. Check for stalemate. This happens if no moves are possible irregardless of the side to move.
			if (moves.size() <= 0) { return DRAW; }

			// Step 4. Check for checkmates
			if (searcher->pos->side_to_move == WHITE) {
				if (moves.size() <= 0 && searcher->pos->in_check()) { return BLACK_WIN; }
			}
			else {
				if (moves.size() <= 0 && searcher->pos->in_check()) { return WHITE_WIN; }
			}

			// Return not done if the position didn't meet the above criteria
			return NOT_DONE;
		}



		/*

		This is the most important method in the Arbiter class. This is responsible for playing each game.
			Note: We will not stop a game in case we exceed the required number of positions since that wouldn't allow us to get a
					game result. This has to be handled in the run() method.

		*/
		void Arbiter::play_game() {

			// Step 1. Load the starting position and initialize a new vector of positions.
			searcher->pos->parseFen(START_FEN);
			game_positions.clear();

			int move_count = 0; // Half-moves played.
			G_RESULT game_result = NOT_DONE;

			// Step 2. Start the game.
			while (true) {

				// Step 2A. Get a list of the legal moves.
				MoveList valid_moves = legal_moves();
				assert(valid_moves.size() > 0);

				// Step 2B. If we are in the early opening, play random moves.
				if (move_count < first_random_moves) {
					unsigned int index = rng() % valid_moves.size();
					assert(index < valid_moves.size());

					// Step 2B.1. Make the move and check if it's game-ending (unlikely)
					searcher->pos->make_move(valid_moves[index]);
					move_count++;

					game_result = game_ended();

					if (game_result != NOT_DONE) { return; } // We return since we wont be using the very early positions anyways.
				}

				// Step 2C. Set up a search and run it.
				prepare_search();
				SearchPv pvLine;

				int score = Search::search_root(searcher, depth, -INF, INF, &pvLine);
				assert(pvLine.pv[0] != NOMOVE);

				// Step 2C.1. All scores should be relative to white, so if we're black, reverse it.
				if (searcher->pos->side_to_move == BLACK) {
					score *= -1;
				}

				// Step 2D. If we're playing randomly, play a random move. Otherwise, play the best move.
				if (random_mover) {
					int index = rng() & valid_moves.size();
					assert(index < valid_moves.size());

					searcher->pos->make_move(valid_moves[index]);
				}
				else {
					Move_t bm; bm.move = pvLine.pv[0]; bm.score = 0;
					searcher->pos->make_move(&bm);
				}
				move_count++;

				// Step 2E. Check if the game has ended. If it has, break.
				game_result = game_ended();
				if (game_result != NOT_DONE) {
					break;
				}

				// Step 2F. Append the current position.
				// Note: Remember to see if the position satisfies the required criteria.
				if (score <= eval_limit && score >= -eval_limit) {
					SavedBoard sb;

					sb.setup(searcher->pos, score);

					// Save this board.
					game_positions.push_back(sb);
				}
			}

			// Step 3. Now that the game is done, save the result to all positions in the game vector.
			assert(game_result != NOT_DONE);
			double numerical_result = (game_result == WHITE_WIN) ? 1.0 : ((game_result == BLACK_WIN) ? 0.0 : 0.5);

			for (int i = 0; i < game_positions.size(); i++) {
				game_positions[i].game_result = numerical_result;
			}

			// Step 4. Now append all game positions to the total positions.
			positions.reserve(positions.size() + game_positions.size());
			for (int i = 0; i < game_positions.size(); i++) {
				positions.push_back(game_positions[i]);
			}

			if (verbose) {
				std::cout << "Generated " << game_positions.size() << " positions." << std::endl;
			}
		}


		/*

		This method will run games as the Arbiter until we have reached the desired amount of positions.

		*/
		void Arbiter::run() {

			// Step 1. Play games until we have generated the desired amount of positions.
			positions.clear();

			while (positions.size() <= position_count) {
				play_game();
			}

			// Step 2. If we have too many positions, erase the ones in the back.
			if (positions.size() > position_count) {
				positions.erase(positions.begin() + position_count, positions.end());
			}

			assert(positions.size() == position_count);
		}
	}




	namespace Analysis {
		// Constructor for the thread analyzer
		ThreadAnalyzer::ThreadAnalyzer(const std::vector<std::string>& all_fens, size_t start, size_t end, unsigned int _d) : search_depth(_d) {
			assert(start < end);

			// Step 1. Copy the positions that we should analyze.
			for (size_t i = start; i < end; i++) {
				my_fens.push_back(all_fens[i]);
			}
		}


		/*
		
		Analyze all the positions we have been given.
		
		*/
		void ThreadAnalyzer::run() {

			// Step 1. Set up a searchthread object.
			SearchThread_t* searcher = new SearchThread_t;

			// Step 2. Loop through all positions.
			for (int p = 0; p < my_fens.size(); p++) {
				// Step 2A. Parse the FEN.
				searcher->pos->parseFen(my_fens[p]);

				// Step 2B. Setup the info-object and analyze the position.
				searcher->info->clear();
				searcher->info->timeset = false; searcher->info->depth = search_depth;

				SearchPv pvLine;
				int score = Search::search_root(searcher, search_depth, -INF, INF, &pvLine);

				// If we're black, inverse the score.
				if (searcher->pos->side_to_move == BLACK) { score *= -1; }


			}
		}
	}




}