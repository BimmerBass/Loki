//#include "movegen.h"
#include "position.h"
#include "psqt.h"


template <>
bool GameState_t::can_castle<WKCA>() {
	return (((castleRights >> WKCA) & 1) == 0) ? false : true;
}

template <>
bool GameState_t::can_castle<WQCA>() {
	return (((castleRights >> WQCA) & 1) == 0) ? false : true;
}

template <>
bool GameState_t::can_castle<BKCA>() {
	return (((castleRights >> BKCA) & 1) == 0) ? false : true;
}

template <>
bool GameState_t::can_castle<BQCA>() {
	return (((castleRights >> BQCA) & 1) == 0) ? false : true;
}

void GameState_t::generate_poskey() {
	posKey = 0;
	
	int index = 0;
	for (int pce = PAWN; pce < NO_TYPE; pce++) {
		Bitboard pceBrdW = pieceBBS[pce][WHITE];
		Bitboard pceBrdB = pieceBBS[pce][BLACK];

		while (pceBrdW) {
			index = PopBit(&pceBrdW);

			posKey ^= BBS::Zobrist::piece_keys[WHITE][pce][index];
		}

		while (pceBrdB) {
			index = PopBit(&pceBrdB);

			posKey ^= BBS::Zobrist::piece_keys[BLACK][pce][index];
		}
	}

	if (enPasSq != NO_SQ) {
		posKey ^= BBS::Zobrist::empty_keys[enPasSq];
	}

	posKey ^= (side_to_move == WHITE) ? BBS::Zobrist::side_key : 0;

	posKey ^= BBS::Zobrist::castling_keys[castleRights];
}

void GameState_t::displayBoardState() {
	std::string output = "................................................................";
	int index = 0;
	for (int i = PAWN; i <= KING; i++) {
		Bitboard whiteBB = pieceBBS[i][WHITE];
		Bitboard blackBB = pieceBBS[i][BLACK];

		while (whiteBB) {
			index = PopBit(&whiteBB);

			switch (i) {
			case 0: output[index] = 'P'; break;
			case 1:	output[index] = 'N'; break;
			case 2:	output[index] = 'B'; break;
			case 3:	output[index] = 'R'; break;
			case 4:	output[index] = 'Q'; break;
			case 5:	output[index] = 'K'; break;
			}
		}

		while (blackBB) {
			index = PopBit(&blackBB);

			switch (i) {
			case 0: output[index] = 'p'; break;
			case 1:	output[index] = 'n'; break;
			case 2:	output[index] = 'b'; break;
			case 3:	output[index] = 'r'; break;
			case 4:	output[index] = 'q'; break;
			case 5:	output[index] = 'k'; break;
			}
		}
	}

	std::string sideToMove = (side_to_move == WHITE) ? "White" : "Black";
	std::cout << "\nGame state:\n\n";
	for (int r = 7; r >= 0; r--) {
		std::cout << r + 1 << "  ";
		for (int f = 0; f < 8; f++) {
			std::cout << output[8 * r + f] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	std::cout << "   a b c d e f g h" << std::endl;
	std::cout << "\n    Side to move: " << sideToMove << std::endl;
	std::printf("	    Position key: %llX\n", posKey);
}


void GameState_t::clearPos() {

	fiftyMove = 0;

	pieceBBS[PAWN][WHITE] = 0;
	pieceBBS[KNIGHT][WHITE] = 0;
	pieceBBS[BISHOP][WHITE] = 0;
	pieceBBS[ROOK][WHITE] = 0;
	pieceBBS[QUEEN][WHITE] = 0;
	pieceBBS[KING][WHITE] = 0;


	pieceBBS[PAWN][BLACK] = 0;
	pieceBBS[KNIGHT][BLACK] = 0;
	pieceBBS[BISHOP][BLACK] = 0;
	pieceBBS[ROOK][BLACK] = 0;
	pieceBBS[QUEEN][BLACK] = 0;
	pieceBBS[KING][BLACK] = 0;

	for (int i = 0; i < 64; i++) {
		piece_list[WHITE][i] = NO_TYPE;
		piece_list[BLACK][i] = NO_TYPE;
	}

	side_to_move = WHITE;

	all_pieces[0] = 0;
	all_pieces[1] = 0;

	enPasSq = NO_SQ;

	castleRights = 0;

	ply = 0;
	fiftyMove = 0;

	posKey = 0;

	history_ply = 0;
}


void GameState_t::parseFen(const std::string FEN_STR) {
	clearPos();

	int r = RANK_8;
	int f = FILE_A;

	int piece = 0;
	int color = WHITE;

	int sq = 0;

	char* fen = new char[FEN_STR.length() + 1];
	
#if defined(_MSC_VER)
	strcpy_s(fen, FEN_STR.length() + 1, FEN_STR.c_str());
#else
	strcpy(fen, FEN_STR.c_str());
#endif

	int count = 0;

	while (r >= RANK_1 && *fen) {
		count = 1;

		switch (*fen) {
		case 'P': piece = PAWN; color = WHITE; break;
		case 'N': piece = KNIGHT; color = WHITE; break;
		case 'B': piece = BISHOP; color = WHITE; break;
		case 'R': piece = ROOK; color = WHITE; break;
		case 'Q': piece = QUEEN; color = WHITE; break;
		case 'K': piece = KING; color = WHITE; break;

		case 'p': piece = PAWN; color = BLACK; break;
		case 'n': piece = KNIGHT; color = BLACK; break;
		case 'b': piece = BISHOP; color = BLACK; break;
		case 'r': piece = ROOK; color = BLACK; break;
		case 'q': piece = QUEEN; color = BLACK; break;
		case 'k': piece = KING; color = BLACK; break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			piece = NO_TYPE;
			count = *fen - '0';
			break;


		case '/':
		case ' ':
			r--;
			f = FILE_A;
			fen++;
			continue;

		default:
			std::cout << "FEN Error" << std::endl;
			return;
		}

		for (int i = 0; i < count; i++) {
			int index = r * 8 + f;
			piece_list[color][index] = piece;
			f++;
		}
		fen++;
	}

	assert(*fen == 'w' || *fen == 'b');

	side_to_move = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	for (int i = 0; i < 4; i++) {
		if (*fen == ' ') {
			break;
		}
		else if (*fen == 'K') {
			castleRights |= (1 << WKCA);
		}
		else if (*fen == 'Q') {
			castleRights |= (1 << WQCA);
		}
		else if (*fen == 'k') {
			castleRights |= (1 << BKCA);
		}
		else if (*fen == 'q') {
			castleRights |= (1 << BQCA);
		}

		/*switch (*fen) {
		case 'K': position->castleRights |= (1 << WKCA);
		case 'Q': position->castleRights |= (1 << WQCA);
		case 'k': position->castleRights |= (1 << BKCA);
		case 'q': position->castleRights |= (1 << BQCA);
		}*/

		fen++;
	}
	fen++;

	if (*fen != '-') {
		f = fen[0] - 'a';
		r = fen[1] - '1';

		assert(f >= FILE_A && f <= FILE_H);
		assert(r >= RANK_1 && r <= RANK_8);

		enPasSq = 8 * r + f;
	}

	for (int pce = PAWN; pce <= KING; pce++) {

		for (sq = 0; sq < 64; sq++) {
			if (piece_list[WHITE][sq] == pce) {
				pieceBBS[pce][WHITE] |= (uint64_t(1) << sq);

				if (pce == KING) {
					king_squares[WHITE] = sq;
				}
			}
			if (piece_list[BLACK][sq] == pce) {
				pieceBBS[pce][BLACK] |= (uint64_t(1) << sq);

				if (pce == KING) {
					king_squares[BLACK] = sq;
				}
			}
		}
	}


	if (!lists_match()) {
		std::cout << "FEN parsing error: piece_list and pieceBBS doesn't match" << std::endl;
		return;
	}

	all_pieces[WHITE] = (pieceBBS[PAWN][WHITE] | pieceBBS[KNIGHT][WHITE] | pieceBBS[BISHOP][WHITE] | pieceBBS[ROOK][WHITE] 
		| pieceBBS[QUEEN][WHITE] | pieceBBS[KING][WHITE]);
	all_pieces[BLACK] = (pieceBBS[PAWN][BLACK] | pieceBBS[KNIGHT][BLACK] | pieceBBS[BISHOP][BLACK] | pieceBBS[ROOK][BLACK] 
		| pieceBBS[QUEEN][BLACK] | pieceBBS[KING][BLACK]);


	// Generate the position hash key
	generate_poskey();
}


/*

Function to see if the king is in check.

*/

bool GameState_t::square_attacked(int square, SIDE side) const {

	// We'll start by seeing if the king is in check by sliders because they are the fastest to look up.
	Bitboard attacks = 0;

	Bitboard occ = all_pieces[WHITE] | all_pieces[BLACK];


	attacks = Magics::attacks_bb<ROOK>(square, occ);
	if ((attacks & (pieceBBS[ROOK][side] | pieceBBS[QUEEN][side])) != 0) {
		return true;
	}


	attacks = Magics::attacks_bb<BISHOP>(square, occ);
	if ((attacks & (pieceBBS[BISHOP][side] | pieceBBS[QUEEN][side])) != 0) {
		return true;
	}


	attacks = BBS::knight_attacks[square];

	if ((attacks & pieceBBS[KNIGHT][side]) != 0) {
		return true;
	}


	attacks = BBS::king_attacks[square];

	if ((attacks & pieceBBS[KING][side]) != 0) {
		return true;
	}


	if (side == WHITE) {
		attacks = (((uint64_t(1) << square) & ~BBS::FileMasks8[FILE_A]) >> 9) | (((uint64_t(1) << square) & ~BBS::FileMasks8[FILE_H]) >> 7);
	}
	else {
		attacks = (((uint64_t(1) << square) & ~BBS::FileMasks8[FILE_A]) << 7) | (((uint64_t(1) << square) & ~BBS::FileMasks8[FILE_H]) << 9);
	}

	if ((attacks & pieceBBS[PAWN][side]) != 0) {
		return true;
	}

	return false;
}

// Returns true if the side to move is in check
bool GameState_t::in_check() const {
	return (side_to_move == WHITE) ? square_attacked(king_squares[WHITE], BLACK) : square_attacked(king_squares[BLACK], WHITE);
}




// Helper function that returns false if piece_list and pieceBBS dont match
bool GameState_t::lists_match() {
	for (int sq = 0; sq < 64; sq++) {
		if (piece_list[WHITE][sq] == NO_TYPE) {
			if (((all_pieces[WHITE] >> sq) & 1) != 0) {
				return false;
			}
			continue;
		}
		else {
			if (((pieceBBS[piece_list[WHITE][sq]][WHITE] >> sq) & 1) == 0) {
				return false;
			}
		}

		if (piece_list[BLACK][sq] == NO_TYPE) {
			if (((all_pieces[BLACK] >> sq) & 1) != 0) {
				return false;
			}
			continue;
		}
		else {
			if (((pieceBBS[piece_list[BLACK][sq]][BLACK] >> sq) & 1) == 0) {
				return false;
			}
		}

		if (piece_list[WHITE][sq] != NO_TYPE && piece_list[BLACK][sq] != NO_TYPE) {
			return false;
		}
	}

	return true;
}



/*

Returns the piecetype on square 'sq' of side 'side'

*/

int GameState_t::piece_on(int sq, SIDE side) const {
	return piece_list[side][sq];
}


/*

Function for making a move.

*/


bool GameState_t::make_move(Move_t* move) {

	// Step 1. Parse information on the move
	SIDE Them = (side_to_move == WHITE) ? BLACK : WHITE;

	int origin = FROMSQ(move->move);
	int destination = TOSQ(move->move);

	int spc = SPECIAL(move->move);
	int promotion_piece = decode_promo[PROMTO(move->move)];

	int piece_moved = piece_list[side_to_move][origin];
	int piece_captured = piece_list[Them][destination];

	assert(origin >= 0 && origin <= 63);
	assert(destination >= 0 && destination <= 63);
	assert(spc >= 0 && spc <= 3);
	assert(promotion_piece >= KNIGHT && promotion_piece <= QUEEN);
	assert(piece_moved >= PAWN && piece_moved < NO_TYPE);
	assert(piece_captured >= PAWN && piece_captured <= NO_TYPE);

	// Step 2. Check if the piece_captured is a king, and return false if it is.
	if (piece_captured == KING) {
		return false;
	}

	// Step 3. Copy irreversible information about the position and save it.
	SavedInfo_t* info = &history[history_ply];

	info->move = move->move;
	info->piece_captured = piece_captured;
	info->piece_moved = piece_moved;
	info->castleRights = castleRights;
	info->fifty_moves = fiftyMove;
	info->enPasSq = enPasSq;
	info->posKey = posKey;
	history_ply++;

	posKey ^= BBS::Zobrist::castling_keys[castleRights];
	posKey ^= (enPasSq == NO_SQ) ? 0 : BBS::Zobrist::empty_keys[enPasSq];
	

	// Step 4. Remove the piece moved from the origin and place it on destination. Additionally, xor out from posKey

	pieceBBS[piece_moved][side_to_move] ^= (uint64_t(1) << origin);
	piece_list[side_to_move][origin] = NO_TYPE;

	posKey ^= BBS::Zobrist::piece_keys[side_to_move][piece_moved][origin];


	if (spc == PROMOTION) { // For promotions, another piece type should be placed on destination.
		pieceBBS[promotion_piece][side_to_move] |= (uint64_t(1) << destination);
		piece_list[side_to_move][destination] = promotion_piece;

		posKey ^= BBS::Zobrist::piece_keys[side_to_move][promotion_piece][destination];
	}
	else {
		pieceBBS[piece_moved][side_to_move] |= (uint64_t(1) << destination);
		piece_list[side_to_move][destination] = piece_moved;

		posKey ^= BBS::Zobrist::piece_keys[side_to_move][piece_moved][destination];
	}


	// Step 5. If a piece has been captured, remove it.

	if (piece_captured != NO_TYPE) {
		pieceBBS[piece_captured][Them] ^= (uint64_t(1) << destination);
		piece_list[Them][destination] = NO_TYPE;

		posKey ^= BBS::Zobrist::piece_keys[Them][piece_captured][destination];
	}

	// Step 6. If the move is a castling move, move the rook.

	if (spc == CASTLING) {
		// Here we know that if the destination index is bigger than origin, we are castling kingside, else it is queenside.
		if (destination > origin) {
			assert((side_to_move == WHITE) ? can_castle<WKCA>() : can_castle<BKCA>());
			// Firstly remove the rook from h1 for white or h8 for black
			pieceBBS[ROOK][side_to_move] ^= (uint64_t(1) << ((side_to_move == WHITE) ? H1 : H8));
			piece_list[side_to_move][(side_to_move == WHITE) ? H1 : H8] = NO_TYPE;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? H1 : H8];

			// Then place the rook on f1 for white or f8 for black
			pieceBBS[ROOK][side_to_move] |= (uint64_t(1) << ((side_to_move == WHITE) ? F1 : F8));
			piece_list[side_to_move][(side_to_move == WHITE) ? F1 : F8] = ROOK;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? F1 : F8];
		}
		else {
			assert((side_to_move == WHITE) ? can_castle<WQCA>() : can_castle<BQCA>());
			// Firstly remove the rook from a1 for white or a8 for black
			pieceBBS[ROOK][side_to_move] ^= (uint64_t(1) << ((side_to_move == WHITE) ? A1 : A8));
			piece_list[side_to_move][(side_to_move == WHITE) ? A1 : A8] = NO_TYPE;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? A1 : A8];

			// Then place the rook on d1 for white or d8 for black
			pieceBBS[ROOK][side_to_move] |= (uint64_t(1) << ((side_to_move == WHITE) ? D1 : D8));
			piece_list[side_to_move][(side_to_move == WHITE) ? D1 : D8] = ROOK;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? D1 : D8];
		}
	}

	// Step 7. If the move is an en-passant, remove the pawn being captured.
	if (spc == ENPASSANT) {
		pieceBBS[PAWN][(side_to_move == WHITE) ? BLACK : WHITE] ^= (uint64_t(1) << ((side_to_move == WHITE) ? (destination - 8) : (destination + 8)));
		piece_list[(side_to_move == WHITE) ? BLACK : WHITE][(side_to_move == WHITE) ? (destination - 8) : (destination + 8)] = NO_TYPE;

		posKey ^= BBS::Zobrist::piece_keys[Them][PAWN][(side_to_move == WHITE) ? (destination - 8) : (destination + 8)];
	}

	// Step 8. Update the castling rights --> if the king has been moved, all castling rights for that side will be removed. If a piece has moved to or from
	// either a1/h1/a8/h8, remove the castling rights for that side. We will only do this is the castling rights are still there, because they would just toggle
	// on and off if we did it every time the king moved.
	if (piece_moved == KING) {
		if (side_to_move == WHITE) {
			if (can_castle<WKCA>()) {
				castleRights ^= (1 << WKCA);
			}
			if (can_castle<WQCA>()) {
				castleRights ^= (1 << WQCA);
			}
		}
		else {
			if (can_castle<BKCA>()) {
				castleRights ^= (1 << BKCA);
			}
			if (can_castle<BQCA>()) {
				castleRights ^= (1 << BQCA);
			}
		}
	}

	if ((origin == A1 || destination == A1) && can_castle<WQCA>()) {
		castleRights ^= (1 << WQCA);
	}
	if ((origin == H1 || destination == H1) && can_castle<WKCA>()) {
		castleRights ^= (1 << WKCA);
	}
	if ((origin == A8 || destination == A8) && can_castle<BQCA>()) {
		castleRights ^= (1 << BQCA);
	}
	if ((origin == H8 || destination == H8) && can_castle<BKCA>()) {
		castleRights ^= (1 << BKCA);
	}


	// Step 9. If the piece moved was a king, we'll update the king position
	if (piece_moved == KING) {
		king_squares[side_to_move] = destination;
	}

	// Step 10. Update the occupancy bitboards.
	all_pieces[WHITE] = (pieceBBS[PAWN][WHITE] | pieceBBS[KNIGHT][WHITE] | pieceBBS[BISHOP][WHITE] |
		pieceBBS[ROOK][WHITE] | pieceBBS[QUEEN][WHITE] | pieceBBS[KING][WHITE]);
	all_pieces[BLACK] = (pieceBBS[PAWN][BLACK] | pieceBBS[KNIGHT][BLACK] | pieceBBS[BISHOP][BLACK] |
		pieceBBS[ROOK][BLACK] | pieceBBS[QUEEN][BLACK] | pieceBBS[KING][BLACK]);

	// Step 11. Update en-passant square.
	enPasSq = NO_SQ;

	if (piece_moved == PAWN && (((side_to_move == WHITE) && destination == origin + 16) || ((side_to_move == BLACK) && destination == origin - 16))) {
		enPasSq = (side_to_move == WHITE) ? destination - 8 : destination + 8;
		posKey ^= BBS::Zobrist::empty_keys[enPasSq];
	}

	// Step 12. Update the fifty-move rule and ply.
	ply += 1;
	fiftyMove += 1;

	if (piece_captured != NO_TYPE || piece_moved == PAWN) { // Reset fifty-move counter if it is a capture or pawn move.
		fiftyMove = 0;
	}
	
	// Add the new castling rights to the posKey
	posKey ^= BBS::Zobrist::castling_keys[castleRights];
	posKey ^= BBS::Zobrist::side_key;

	// Step 13. See if the king is in check. If it is, then undo the move and return false.
	if (square_attacked(king_squares[side_to_move], (side_to_move == WHITE) ? BLACK : WHITE)) {
		side_to_move = (side_to_move == WHITE) ? BLACK : WHITE; // undo_move will toggle this, so we have to change it before calling the function.
		undo_move();
		return false;
	}

	side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;

	return true;
}


/*

Function for undoing moves.

*/


void GameState_t::undo_move() {
	// If no previous moves have been made, we can't undo anything.
	if (history_ply == 0) {
		return;
	}

	// Step 1. Get all the irreversible information.
	SavedInfo_t* info = &history[history_ply - 1];

	int origin = FROMSQ(info->move);
	int destination = TOSQ(info->move);

	int spc = SPECIAL(info->move);
	int promotion_piece = decode_promo[PROMTO(info->move)];

	int piece_moved = info->piece_moved;
	int piece_captured = info->piece_captured;

	// Step 2. Toggle the side to move.
	side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;
	posKey ^= BBS::Zobrist::side_key;

	// Step 3. Remove the piece moved from the destination and place it on the origin
	if (spc != PROMOTION) { // We can't remove a pawn that is not on the destination.
		pieceBBS[piece_moved][side_to_move] ^= (uint64_t(1) << destination);
		piece_list[side_to_move][destination] = NO_TYPE;

		posKey ^= BBS::Zobrist::piece_keys[side_to_move][piece_moved][destination];
	}
	else {
		pieceBBS[promotion_piece][side_to_move] ^= (uint64_t(1) << destination);
		piece_list[side_to_move][destination] = NO_TYPE;

		posKey ^= BBS::Zobrist::piece_keys[side_to_move][promotion_piece][destination];
	}

	pieceBBS[piece_moved][side_to_move] |= (uint64_t(1) << origin);
	piece_list[side_to_move][origin] = piece_moved;

	posKey ^= BBS::Zobrist::piece_keys[side_to_move][piece_moved][origin];


	// Step 4. Place the captured piece on the destination
	if (piece_captured != NO_TYPE) {
		pieceBBS[piece_captured][(side_to_move == WHITE) ? BLACK : WHITE] |= (uint64_t(1) << destination);
		piece_list[(side_to_move == WHITE) ? BLACK : WHITE][destination] = piece_captured;

		posKey ^= BBS::Zobrist::piece_keys[(side_to_move == WHITE) ? BLACK : WHITE][piece_captured][destination];
	}

	// Step 5. If the move was a castling, place the rook back in the corner.
	if (spc == CASTLING) {
		// If destination was bigger than origin, it must've been kingside castle.
		if (destination > origin) {
			// Firstly, remove the rook from F1 for white or F8 for black
			pieceBBS[ROOK][side_to_move] ^= (uint64_t(1) << ((side_to_move == WHITE) ? F1 : F8));
			piece_list[side_to_move][(side_to_move == WHITE) ? F1 : F8] = NO_TYPE;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? F1 : F8];

			// Secondly, place the rook back on H1 for white or H8 for black.
			pieceBBS[ROOK][side_to_move] |= (uint64_t(1) << ((side_to_move == WHITE) ? H1 : H8));
			piece_list[side_to_move][(side_to_move == WHITE) ? H1 : H8] = ROOK;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? H1 : H8];
		}
		else {
			// Firstly, remove the rook from D1 for white or D8 for black
			pieceBBS[ROOK][side_to_move] ^= (uint64_t(1) << ((side_to_move == WHITE) ? D1 : D8));
			piece_list[side_to_move][(side_to_move == WHITE) ? D1 : D8] = NO_TYPE;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? D1 : D8];

			// Secondly, place the rook back on A1 for white or A8 for black.
			pieceBBS[ROOK][side_to_move] |= (uint64_t(1) << ((side_to_move == WHITE) ? A1 : A8));
			piece_list[side_to_move][(side_to_move == WHITE) ? A1 : A8] = ROOK;
			posKey ^= BBS::Zobrist::piece_keys[side_to_move][ROOK][(side_to_move == WHITE) ? A1 : A8];
		}
	}

	// Step 6. If the move was en-passant, place back the pawn.
	if (spc == ENPASSANT) {
		if (side_to_move == WHITE) {
			pieceBBS[PAWN][BLACK] |= (uint64_t(1) << (destination - 8));
			piece_list[BLACK][destination - 8] = PAWN;

			posKey ^= BBS::Zobrist::piece_keys[BLACK][PAWN][destination - 8];
		}
		else {
			pieceBBS[PAWN][WHITE] |= (uint64_t(1) << (destination + 8));
			piece_list[WHITE][destination + 8] = PAWN;

			posKey ^= BBS::Zobrist::piece_keys[WHITE][PAWN][destination + 8];
		}
	}

	// Step 7. Update the occupancy bitboards.
	all_pieces[WHITE] = (pieceBBS[PAWN][WHITE] | pieceBBS[KNIGHT][WHITE] | pieceBBS[BISHOP][WHITE] |
		pieceBBS[ROOK][WHITE] | pieceBBS[QUEEN][WHITE] | pieceBBS[KING][WHITE]);
	all_pieces[BLACK] = (pieceBBS[PAWN][BLACK] | pieceBBS[KNIGHT][BLACK] | pieceBBS[BISHOP][BLACK] |
		pieceBBS[ROOK][BLACK] | pieceBBS[QUEEN][BLACK] | pieceBBS[KING][BLACK]);

	// Step 8. If the king was moved, set the king_square to the origin.
	if (piece_moved == KING) {
		king_squares[side_to_move] = origin;
	}

	// Step 9. Set en-passant square, castling rights and fifty-move counter.
	posKey ^= BBS::Zobrist::castling_keys[castleRights];
	
	if (enPasSq != NO_SQ) {
		posKey ^= BBS::Zobrist::empty_keys[enPasSq];
	}
	enPasSq = info->enPasSq;
	
	if (enPasSq != NO_SQ) {
		posKey ^= BBS::Zobrist::empty_keys[enPasSq];
	}
	
	castleRights = info->castleRights;
	posKey ^= BBS::Zobrist::castling_keys[castleRights];

	fiftyMove = info->fifty_moves;

	// Step 10. Decrement the ply and history ply.
	ply--;
	history_ply--;
}




/*

Doing a null move

*/

int GameState_t::make_nullmove() {
	// Step 1. Change side to move
	side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;

	// Step 2. Toggle side to move in the hashkey
	posKey ^= BBS::Zobrist::side_key;

	// Step 3. If there is an en-passant square, return the index and remove it.
	if (enPasSq != NO_SQ) {
		int enPas = enPasSq;
		enPasSq = NO_SQ;

		return enPas;
	}
	return NO_SQ;
}


/*

Undoing a null-move

*/

void GameState_t::undo_nullmove(int oldEnPas) {
	// Step 1. Re-insert the old en-passant square
	enPasSq = oldEnPas;

	// Step 2. Toggle side in hashkey
	posKey ^= BBS::Zobrist::side_key;

	// Step 3. Change side to move.
	side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;
}



/*

Determines if it is safe to do null moves. True if the side to move has any slider

*/

bool GameState_t::safe_nullmove() const {
	return ((pieceBBS[BISHOP][side_to_move] | pieceBBS[ROOK][side_to_move] | pieceBBS[QUEEN][side_to_move]) != 0) ? true : false;
}


/*

Used for delta pruning. Determines if we are in a late endgame

*/

bool GameState_t::is_endgame() const {
	return ((pieceBBS[KNIGHT][side_to_move] | pieceBBS[BISHOP][side_to_move] | pieceBBS[ROOK][side_to_move] | pieceBBS[QUEEN][side_to_move]) == 0) ? true : false;
}


/*

Used for razoring

*/

bool GameState_t::non_pawn_material() const {
	SIDE Them = (side_to_move == WHITE) ? BLACK : WHITE;
	return ((pieceBBS[KNIGHT][side_to_move] | pieceBBS[BISHOP][side_to_move] | pieceBBS[ROOK][side_to_move] | pieceBBS[QUEEN][side_to_move]) != 0 ? true : false)
		&& ((pieceBBS[KNIGHT][Them] | pieceBBS[BISHOP][Them] | pieceBBS[ROOK][Them] | pieceBBS[QUEEN][Them]) != 0 ? true : false);
}


/*

Default constructor

*/

GameState_t::GameState_t() {
}

/*

Copy constructor of the GameState_t class (Usage: GameState_t newG = pos)

*/


GameState_t::GameState_t(const GameState_t& pos) {

	// Copy piece bitboards
	pieceBBS[PAWN][WHITE]	= pos.pieceBBS[PAWN][WHITE];
	pieceBBS[KNIGHT][WHITE] = pos.pieceBBS[KNIGHT][WHITE];
	pieceBBS[BISHOP][WHITE] = pos.pieceBBS[BISHOP][WHITE];
	pieceBBS[ROOK][WHITE]	= pos.pieceBBS[ROOK][WHITE];
	pieceBBS[QUEEN][WHITE]	= pos.pieceBBS[QUEEN][WHITE];
	pieceBBS[KING][WHITE]	= pos.pieceBBS[KING][WHITE];

	pieceBBS[PAWN][BLACK]	= pos.pieceBBS[PAWN][BLACK];
	pieceBBS[KNIGHT][BLACK] = pos.pieceBBS[KNIGHT][BLACK];
	pieceBBS[BISHOP][BLACK] = pos.pieceBBS[BISHOP][BLACK];
	pieceBBS[ROOK][BLACK]	= pos.pieceBBS[ROOK][BLACK];
	pieceBBS[QUEEN][BLACK]	= pos.pieceBBS[QUEEN][BLACK];
	pieceBBS[KING][BLACK]	= pos.pieceBBS[KING][BLACK];


	// Copy piece_list
	std::copy(std::begin(pos.piece_list[WHITE]), std::end(pos.piece_list[WHITE]), std::begin(piece_list[WHITE]));
	std::copy(std::begin(pos.piece_list[BLACK]), std::end(pos.piece_list[BLACK]), std::begin(piece_list[BLACK]));

	// Copy side to move
	side_to_move = pos.side_to_move;

	// Copy all pieces
	all_pieces[WHITE] = pos.all_pieces[WHITE];
	all_pieces[BLACK] = pos.all_pieces[BLACK];

	// Copy king squares
	king_squares[WHITE] = pos.king_squares[WHITE];
	king_squares[BLACK] = pos.king_squares[BLACK];

	// Copy en-passant square, castling rights, ply and fifty move counter
	enPasSq = pos.enPasSq;
	castleRights = pos.castleRights;
	ply = pos.ply;
	fiftyMove = pos.fiftyMove;

	// Copy zobrist hashkey
	posKey = pos.posKey;

	// Copy history and history ply
	std::copy(std::begin(pos.history), std::end(pos.history), std::begin(history));
	history_ply = pos.history_ply;
}




bool GameState_t::is_repetition() const {

	for (int p = 0; p < history_ply; p++) {
		// The exact same position has been reached before, so it is a repetition.
		if (history[p].posKey == posKey) {
			return true;
		}
	}

	return false;
}



void GameState_t::mirror_board() {
	int temp_pieces[2][64] = { {0} };

	for (int i = 0; i < 64; i++) {
		temp_pieces[WHITE][i] = NO_TYPE;
		temp_pieces[BLACK][i] = NO_TYPE;
	}

	SIDE temp_side = (side_to_move == WHITE) ? BLACK : WHITE;

	int tempCastlePerm = 0;
	int tempEnPas = NO_SQ;

	if (((castleRights >> WKCA) & 1) == 1) {
		tempCastlePerm |= (1 << BKCA);
	}
	if (((castleRights >> WQCA) & 1) == 1) {
		tempCastlePerm |= (1 << BQCA);
	}
	if (((castleRights >> BKCA) & 1) == 1) {
		tempCastlePerm |= (1 << WKCA);
	}
	if (((castleRights >> BQCA) & 1) == 1) {
		tempCastlePerm |= (1 << WQCA);
	}

	if (enPasSq != NO_SQ) {
		tempEnPas = PSQT::Mirror64[enPasSq];
	}

	for (int sq = 0; sq < 64; sq++) {
		temp_pieces[WHITE][sq] = piece_list[BLACK][PSQT::Mirror64[sq]];
		temp_pieces[BLACK][sq] = piece_list[WHITE][PSQT::Mirror64[sq]];
	}

	clearPos();

	for (int sq = 0; sq < 64; sq++) {
		piece_list[WHITE][sq] = temp_pieces[WHITE][sq];
		piece_list[BLACK][sq] = temp_pieces[BLACK][sq];
		
		if (piece_list[WHITE][sq] != NO_TYPE) {
			pieceBBS[piece_list[WHITE][sq]][WHITE] |= (uint64_t(1) << sq);
		}

		if (piece_list[BLACK][sq] != NO_TYPE) {
			pieceBBS[piece_list[BLACK][sq]][BLACK] |= (uint64_t(1) << sq);
		}
	}

	side_to_move = temp_side;
	castleRights = tempCastlePerm;
	enPasSq = tempEnPas;

	generate_poskey();

	all_pieces[WHITE] = (pieceBBS[PAWN][WHITE] | pieceBBS[KNIGHT][WHITE] | pieceBBS[BISHOP][WHITE] |
		pieceBBS[ROOK][WHITE] | pieceBBS[QUEEN][WHITE] | pieceBBS[KING][WHITE]);
	all_pieces[BLACK] = (pieceBBS[PAWN][BLACK] | pieceBBS[KNIGHT][BLACK] | pieceBBS[BISHOP][BLACK] |
		pieceBBS[ROOK][BLACK] | pieceBBS[QUEEN][BLACK] | pieceBBS[KING][BLACK]);

	king_squares[WHITE] = bitScanForward(pieceBBS[KING][WHITE]);
	king_squares[BLACK] = bitScanForward(pieceBBS[KING][BLACK]);

	assert(lists_match());
}




const int pieceVals[6] = { 100, 320, 350, 560,1000 };
int GameState_t::best_capture_possible() const {

	int smallest_attacker = KING;
	int biggest_victim = PAWN;

	for (int pce = PAWN; pce <= QUEEN; pce++) {

		if (pieceBBS[pce][(side_to_move == WHITE) ? BLACK : WHITE] != 0) {
			biggest_victim = pce;
		}

	}

	for (int pce = QUEEN; pce >= PAWN; pce--) {

		if (pieceBBS[pce][side_to_move] != 0) {
			smallest_attacker = pce;
		}

	}

	return (pieceVals[biggest_victim] - pieceVals[smallest_attacker]);
}




/*

Returns a bitboard with all attackers to the given square

*/

Bitboard GameState_t::attackers_to(int sq, Bitboard occupied) const {

	Bitboard attackers = 0;
	Bitboard sqBrd = uint64_t(1) << sq;

	// Pawn attackers
	attackers |= ((shift<NORTHWEST>(sqBrd) | shift<NORTHEAST>(sqBrd)) & pieceBBS[PAWN][BLACK]);
	attackers |= ((shift<SOUTHWEST>(sqBrd) | shift<SOUTHEAST>(sqBrd)) & pieceBBS[PAWN][WHITE]);

	// Knights
	attackers |= ((BBS::knight_attacks[sq]) & (pieceBBS[KNIGHT][WHITE] | pieceBBS[KNIGHT][BLACK]));

	// Bishops
	attackers |= ((Magics::attacks_bb<BISHOP>(sq, occupied)) & (pieceBBS[BISHOP][WHITE] | pieceBBS[BISHOP][BLACK]));

	// Rooks
	attackers |= ((Magics::attacks_bb<ROOK>(sq, occupied)) & (pieceBBS[ROOK][WHITE] | pieceBBS[ROOK][BLACK]));

	// Queens
	attackers |= ((Magics::attacks_bb<QUEEN>(sq, occupied)) & (pieceBBS[QUEEN][WHITE] | pieceBBS[QUEEN][BLACK]));

	// Kings
	attackers |= ((BBS::king_attacks[sq]) & (pieceBBS[KING][WHITE] | pieceBBS[KING][BLACK]));

	return attackers;
}


/*

Static Exchange Evaluation Greater Than or Equal (SEE_GE) is a good way of analyzing if captures loose or gain material before playing them.

NOTE: This implementation is heavily inspired by the one Stockfish uses.

*/

constexpr int piece_values[6] = { 100, 300, 320, 500, 900, 20000 };
bool GameState_t::see_ge(int move, int threshold) const {

	// We won't do SEE on moves like en-passant or capture promotions.
	if (SPECIAL(move) != NOT_SPECIAL) {
		return 0 >= threshold;
	}


	// Declare variables
	Bitboard stm_attackers = 0;
	int from_sq = FROMSQ(move), to_sq = TOSQ(move);
	int nextVictim = piece_on(from_sq, side_to_move); // The next piece to be captured in this sequence is the one that 'move' moves.
	SIDE Us = side_to_move;
	SIDE stm = (Us == WHITE) ? BLACK : WHITE;
	int balance = piece_values[piece_on(to_sq, stm)] - threshold;

	if (balance < 0) {
		return false;
	}


	// Assume that the opponent can capture our piece for free
	balance -= piece_values[nextVictim];

	// If our balance is greater than 0 even if the opponent can capture our piece for free, the capture is good (example: PxQ).
	if (balance >= 0) {
		return true;
	}

	// Now we need to find all attackers to the destination square. We need to remove the piece moved since there can possibly be an x-ray attacker behind it.
	Bitboard occupied = (all_pieces[BLACK] | all_pieces[WHITE]) ^ (uint64_t(1) << from_sq) ^ (uint64_t(1) << to_sq);
	occupied ^= (pinned_pieces<WHITE>() | pinned_pieces<BLACK>()); // We'll ignore all pieces that are pinned. FIXME: Let pinned pieces attack when they're not pinned any longer.
	Bitboard attackers = attackers_to(to_sq, occupied) & occupied;

	while (true) {
		stm_attackers = attackers & all_pieces[stm];

		// If there are no more attackers, we'll break the loop.
		if (!stm_attackers) {
			break;
		}

		/*
		FIXME: GET NEXT VICTIM HERE -- SEE WONT WORK WITHOUT IT
		*/

		// Switch the side to move
		stm = (stm == WHITE) ? BLACK : WHITE;



		assert(balance < 0);

		balance = -balance - 1 - piece_values[nextVictim];

		if (balance >= 0) {

			if (nextVictim == KING && (attackers & all_pieces[stm]) != 0) {
				stm = (stm == WHITE) ? BLACK : WHITE;
			}
			break;
		}
		assert(nextVictim != KING);
	}

	return (Us != stm);
}