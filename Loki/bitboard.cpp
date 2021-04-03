#include "bitboard.h"

#include <random>

Bitboard BBS::knight_attacks[64] = { 0 };

void BBS::init_knightAttacks() {
	Bitboard knightAttackBrd = 0;

	// We'll loop through all squares and add knight attacks as if it was an empty board.
	for (int sq = 0; sq < 64; sq++) {
		knightAttackBrd = 0;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~FileMasks8[FILE_H]) << 17;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~FileMasks8[FILE_A]) << 15;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_G] | FileMasks8[FILE_H])) << 10;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_G] | FileMasks8[FILE_H])) >> 6;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~FileMasks8[FILE_H]) >> 15;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~FileMasks8[FILE_A]) >> 17;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_A] | FileMasks8[FILE_B])) << 6;
		knightAttackBrd |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_A] | FileMasks8[FILE_B])) >> 10;

		knight_attacks[sq] = knightAttackBrd;
	}

}


Bitboard BBS::king_attacks[64] = { 0 };

void BBS::init_kingAttacks() {
	Bitboard attackBrd = 0;

	for (int sq = 0; sq < 64; sq++) {
		attackBrd = 0;
		Bitboard sqBrd = uint64_t(1) << sq;

		attackBrd |= (sqBrd & ~RankMasks8[RANK_8]) << 8;
		attackBrd |= (sqBrd & ~RankMasks8[RANK_1]) >> 8;
		attackBrd |= (sqBrd & ~FileMasks8[FILE_H]) << 1;
		attackBrd |= (sqBrd & ~FileMasks8[FILE_A]) >> 1;

		attackBrd |= (sqBrd & ~(RankMasks8[RANK_8] | FileMasks8[FILE_A])) << 7;
		attackBrd |= (sqBrd & ~(RankMasks8[RANK_8] | FileMasks8[FILE_H])) << 9;
		attackBrd |= (sqBrd & ~(RankMasks8[RANK_1] | FileMasks8[FILE_A])) >> 9;
		attackBrd |= (sqBrd & ~(RankMasks8[RANK_1] | FileMasks8[FILE_H])) >> 7;



		king_attacks[sq] = attackBrd;
	}
}


Bitboard BBS::Zobrist::piece_keys[2][6][64] = { {{0}} };
Bitboard BBS::Zobrist::empty_keys[64] = { 0 };
Bitboard BBS::Zobrist::side_key = 0;
Bitboard BBS::Zobrist::castling_keys[16] = { 0 };


void BBS::Zobrist::init_zobrist() {
	std::mt19937_64 rng(0x1234);

	for (int pce = PAWN; pce < NO_TYPE; pce++) {

		for (int sq = 0; sq < 64; sq++) {
			piece_keys[WHITE][pce][sq] = rng();
			piece_keys[BLACK][pce][sq] = rng();
		}
	}

	for (int sq = 0; sq < 64; sq++) {
		empty_keys[sq] = rng();
	}

	// Side Key
	side_key = rng();

	for (int c = 0; c < 16; c++) {
		castling_keys[c] = rng();
	}
}



Bitboard BBS::EvalBitMasks::passed_pawn_masks[2][64] = { {0} };
Bitboard BBS::EvalBitMasks::isolated_bitmasks[8] = { 0 };
Bitboard BBS::EvalBitMasks::outpost_masks[2][64] = { {0} };
Bitboard BBS::EvalBitMasks::rear_span_masks[2][64] = { {0} };
Bitboard BBS::EvalBitMasks::backwards_masks[2][64] = { {0} };

void BBS::EvalBitMasks::initBitMasks() {
	Bitboard bitmask = 0;
	/*
	Passed pawn bitmasks
	*/
	for (int sq = 0; sq < 64; sq++) {
		bitmask = 0;

		int r = sq / 8;
		int f = sq % 8;


		Bitboard flankmask = 0;

		flankmask |= FileMasks8[f];

		if (f > FILE_A) {
			flankmask |= FileMasks8[f - 1];
		}
		if (f < FILE_H) {
			flankmask |= FileMasks8[f + 1];
		}

		// Create the bitmask for the white pawns
		for (int i = r + 1; i <= RANK_8; i++) {
			bitmask |= (flankmask & RankMasks8[i]);
		}
		passed_pawn_masks[WHITE][sq] = bitmask;

		// Now do it for the black ones.
		bitmask = 0;
		for (int i = r - 1; i >= RANK_1; i--) {
			bitmask |= (flankmask & RankMasks8[i]);
		}
		passed_pawn_masks[BLACK][sq] = bitmask;
	}



	/*
	Isolated bitmasks
	*/
	
	for (int f = FILE_A; f <= FILE_H; f++) {
		bitmask = 0;

		// If a pawn is isolated, there are no pawns on the files directly next to it.
		// We shouln't include the file that the pawn is on itself, since we'd not be able to recognize it as isolated if it were doubled.
		bitmask = ((f > FILE_A) ? FileMasks8[f - 1] : 0) | ((f < FILE_H) ? FileMasks8[f + 1] : 0);

		isolated_bitmasks[f] = bitmask;
	}

	/*
	
	Outpost masks -> these can just be made by AND'ing the isolated bitmasks and passed pawn bitmasks.
	
	*/
	for (int sq = 0; sq < 64; sq++) {

		outpost_masks[WHITE][sq] = (passed_pawn_masks[WHITE][sq] & isolated_bitmasks[sq % 8]);
		outpost_masks[BLACK][sq] = (passed_pawn_masks[BLACK][sq] & isolated_bitmasks[sq % 8]);

	}


	/*
	
	Rearspan bitmasks. The squares behind pawns.
	
	*/

	for (int sq = 0; sq < 64; sq++) {
		rear_span_masks[WHITE][sq] = (passed_pawn_masks[BLACK][sq] & BBS::FileMasks8[sq % 8]);
		rear_span_masks[BLACK][sq] = (passed_pawn_masks[WHITE][sq] & BBS::FileMasks8[sq % 8]);
	}


	/*
	
	Backwards bitmasks. These are the squares on the current rank and all others behind it, on the adjacent files if a square.
	
	*/

	for (int sq = 0; sq < 64; sq++) {

		backwards_masks[WHITE][sq] = (passed_pawn_masks[BLACK][sq] & ~BBS::FileMasks8[sq % 8]);
		backwards_masks[BLACK][sq] = (passed_pawn_masks[WHITE][sq] & ~BBS::FileMasks8[sq % 8]);

		if (sq % 8 != FILE_H) {
			backwards_masks[WHITE][sq] |= (uint64_t(1) << (sq + 1));
			backwards_masks[BLACK][sq] |= (uint64_t(1) << (sq + 1));
		}
		if (sq % 8 != FILE_A) {
			backwards_masks[WHITE][sq] |= (uint64_t(1) << (sq - 1));
			backwards_masks[BLACK][sq] |= (uint64_t(1) << (sq - 1));
		}
	}

}



void BBS::INIT() {
	init_knightAttacks();
	init_kingAttacks();

	Zobrist::init_zobrist();

	EvalBitMasks::initBitMasks();

}



void Magics::INIT() {
	_initialize_slider_tables(true);
	_initialize_slider_tables(false);
}



namespace Magics {

	Bitboard _bishopAttacks[64][512] = { {0} };
	Bitboard _rookAttacks[64][4096] = { {0} };

	Bitboard magic_rook_masks[64] = { 0 };
	Bitboard magic_bishop_masks[64] = { 0 };


	Bitboard set_occupancy(int index, int bit_cnt, Bitboard mask) {
		Bitboard occupancy = 0;

		for (int count = 0; count < bit_cnt; count++) {

			int square = bitScanForward(mask);

			mask ^= (uint64_t(1) << square);

			if (index & (uint64_t(1) << count)) {
				occupancy |= (uint64_t(1) << square);
			}

		}
		return occupancy;
	}

	template<>
	Bitboard _getSlowAttack<BISHOP>(int sq, Bitboard occupied) {
		Bitboard attackBrd = 0;

		/*
		NORTH WEST
		*/

		int r = (sq / 8) + 1;
		int f = (sq % 8) - 1;

		int cnt = 0;

		while ((r <= RANK_8 && f >= FILE_A) && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			r++;
			f--;
		}

		/*
		NORTH EAST
		*/


		r = (sq / 8) + 1;
		f = (sq % 8) + 1;

		cnt = 0;

		while ((r <= RANK_8 && f <= FILE_H) && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			r++;
			f++;
		}

		/*
		SOUTH WEST
		*/

		r = (sq / 8) - 1;
		f = (sq % 8) - 1;

		cnt = 0;

		while ((r >= RANK_1 && f >= FILE_A) && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			r--;
			f--;
		}

		/*
		SOUTH EAST
		*/

		r = (sq / 8) - 1;
		f = (sq % 8) + 1;

		cnt = 0;

		while ((r >= RANK_1 && f <= FILE_H) && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			r--;
			f++;
		}



		return attackBrd;
	}

	template<>
	Bitboard _getSlowAttack<ROOK>(int sq, Bitboard occupied) {
		Bitboard attackBrd = 0;

		/*
		NORTH
		*/
		int r = (sq / 8) + 1;
		int f = sq % 8;

		int cnt = 0;

		while (r <= RANK_8 && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			r++;
		}


		/*
		SOUTH
		*/

		r = (sq / 8) - 1;
		f = sq % 8;

		cnt = 0;

		while (r >= RANK_1 && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			r--;
		}


		/*
		EAST
		*/

		f = (sq % 8) + 1;
		r = sq / 8;

		cnt = 0;

		while (f <= FILE_H && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			f++;
		}


		/*
		WEST
		*/

		f = (sq % 8) - 1;
		r = sq / 8;

		cnt = 0;

		while (f >= FILE_A && cnt < 1) {
			attackBrd |= uint64_t(1) << (r * 8 + f);

			if (((uint64_t(1) << (r * 8 + f)) & occupied) != 0) {
				cnt++;
			}

			f--;
		}
	
		
		return attackBrd;
	}



	namespace {

		// mask bishop attacks
		Bitboard bishopMask(int square)
		{
			// attack bitboard
			Bitboard attacks = 0ULL;

			// init files & ranks
			int f, r;

			// init target files & ranks
			int tr = square / 8;
			int tf = square % 8;

			// generate attacks
			for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
			for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
			for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
			for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

			// return attack map for bishop on a given square
			return attacks;
		}

		// mask rook attacks
		Bitboard rookMask(int square)
		{
			// attacks bitboard
			Bitboard attacks = 0ULL;

			// init files & ranks
			int f, r;

			// init target files & ranks
			int tr = square / 8;
			int tf = square % 8;

			// generate attacks
			for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
			for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
			for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
			for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

			// return attack map for bishop on a given square
			return attacks;
		}
	}


	void _initialize_slider_tables(bool is_rook) {

		// Loop through each square.
		for (int sq = 0; sq < 64; sq++) {

			magic_bishop_masks[sq] = bishopMask(sq);
			magic_rook_masks[sq] = rookMask(sq);

			Bitboard mask = (is_rook) ? rookMask(sq) : bishopMask(sq);

			int bit_count = countBits(mask);

			int occupancy_variations = uint64_t(1) << bit_count;

			for (int count = 0; count < occupancy_variations; count++) {
				Bitboard occupancy = set_occupancy(count, bit_count, mask);

				if (is_rook) {
					uint64_t magic_index = occupancy * rook_magics[sq] >> (64 - rook_relevant_bits[sq]);
					_rookAttacks[sq][magic_index] = _getSlowAttack<ROOK>(sq, occupancy);
				}

				else {
					uint64_t magic_index = occupancy * bishop_magics[sq] >> (64 - bishop_relevant_bits[sq]);
					_bishopAttacks[sq][magic_index] = _getSlowAttack<BISHOP>(sq, occupancy);
				}
			}

		}

	}



	template <>
	Bitboard attacks_bb<ROOK>(int sq, Bitboard occ) {
		//return _rookAttacks[sq][(occ & magic_rook_masks[sq]) * rook_magics[sq] >> 64 - rook_relevant_bits[sq]];
		occ &= magic_rook_masks[sq];
		occ *= rook_magics[sq];
		occ >>= (64 - rook_relevant_bits[sq]);

		return _rookAttacks[sq][occ];
	}

	template <>
	Bitboard attacks_bb<BISHOP>(int sq, Bitboard occ) {
		//return _bishopAttacks[sq][(occ & magic_bishop_masks[sq]) * bishop_magics[sq] >> 64 - bishop_relevant_bits[sq]];
		occ &= magic_bishop_masks[sq];
		occ *= bishop_magics[sq];
		occ >>= (64 - bishop_relevant_bits[sq]);

		return _bishopAttacks[sq][occ];
	}

	template <>
	Bitboard attacks_bb<QUEEN>(int sq, Bitboard occ) {
		return (attacks_bb<ROOK>(sq, occ) | attacks_bb<BISHOP>(sq, occ));
	}

}