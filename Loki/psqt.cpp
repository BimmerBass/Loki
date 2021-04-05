#include "psqt.h"




namespace PSQT {

#define S(mg, eg) Score(mg, eg)


	const Score PawnTable[64] = {
		S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)		,
		S(-15, 31)		,	S(15, 31)		,	S(-22, 35)		,	S(-4, 27)		,	S(-2, 7)		,	S(32, 25)		,	S(51, 23)		,	S(1, 5)		,
		S(-15, 21)		,	S(7, 23)		,	S(4, 11)		,	S(-11, 29)		,	S(13, 17)		,	S(2, 23)		,	S(47, 17)		,	S(-1, 9)	,
		S(-22, 37)		,	S(-6, 35)		,	S(-16, 11)		,	S(13, 1)		,	S(-1, 15)		,	S(4, 9)			,	S(-8, 33)		,	S(-24, 23)	,
		S(-6, 63)		,	S(3, 51)		,	S(5, 29)		,	S(-8, 39)		,	S(16, 3)		,	S(13, 25)		,	S(-5, 55)		,	S(-26, 49)	,
		S(18, 119)		,	S(-8, 133)		,	S(6, 113)		,	S(12, 89)		,	S(-2, 91)		,	S(30, 87)		,	S(-4, 125)		,	S(32, 133)	,
		S(69, 149)		,	S(27, 145)		,	S(47, 103)		,	S(31, 125)		,	S(23, 115)		,	S(37, 123)		,	S(31, 127)		,	S(15, 133)	,
		S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)			,	S(0, 0)
	};


	const Score KnightTable[64] = {
		S(-55, -37)		,	S(-14, -10)		,	S(-47, -15)		,	S(-51, 15)		,	S(1, -5)		,	S(-33, 13)		,	S(-12, -36)		,	S(-17, -69)		,
		S(-57, 5)		,	S(23, -23)		,	S(-11, -11)		,	S(15, -1)		,	S(17, -3)		,	S(19, 5)		,	S(-17, 29)		,	S(-7, -23)		,
		S(-22, -2)		,	S(-13, 13)		,	S(9, -9)		,	S(5, 19)		,	S(15, 21)		,	S(25, -9)		,	S(37, -19)		,	S(-6, -24)		,
		S(-3, 1)		,	S(29, 5)		,	S(1, 35)		,	S(19, 37)		,	S(27, 29)		,	S(31, 23)		,	S(-1, 31)		,	S(7, -3)		,
		S(-3, 5)		,	S(13, 17)		,	S(13, 31)		,	S(59, 35)		,	S(43, 33)		,	S(79, 21)		,	S(27, 17)		,	S(49, -15)		,
		S(-24, -26)		,	S(31, -1)		,	S(72, -3)		,	S(39, 23)		,	S(100, -11)		,	S(29, 27)		,	S(73, -29)		,	S(8, -12)		,
		S(-7, -39)		,	S(-19, 1)		,	S(47, 7)		,	S(17, 9)		,	S(5, -5)		,	S(35, -13)		,	S(-35, 21)		,	S(9, -57)		,
		S(-59, -52)		,	S(22, -32)		,	S(-43, -29)		,	S(-25, -29)		,	S(1, -7)		,	S(-41, -9)		,	S(-40, 8)		,	S(-5, -73)
	};


	const Score BishopTable[64] = {
		S(9, -29)	,	S(-16, 23)	,	S(-11, 6)	,	S(-2, 3)	,	S(8, 4)		,	S(-16, 10)	,	S(-34, 18)	,	S(-19, 32)	,
		S(73, -46)	,	S(17, -14)	,	S(12, -2)	,	S(-1, 1)	,	S(8, 3)		,	S(6, 0)		,	S(36, -10)	,	S(17, -27)	,
		S(-2, -10)	,	S(19, -12)	,	S(16, -5)	,	S(10, 2)	,	S(13, 8)	,	S(31, -10)	,	S(13, -8)	,	S(10, 8)	,
		S(-50, 16)	,	S(20, -15)	,	S(0, -1)	,	S(22, -2)	,	S(30, -1)	,	S(-5, -9)	,	S(-5, -14)	,	S(48, -33)	,
		S(-4, -5)	,	S(-6, 8)	,	S(-4, -1)	,	S(21, 1)	,	S(23, -8)	,	S(24, -6)	,	S(1, -6)	,	S(-2, 10)	,
		S(-34, 4)	,	S(3, -9)	,	S(7, -17)	,	S(25, -16)	,	S(44, -29)	,	S(54, -20)	,	S(79, -20)	,	S(-40, 17)	,
		S(28, -29)	,	S(-3, -10)	,	S(18, -26)	,	S(48, -38)	,	S(6, -4)	,	S(74, -28)	,	S(-26, 1)	,	S(-95, 13)	,
		S(-34, -1)	,	S(-70, -17)	,	S(36, -10)	,	S(3, -37)	,	S(28, -33)	,	S(-94, 32)	,	S(-73, -56)	,	S(-64, -14)	,
	};

	const Score RookTable[64] = {
		S(-12, -24)	,	S(2, -20)	,	S(21, -23)	,	S(36, -32)	,	S(41, -40)	,	S(27, -34)	,	S(-40, -8)	,	S(-18, -38)	,
		S(-38, -14)	,	S(-5, -23)	,	S(-15, -10)	,	S(-1, -13)	,	S(23, -32)	,	S(33, -32)	,	S(-32, 1)	,	S(-77, 4)	,
		S(-45, -1)	,	S(-9, -14)	,	S(15, -36)	,	S(-14, -15)	,	S(20, -35)	,	S(18, -32)	,	S(14, -30)	,	S(-32, -23)	,
		S(-49, 9)	,	S(-16, -8)	,	S(-26, 8)	,	S(22, -23)	,	S(-5, -12)	,	S(-19, -5)	,	S(25, -23)	,	S(-24, -9)	,
		S(-49, 10)	,	S(23, -23)	,	S(9, 5)		,	S(51, -30)	,	S(44, -21)	,	S(13, 1)	,	S(-22, -8)	,	S(-15, -2)	,
		S(-22, 5)	,	S(-8, 10)	,	S(53, -21)	,	S(27, -1)	,	S(32, -21)	,	S(-43, 7)	,	S(1, -8)	,	S(38, -21)	,
		S(-6, 11)	,	S(15, 7)	,	S(30, 2)	,	S(56, -13)	,	S(91, -29)	,	S(40, -4)	,	S(52, -13)	,	S(-22, 9)	,
		S(27, -15)	,	S(48, -22)	,	S(0, -12)	,	S(26, -16)	,	S(56, -18)	,	S(49, -29)	,	S(48, -25)	,	S(-21, -8)
	};


	const Score QueenTable[64] = {
		S(-13, -7)	,	S(-3, -31)	,	S(-1, -3)	,	S(27, -57)	,	S(-3, -7)	,	S(-15, -45)	,	S(-33, -21)	,	S(-41, 11)	,
		S(-13, -49)	,	S(-17, 5)	,	S(19, -41)	,	S(17, -29)	,	S(27, -35)	,	S(23, -15)	,	S(-3, -33)	,	S(-3, 9)	,
		S(-27, 27)	,	S(17, -47)	,	S(-7, 13)	,	S(3, -1)	,	S(5, 19)	,	S(7, 31)	,	S(21, 33)	,	S(15, 25)	,
		S(-7, -19)	,	S(-27, 15)	,	S(-11, 23)	,	S(-13, 61)	,	S(-11, 53)	,	S(11, 23)	,	S(7, 39)	,	S(7, 33)	,
		S(-35, -1)	,	S(-35, 17)	,	S(-13, 21)	,	S(-9, 31)	,	S(21, 45)	,	S(15, 49)	,	S(37, 13)	,	S(7, 37)	,
		S(-23, -9)	,	S(-37, 25)	,	S(21, -7)	,	S(9, 51)	,	S(67, 43)	,	S(47, 59)	,	S(23, 69)	,	S(53, 19)	,
		S(-33, -15)	,	S(-63, 41)	,	S(15, 25)	,	S(-19, 57)	,	S(-5, 61)	,	S(63, 25)	,	S(51, -17)	,	S(33, 37)	,
		S(-29, -11)	,	S(-23, 47)	,	S(41, 7)	,	S(49, -3)	,	S(25, 41)	,	S(-7, 49)	,	S(-3, 41)	,	S(65, 9)
	};

	const Score KingTable[64] = {
		S(-4, -76)	,	S(41, -57)	,	S(21, -42)	,	S(-40, -39)	,	S(29, -55)	,	S(-21, -34)	,	S(46, -63)	, S(33, -96)	,
		S(-9, -55)	,	S(-3, -39)	,	S(-1, -18)	,	S(-47, -6)	,	S(-35, -4)	,	S(-23, -10)	,	S(13, -29)	, S(15, -49)	,
		S(-30, -38)	,	S(-16, -22)	,	S(-22, -6)	,	S(-40, 13)	,	S(-36, 13)	,	S(-32, 5)	,	S(-4, -14)	, S(-6, -42)	,
		S(-34, -29)	,	S(-2, -13)	,	S(-30, 15)	,	S(-38, 16)	,	S(-34, 18)	,	S(-28, 9)	,	S(-36, -5)	, S(-36, -33)	,
		S(16, -15)	,	S(-2, 12)	,	S(-10, 15)	,	S(-38, 28)	,	S(-40, 16)	,	S(-32, 23)	,	S(-22, 17)	, S(-42, -23)	,
		S(-78, 4)	,	S(-30, 5)	,	S(-30, 18)	,	S(-12, 5)	,	S(-22, 15)	,	S(-16, 31)	,	S(-34, 24)	, S(-24, -2)	,
		S(-38, -25)	,	S(-34, 11)	,	S(-70, 4)	,	S(-52, -4)	,	S(-74, -6)	,	S(-32, -8)	,	S(-80, 17)	, S(-60, -17)	,
		S(-84, -46)	,	S(-90, -39)	,	S(-56, -26)	,	S(-62, -53)	,	S(-84, -19)	,	S(-46, -6)	,	S(-36, -59)	, S(-56, -40)
	};


	// This table contains the penalties for advanced pawns in front of the king.
	const int castledPawnAdvancementMg[64] = {
		0	,	 0	,	0	,	 0	,	 0	,	 0	,	 0	,	 0	,
		0	,	 0	,	0	,	 0	,	 0	,	 0	,	 0	,	 0	,
		10	,	15	,	5	,	0	,	0	,	5	,	10	,	10	,
		15	,	25	,	10	,	5	,	5	,	10	,	25	,	15	,
		25	,	35	,	15	,	10	,	10	,	15	,	35	,	25	,
		35	,	45	,	25	,	15	,	15	,	25	,	45	,	35	,
		45	,	45	,	25	,	20	,	20	,	25	,	45	,	45	,
		45	,	45	,	25	,	25	,	25	,	25	,	45	,	45
	};




	// This is to make the psqt give the same values for black pieces. For example square a1 for white corresponds to square h8 for black.
	const int Mirror64[64] = {
		56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
		48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
		40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
		32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
		24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
		16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
		8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
		0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
	};




	/*
	
	Mobility bonuses
	
	*/

	const std::vector<Score> knightMobility = { {S(-69, -76), S(-36, -68), S(-17, -30), S(-2, -14), S(4, -6), S(11, 8), // Knight
		S(20, 9), S(33, 12), S(42, 5)} };
	
	const std::vector<Score> bishopMobility = { S(-46, -54), S(-21, -30), S(11, -13), S(28, 13), S(36, 28), S(50, 37),
		S(54, 51), S(59, 51), S(59, 62), S(66, 59), S(83, 57), S(102, 56),
		S(82, 69), S(91, 73) };


	const std::vector<Score> rookMobility = { S(-60,-82), S(-24,-15), S(0, 17) ,S(3, 43), S(4, 72), S(14,100), // Rook. TODO: Re-tune this
	  S(20,102), S(30,122), S(41,133), S(41 ,139), S(41,153), S(45,160),
	  S(57,165), S(58,170), S(67,175) };

	const std::vector<Score> queenMobility = {
		S(-38, -66), S(-25, -14), S(-1, -21), S(21, 6), S(31, 34), S(30, 39), S(36, 46), S(40, 46), S(46, 61), // Queen
		S(45, 70), S(52, 80), S(53, 92), S(54, 107), S(61, 107), S(69, 109),
		S(71, 118), S(66, 131), S(69, 137), S(67, 144), S(69, 156), S(97, 138),
		S(105, 144), S(96, 156), S(85, 172), S(111, 191),
		S(129, 170), S(115, 170), S(96, 202) };



	const std::vector<std::vector<Score>> mobilityBonus = {
		knightMobility,
		bishopMobility,
		rookMobility,
		queenMobility
	};


	// For the time being, these values are taken from https://www.chessprogramming.org/King_Safety until proper tuning is implemented.
	// Therefore, endgame and middlegame scores are also the same at the moment.
	Score safety_table[100] = {
		S(0, 0),		S(1, 1),		S(1, 1),		S(2, 2),		S(2, 2),		S(3, 3),		S(4, 4),		S(4, 4),		S(5, 5),		S(6, 6),
		S(9, 9),		S(12, 12),		S(15, 15),		S(18, 18),		S(21, 21),		S(24, 24),		S(27, 27),		S(30, 30),		S(33, 33),		S(36, 36),
		S(40, 40),		S(50, 50),		S(60, 60),		S(75, 75),		S(90, 90),		S(100, 100),	S(110, 110),	S(120, 120),	S(130, 130),	S(140, 140),
		S(150, 150),	S(160, 160),	S(170, 170),	S(180, 180),	S(190, 190),	S(200, 200),	S(210, 210),	S(220, 220),	S(230, 230),	S(240, 240),
		S(250, 250),	S(260, 260),	S(270, 270),	S(280, 280),	S(290, 290),	S(300, 300),	S(310, 310),	S(320, 320),	S(330, 330),	S(340, 340),
		S(350, 350),	S(360, 360),	S(370, 370),	S(380, 380),	S(390, 390),	S(400, 400),	S(410, 410),	S(420, 420),	S(430, 430),	S(440, 440),
		S(450, 450),	S(460, 460),	S(470, 470),	S(480, 480),	S(490, 490),	S(500, 500),	S(510, 510),	S(520, 520),	S(530, 530),	S(540, 540),
		S(550, 550),	S(560, 560),	S(570, 570),	S(580, 580),	S(590, 590),	S(600, 600),	S(610, 610),	S(620, 620),	S(630, 630),	S(640, 640),
		S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),
		S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650),	S(650, 650)
	};



	// This table has all the bonuses for passed pawns. It is made this way to hopefully give the evaluation some more accuracy instead of just scoring
	//	passers by rank.
	const Score passedPawnTable[64] = {
		S(34, 26)	, S(52, 11)	,	S(20, 5)	,	S(13, 50)	,	S(3, 27)	,	S(0, 66)	,	S(4, 66)	,	S(16, 4)	,
		S(0, 0)		, S(2, 0)	,	S(4, 1)		,	S(12, 19)	,	S(0, 23)	,	S(17, 1)	,	S(1, 1)		,	S(1, 2)		,
		S(0, 1)		, S(0, 6)	,	S(1, 0)		,	S(2, 3)		,	S(1, 3)		,	S(9, 0)		,	S(16, 2)	,	S(9, 0)		,
		S(3, 27)	, S(2, 19)	,	S(1, 21)	,	S(0, 18)	,	S(0, 7)		,	S(14, 13)	,	S(18, 15)	,	S(5, 23)	,
		S(6, 53)	, S(36, 35)	,	S(19, 35)	,	S(14, 18)	,	S(28, 35)	,	S(18, 31)	,	S(1, 49)	,	S(1, 51)	,
		S(40, 107)	, S(62, 73)	,	S(20, 59)	,	S(1, 51)	,	S(40, 45)	,	S(42, 49)	,	S(12, 71)	,	S(5, 61)	,
		S(84, 137)	, S(48, 163),	S(26, 145)	,	S(66, 91)	,	S(116, 107)	,	S(54, 133)	,	S(16, 129)	,	S(38, 153)	,
		S(63, 74)	, S(16, 13)	,	S(47, 13)	,	S(4, 19)	,	S(11, 11)	,	S(44, 32)	,	S(26, 30)	,	S(11, 17)
	};



	// Penalties for the distance between the king and a pawn on the flank.
	const Score king_pawn_distance_penalty[8] = {
		S(0, 0),
		S(-18, -38),
		S(-8, -32),
		S(-5, -19),
		S(-8, -8),
		S(-12, 36),
		S(81, -40),
		S(-132, 27),
	};



	// This table contains the penalties for storming pawns on the king flank
	const Score pawnStorm[64] = {
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)			,	S(0, 0)		,	S(0, 0)		,	S(0, 0),
		S(57, -194)	,	S(223, -8)	,	S(139, 76)	,	S(33, 128)	,	S(-159, -232)	,	S(-3, 66)	,	S(133, 146)	,	S(27, 78),
		S(13, -136)	,	S(-275, 0)	,	S(171, -4)	,	S(123, 60)	,	S(-79, -14)		,	S(147, 86)	,	S(73, 100)	,	S(-1, -10),
		S(10, 178)	,	S(0, -8)	,	S(188, 56)	,	S(140, 64)	,	S(80, -54)		,	S(30, -10)	,	S(60, -20)	,	S(-8, 14),
		S(-11, 16)	,	S(75, -80)	,	S(45, 78)	,	S(-77, 60)	,	S(-87, -30)		,	S(-1, 14)	,	S(67, -26)	,	S(-3, 16),
		S(-101, 108),	S(11, -136)	,	S(-45, 96)	,	S(-291, 58)	,	S(-3, 20)		,	S(1, 18)	,	S(37, -18)	,	S(-17, 18),
		S(-60, 64)	,	S(18, -24)	,	S(74, -30)	,	S(80, -24)	,	S(6, 38)		,	S(2, 48)	,	S(44, 4)	,	S(-14, -34),
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)			,	S(0, 0)		,	S(0, 0)		,	S(0, 0)
	};


	// Penalties for open files near the king. Indexed by file number.
	const Score open_kingfile_penalty[8] = {
		S(-26, 50),
		S(143, -118),
		S(108, -38),
		S(31, 30),
		S(77, -58),
		S(-6, 50),
		S(131, -78),
		S(124, -98)
	};


	
	// Penalties for semi-open files near the king. Again, indexed by file number
	const Score semiopen_kingfile_penalty[8] = {
		S(-23, 58),
		S(107, -154),
		S(80, -98),
		S(-9, 60),
		S(-7, 10),
		S(0, 32),
		S(47, -12),
		S(-37, 62)
	};



	const Score space_bonus[32] = {
		S(-40, 2)	,	S(-21, 19)	,	S(66, 9)	,	S(41, 14)	,	S(36, 12)	,	S(35, 10)	,	S(23, 15)	, S(18, 17),
		S(5, 26)	,	S(2, 24)	,	S(-1, 27)	,	S(0, 23)	,	S(1, 30)	,	S(12, 26)	,	S(13, 23)	, S(20, 13),
		S(31, 24)	,	S(40, 10)	,	S(39, 41)	,	S(74, -43)	,	S(69, 36)	,	S(68, 24)	,	S(99, 58)	, S(144, 61),
		S(90, 7)	,	S(201, 36)	,	S(90, 4)	,	S(101, 61)	,	S(148, 73)	,	S(219, 38)	,	S(50, 4)	, S(187, 69),
	};

#undef S

	namespace {

		int compute_md(int sq1, int sq2) {
			int file1, file2, rank1, rank2;

			int rankDistance, fileDistance;

			file1 = sq1 % 8; rank1 = sq1 / 8;
			file2 = sq2 % 8; rank2 = sq2 / 8;

			rankDistance = abs(rank2 - rank1);
			fileDistance = abs(file2 - file1);

			return rankDistance + fileDistance;
		}

	}


	int ManhattanDistance[64][64] = {};

	void initManhattanDistance() {

		for (int sq1 = 0; sq1 < 64; sq1++) {

			for (int sq2 = 0; sq2 < 64; sq2++) {
				ManhattanDistance[sq1][sq2] = compute_md(sq1, sq2);
			}

		}
	}

	void INIT() {
		initManhattanDistance();
	}
}