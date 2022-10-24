//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef MAGICS_INDEX_H
#define MAGICS_INDEX_H

namespace loki::movegen::magics
{

	/// <summary>
	/// Implementation of a plain magic bitboards index
	/// (see https://www.chessprogramming.org/Magic_Bitboards#Plain)
	/// </summary>
	template<PIECE _Pce>
	class magics_index
	{
	private:
		static_assert(_Pce == BISHOP || _Pce == ROOK, "piece type must be a bishop or rook.");

		/// <summary>
		/// Size of the index per square -> the amount of occupancy variations that exists.
		/// </summary>
		inline static constexpr auto index_size = []() {
			if constexpr (_Pce == BISHOP)
				return 512;
			else
				return 4096;
		}();

		// Even though nested classes/structs are bad, it'll be okay here since only magics_index should be able to see it anyways.
		struct magic_entry
		{
			bitboard_t mask;
			bitboard_t magic;
			size_t shift;
		};

		using index_t = std::array<
			std::array<bitboard_t, index_size>,
			64
		>;

		// All attacks.
		std::unique_ptr<index_t> m_attack_index{ nullptr };

		// Magics and masks.
		std::array<magic_entry, 64> m_magic_entries;
	public:
		magics_index();

		bitboard_t attacks_bb(SQUARE sq, bitboard_t occupancy) const noexcept;

	private:
		void init() noexcept;

		void initialize_entry_table() noexcept;
	private:
#pragma region Pre-initialized magics array
		inline static constexpr std::array<bitboard_t, 64> magics = [] {
			if constexpr (_Pce == BISHOP)
			{
				return std::array<bitboard_t, 64> {
					0x4151002060840ULL, 0x403060403020001ULL, 0x1800c400800010ULL, 0x2848c100080024ULL,
						0x84050420e00001ULL, 0x4406090460180001ULL, 0x4014120846090024ULL, 0x4808150c01044004ULL,
						0x40448020400ULL, 0x100850a2020400b4ULL, 0x4042440800810080ULL, 0x7092440c02805000ULL,
						0x200040308220000ULL, 0xe08420804e6ULL, 0x100020801043008ULL, 0x1801021200822800ULL,
						0x10002002624800ULL, 0x308001202081202ULL, 0x480800c418011010ULL, 0x822000406120001ULL,
						0x204000822080560ULL, 0x4004082202020200ULL, 0x2012010c48140408ULL, 0x2021000022982400ULL,
						0x20080110100103ULL, 0x14200010110100ULL, 0x204808100b014100ULL, 0x20090080040c0ULL,
						0x4001010040104011ULL, 0x3910a00040a0111ULL, 0x4005140002c20820ULL, 0x5243020001055102ULL,
						0x2002084050041000ULL, 0x24a02202c300124ULL, 0x2882030102900041ULL, 0x10510801040040ULL,
						0x4000404040440100ULL, 0x1008050102080880ULL, 0x410800a401010100ULL, 0x2004211240062410ULL,
						0x22d11301004401cULL, 0x582828010c04ULL, 0xa10140024020808ULL, 0xc20004200800800ULL,
						0x4202022a000400ULL, 0x414112020e000c10ULL, 0x100441940c000509ULL, 0xa102c0440c10080ULL,
						0x4081010802411801ULL, 0x414404404200001ULL, 0x20004c4454100801ULL, 0x810000a10440000ULL,
						0x80080803040804ULL, 0x540408020000ULL, 0x1020200202004820ULL, 0x484108200510230ULL,
						0x600e010051042020ULL, 0x4404880880ULL, 0x428044044400ULL, 0x1400000208420202ULL,
						0x4000004004904400ULL, 0x10200a1408104101ULL, 0x48004002184200a0ULL, 0x2041000850100ULL
				};
			}
			else
			{
				return std::array<bitboard_t, 64> {
					0x2380004000201080ULL, 0x2040100020004001ULL, 0x180086002100080ULL, 0x4080048008021000ULL,
						0xa00086004100200ULL, 0x80018004004200ULL, 0x400640810030082ULL, 0x4280014100102080ULL,
						0x80a002600450080ULL, 0x4005004004802100ULL, 0x81004104102000ULL, 0x3441000921021000ULL,
						0x2002010460008ULL, 0x806001004020008ULL, 0x3541002100140200ULL, 0x25000260810002ULL,
						0x800040042002d2ULL, 0x10104000442000ULL, 0x50028010802000ULL, 0xf0008028001080ULL,
						0x8008008040080ULL, 0x10100080c0002ULL, 0xa00040002010890ULL, 0xa42020000670084ULL,
						0xc80084040002000ULL, 0x200640005006ULL, 0x51410100200091ULL, 0x101c900201000ULL,
						0x21080100050010ULL, 0x242001200190410ULL, 0x1a1400081011ULL, 0x1204200140091ULL,
						0x1000400020800088ULL, 0x1100442000401000ULL, 0x204102001004100ULL, 0x4006102202004008ULL,
						0x28010400800881ULL, 0x80100040100081eULL, 0x101100e24000148ULL, 0x44004412000981ULL,
						0x480094220084000ULL, 0x10200050024002ULL, 0x61002000110042ULL, 0x5005001890021ULL,
						0x8010500090010ULL, 0x9000864010002ULL, 0x501100142440008ULL, 0x12a040040820001ULL,
						0xa44408209002200ULL, 0x1010084000200040ULL, 0x208012004200ULL, 0x2243001001900ULL,
						0x1022000810042200ULL, 0x1c22000830040600ULL, 0x800100508220400ULL, 0x1000a44820300ULL,
						0x81430110800021ULL, 0x80401100260082ULL, 0x2001010830c1ULL, 0x200210049045001ULL,
						0x42060088209c3042ULL, 0x700a604001811ULL, 0x80201100084ULL, 0x168004a21040086ULL
				};
			}
		}();
#pragma endregion
	};

	template<PIECE _Pce>
	using magics_index_t = std::unique_ptr<magics_index<_Pce>>;
}

#endif