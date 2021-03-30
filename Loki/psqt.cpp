#include "psqt.h"




namespace PSQT {
	// Pawns are incentivized to develop in the center, and to not move in front of a castled king.
	// Pawns in front of the king will be evaluated in the piece-evaluations.


	const int PawnTableMg[64] = {
		0	,	4	,	-1	,	11	,	-3	,	2	,	5	,	-2	,
		10	,	9	,	4	,	-16	,	-15	,	13	,	18	,	22	,
		3	,	2	,	3	,	-3	,	-7	,	1	,	0	,	4	,
		4	,	7	,	14	,	18	,	24	,	0	,	4	,	-4	,
		-1	,	0	,	5	,	10	,	15	,	0	,	6	,	3	,
		0	,	7	,	3	,	15	,	5	,	11	,	5	,	5	,
		18	,	15	,	24	,	27	,	23	,	23	,	12	,	10	,
		1	,	8	,	5	,	9	,	0	,	6	,	1	,	11
	};

	// In the endgame, we generally want the pawns to advance. The engine is incentivized to push pawns on the sides, but this difference
	// isn't that big

	const int PawnTableEg[64] = {
		1	,	23	,	5	,	10	,	4	,	13	,	17	,	7	,
		5	,	30	,	20	,	31	,	25	,	49	,	27	,	20	,
		5	,	32	,	15	,	41	,	36	,	31	,	19	,	27	,
		33	,	41	,	35	,	28	,	46	,	29	,	41	,	32	,
		54	,	47	,	62	,	47	,	55	,	54	,	89	,	72	,
		80	,	63	,	57	,	81	,	69	,	81	,	89	,	108	,
		177	,	174	,	162	,	149	,	156	,	152	,	187	,	178	,
		0	,	20	,	-6	,	24	,	1	,	8	,	-3	,	3
	};



	// Knights should be developed to the center, and stay away from the edges.

	const int KnightTableMg[64] = {
		12	,	-11	,	12	,	9	,	26	,	-4	,	-1	,	8	,
		1	,	-4	,	5	,	19	,	24	,	18	,	10	,	11	,
		-2	,	14	,	22	,	-2	,	21	,	36	,	19	,	1	,
		-2	,	7	,	24	,	26	,	31	,	17	,	0	,	-8	,
		16	,	14	,	28	,	27	,	24	,	20	,	35	,	-11	,
		13	,	10	,	30	,	22	,	6	,	25	,	20	,	7	,
		-12	,	-7	,	23	,	18	,	7	,	1	,	10	,	8	,
		-6	,	3	,	-6	,	8	,	22	,	-11	,	-7	,	18
	};

	const int KnightTableEg[64] = {
		-5	,	-20	,	17	,	-1	,	0	,	-6	,	-17	,	16	,
		-1	,	-1	,	26	,	23	,	11	,	7	,	3	,	3	,
		18	,	-10	,	21	,	3	,	25	,	23	,	6	,	9	,
		-7	,	15	,	25	,	17	,	21	,	21	,	10	,	14	,
		8	,	23	,	11	,	21	,	25	,	27	,	9	,	4	,
		16	,	2	,	28	,	-3	,	22	,	-9	,	12	,	-3	,
		7	,	-2	,	16	,	2	,	3	,	4	,	2	,	-16	,
		5	,	6	,	8	,	-11	,	-17	,	2	,	17	,	-12	,
	};



	// Bishops should also be developed in the center or to b2 og g2
	const int BishopTableMg[64] = {
		-9	,	-3	,	-10	,	-3	,	-2	,	-15	,	-3	,	4	,
		-7	,	19	,	1	,	7	,	13	,	2	,	26	,	-5	,
		-9	,	2	,	1	,	-3	,	1	,	18	,	7	,	4	,
		-5	,	9	,	8	,	13	,	13	,	10	,	0	,	8	,
		-1	,	0	,	3	,	7	,	12	,	2	,	7	,	-3	,
		-5	,	5	,	12	,	4	,	0	,	3	,	6	,	-1	,
		-5	,	2	,	-2	,	3	,	5	,	-2	,	5	,	-7	,
		-6	,	-6	,	-3	,	11	,	-3	,	-2	,	11	,	-4
	};

	const int BishopTableEg[64] = {
		3	,	-6	,	-13	,	-9	,	-3	,	12	,	5	,	-2	,
		4	,	6	,	14	,	7	,	11	,	-1	,	5	,	-14	,
		-2	,	13	,	14	,	-1	,	9	,	2	,	15	,	-4	,
		-5	,	11	,	13	,	12	,	13	,	11	,	9	,	2	,
		11	,	13	,	10	,	23	,	9	,	11	,	2	,	-5	,
		0	,	-3	,	11	,	0	,	-1	,	7	,	11	,	-5	,
		0	,	3	,	4	,	-4	,	-1	,	4	,	1	,	-5	,
		2	,	-9	,	1	,	2	,	-1	,	0	,	4	,	5
	};


	// Rooks are incentivized to move to the center and 7th rank. They will later gain value by occupying open files.

	const int RookTableMg[64] = {
		3	,	11	,	18	,	20	,	15	,	23	,	-9	,	-15	,
		0	,	7	,	19	,	8	,	-4	,	7	,	10	,	-3	,
		-1	,	-2	,	11	,	14	,	-5	,	-8	,	8	,	-9	,
		1	,	14	,	13	,	7	,	13	,	17	,	14	,	4	,
		-3	,	12	,	9	,	3	,	12	,	2	,	10	,	13	,
		3	,	13	,	-5	,	14	,	7	,	15	,	6	,	4	,
		31	,	25	,	35	,	36	,	31	,	40	,	21	,	32	,
		29	,	20	,	24	,	17	,	4	,	16	,	33	,	11
	};

	const int RookTableEg[64] = {
		7	,	18	,	4	,	12	,	12	,	7	,	2	,	-5	,
		-4	,	26	,	-1	,	8	,	5	,	13	,	25	,	13	,
		-4	,	2	,	1	,	16	,	5	,	5	,	-7	,	3	,
		7	,	20	,	-1	,	19	,	12	,	0	,	-3	,	-3	,
		-2	,	9	,	3	,	-1	,	3	,	15	,	10	,	5	,
		-13	,	7	,	-4	,	10	,	5	,	16	,	4	,	8	,
		18	,	28	,	25	,	27	,	26	,	34	,	31	,	29	,
		1	,	10	,	17	,	26	,	29	,	21	,	7	,	16
	};

	// Queen should not occupy the eight'th rank in middlegame, because we want the rooks connected. Small centralization bonus

	const int QueenTableMg[64] = {
		53	,	27	,	31	,	48	,	8	,	37	,	9	,	26	,
		25	,	15	,	56	,	48	,	65	,	46	,	49	,	109	,
		9	,	35	,	44	,	31	,	22	,	42	,	60	,	7	,
		21	,	41	,	53	,	21	,	19	,	66	,	32	,	39	,
		19	,	37	,	69	,	38	,	58	,	69	,	14	,	37	,
		4	,	34	,	-4	,	82	,	55	,	55	,	30	,	44	,
		24	,	-21	,	55	,	19	,	16	,	52	,	2	,	15	,
		7	,	42	,	29	,	16	,	33	,	36	,	16	,	69
	};


	const int QueenTableEg[64] = {
		16	,	26	,	31	,	48	,	20	,	21	,	22	,	29	,
		3	,	36	,	39	,	7	,	33	,	12	,	29	,	32	,
		26	,	64	,	3	,	41	,	44	,	65	,	5	,	79	,
		39	,	24	,	28	,	18	,	64	,	44	,	11	,	34	,
		21	,	31	,	17	,	3	,	60	,	19	,	19	,	57	,
		7	,	27	,	6	,	5	,	46	,	19	,	38	,	20	,
		-1	,	-15	,	22	,	43	,	31	,	30	,	67	,	29	,
		5	,	27	,	31	,	35	,	23	,	24	,	65	,	20
	};



	const int KingTableMg[64] = {
		30	,	31	,	18	,	-8	,	4	,	-7	,	40	,	14	,
		4	,	25	,	10	,	-11	,	-3	,	-6	,	15	,	9	,
		9	,	-18	,	-11	,	-8	,	-30	,	-14	,	3	,	-6	,
		1	,	-26	,	-11	,	-23	,	-28	,	-13	,	-16	,	-2	,
		-23	,	-21	,	-32	,	-52	,	-41	,	-31	,	-40	,	-37	,
		-15	,	-17	,	-14	,	-49	,	-51	,	-42	,	-44	,	-37	,
		-40	,	-27	,	-50	,	-37	,	-51	,	-37	,	-33	,	-40	,
		-12	,	-43	,	-33	,	-33	,	-45	,	-29	,	-30	,	-29
	};


	const int KingTableEg[64] = {
		-41	,	-7	,	-14	,	-16	,	-16	,	-29	,	-1	,	-51	,
		-21	,	-4	,	19	,	20	,	30	,	18	,	10	,	-1	,
		-4	,	7	,	51	,	63	,	37	,	28	,	15	,	-7	,
		-9	,	10	,	47	,	71	,	75	,	44	,	16	,	-13	,
		-23	,	7	,	52	,	60	,	76	,	40	,	20	,	-6	,
		-11	,	8	,	37	,	64	,	52	,	41	,	13	,	-12	,
		-23	,	12	,	7	,	19	,	12	,	14	,	15	,	-15	,
		-22	,	-30	,	-12	,	11	,	-13	,	-16	,	-23	,	-24
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
#define S(mg, eg) Score(mg, eg)
	const std::vector<Score> knightMobility = { S(-62,-79), S(-53,-57), S(-12,-31), S(-3,-17), S(3,  7), S(12, 13), // Knight
	  S(21, 16), S(28, 21), S(37, 26) };
	const std::vector<Score> bishopMobility = { S(-47,-59), S(-20,-25), S(14, -8), S(29, 12), S(39, 21), S(53, 40), // Bishop
	  S(53, 56), S(60, 58), S(62, 65), S(69, 72), S(78, 78), S(83, 87),
	  S(91, 88), S(96, 98) };

	const std::vector<Score> rookMobility = { S(-60,-82), S(-24,-15), S(0, 17) ,S(3, 43), S(4, 72), S(14,100), // Rook
	  S(20,102), S(30,122), S(41,133), S(41 ,139), S(41,153), S(45,160),
	  S(57,165), S(58,170), S(67,175) };

	const std::vector<Score> queenMobility = { S(-29,-49), S(-16,-29), S(-8, -8), S(-8, 17), S(18, 39), S(25, 54), // Queen
	  S(23, 59), S(37, 73), S(41, 76), S(54, 95), S(65, 95) ,S(68,101),
	  S(69,124), S(70,128), S(70,132), S(70,133) ,S(71,136), S(72,140),
	  S(74,147), S(76,149), S(90,153), S(104,169), S(105,171), S(106,171),
	  S(112,178), S(114,185), S(114,187), S(119,221) };

	const std::vector<std::vector<Score>> mobilityBonus = {
		knightMobility,
		bishopMobility,
		rookMobility,
		queenMobility
	};


	// For the time being, these values are taken from https://www.chessprogramming.org/King_Safety until proper tuning is implemented.
	// Therefore, endgame and middlegame scores are also the same at the moment.
	const Score safety_table[100] = {
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
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,
		S(5, 10)	,	S(5, 10)	,	S(5, 10)	,	S(5, 10)	,	S(5, 10)	,	S(5, 10)	,	S(5, 10)	,	S(5, 10)	,
		S(10, 20)	,	S(10, 20)	,	S(10, 20)	,	S(10, 20)	,	S(10, 20)	,	S(10, 20)	,	S(10, 20)	,	S(10, 20)	,
		S(20, 40)	,	S(20, 40)	,	S(20, 40)	,	S(20, 40)	,	S(20, 40)	,	S(20, 40)	,	S(20, 40)	,	S(20, 40)	,
		S(30, 60)	,	S(30, 60)	,	S(30, 60)	,	S(30, 60)	,	S(30, 60)	,	S(30, 60)	,	S(30, 60)	,	S(30, 60)	,
		S(40, 80)	,	S(40, 80)	,	S(40, 80)	,	S(40, 80)	,	S(40, 80)	,	S(40, 80)	,	S(40, 80)	,	S(40, 80)	,
		S(50, 100)	,	S(50, 100)	,	S(50, 100)	,	S(50, 100)	,	S(50, 100)	,	S(50, 100)	,	S(50, 100)	,	S(50, 100)	,
		S(0, 0) 	,	S(0, 0) 	,	S(0, 0) 	,	S(0, 0) 	,	S(0, 0) 	,	S(0, 0) 	,	S(0, 0) 	,	S(0, 0)
	};


	const Score king_pawn_distance_penalty[8] = {
		S(0, 0),
		S(0, 0),
		S(0, 0),
		S(5, 5),
		S(10, 10),
		S(20, 20),
		S(25, 30),
		S(30, 45)
	};


	// This table contains the penalties for storming pawns on the king flank
	const Score pawnStorm[64] = {
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,
		S(15, 40)	,	S(15, 40)	,	S(15, 40)	,	S(15, 40)	,	S(15, 40)	,	S(15, 40)	,	S(15, 40)	,	S(15, 40)	,
		S(13, 36)	,	S(13, 36)	,	S(13, 36)	,	S(13, 36)	,	S(13, 36)	,	S(13, 36)	,	S(13, 36)	,	S(13, 36)	,
		S(12, 30)	,	S(12, 30)	,	S(12, 30)	,	S(12, 30)	,	S(12, 30)	,	S(12, 30)	,	S(12, 30)	,	S(12, 30)	,
		S(9, 18)	,	S(9, 18)	,	S(9, 18)	,	S(9, 18)	,	S(9, 18)	,	S(9, 18)	,	S(9, 18)	,	S(9, 18)	,
		S(3, 6)		,	S(3, 6)		,	S(3, 6)		,	S(3, 6)		,	S(3, 6)		,	S(3, 6)		,	S(3, 6)		,	S(3, 6)		,
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,
		S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)		,	S(0, 0)
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