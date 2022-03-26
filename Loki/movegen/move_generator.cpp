#include "loki.pch.h"


namespace loki::movegen {

	move_generator::attack_table_t	move_generator::knight_attacks{ 0 };
	move_generator::attack_table_t	move_generator::king_attacks{ 0 };

	move_generator::move_generator(position::position_t pos, magics::slider_generator_t slider_generator) noexcept :
		m_position(pos),
		m_slider_generator(slider_generator) {

		static std::mutex mtx;
		static std::atomic_bool tables_initialized = false;
		{
			std::lock_guard<std::mutex> lock(mtx);
			if (!tables_initialized) {
				init_knight_attacks();
				init_king_attacks();

				tables_initialized = true;
			}
		}
	}

	/// <summary>
	/// Initialize the table of attacks for knights.
	/// </summary>
	/// <returns></returns>
	void move_generator::init_knight_attacks() noexcept {

		for (auto sq = 0; sq < 64; sq++) {
			bitboard_t current_attack_mask = 0;

			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_H]) << 17;
			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_A]) << 15;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_G] | bitmasks::file_masks[FILE_H])) << 10;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_G] | bitmasks::file_masks[FILE_H])) >> 6;
			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_H]) >> 15;
			current_attack_mask |= ((uint64_t(1) << sq) & ~bitmasks::file_masks[FILE_A]) >> 17;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_A] | bitmasks::file_masks[FILE_B])) << 6;
			current_attack_mask |= ((uint64_t(1) << sq) & ~(bitmasks::file_masks[FILE_A] | bitmasks::file_masks[FILE_B])) >> 10;

			knight_attacks[sq] = current_attack_mask;
		}
	}

	/// <summary>
	/// Initialize the table of king attacks.
	/// </summary>
	/// <returns></returns>
	void move_generator::init_king_attacks() noexcept {
		for (auto sq = 0; sq < 64; sq++) {
			bitboard_t current_attack_mask = 0;
			bitboard_t sq_board = uint64_t(1) << sq;

			current_attack_mask |= (sq_board & ~bitmasks::rank_masks[RANK_8]) << 8;
			current_attack_mask |= (sq_board & ~bitmasks::rank_masks[RANK_1]) >> 8;
			current_attack_mask |= (sq_board & ~bitmasks::file_masks[FILE_H]) << 1;
			current_attack_mask |= (sq_board & ~bitmasks::file_masks[FILE_A]) >> 1;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_8] | bitmasks::file_masks[FILE_A])) << 7;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_8] | bitmasks::file_masks[FILE_H])) << 9;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_1] | bitmasks::file_masks[FILE_A])) >> 9;
			current_attack_mask |= (sq_board & ~(bitmasks::rank_masks[RANK_1] | bitmasks::file_masks[FILE_H])) >> 7;


			king_attacks[sq] = current_attack_mask;
		}
	}
}