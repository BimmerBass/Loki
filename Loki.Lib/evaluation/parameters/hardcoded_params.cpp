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
#include "loki.pch.hpp"

#undef S
#define S(v) static_cast<eValue>( (v) )

namespace loki::evaluation
{

	void hardcoded_params::initialize()
	{
		// Init material
		pawn = static_cast<eValue>(512);
		knight = static_cast<eValue>(1536);
		bishop = static_cast<eValue>(1536);
		rook = static_cast<eValue>(2560);
		queen = static_cast<eValue>(4608);

		// Init PSQT.
		piece_square_tables = PieceSquareTables(
			{
				S(0)	,	S(0)	,	S(0)	,	S(0)	,	S(0)	,	S(0)	,	S(0)	, S(0)		,
				S(1)	,	S(78)	,	S(-12)	,	S(24)	,	S(4)	,	S(114)	,	S(160)	, S(9)		,
				S(-11)	,	S(47)	,	S(24)	,	S(9)	,	S(55)	,	S(34)	,	S(142)	, S(9)		,
				S(-9)	,	S(29)	,	S(-27)	,	S(34)	,	S(16)	,	S(21)	,	S(21)	, S(-32)	,
				S(65)	,	S(72)	,	S(50)	,	S(29)	,	S(44)	,	S(65)	,	S(57)	, S(-4)		,
				S(198)	,	S(149)	,	S(160)	,	S(144)	,	S(111)	,	S(188)	,	S(150)	, S(252)	,
				S(367)	,	S(254)	,	S(252)	,	S(239)	,	S(206)	,	S(252)	,	S(242)	, S(208)	,
				S(0)	,	S(0)	,	S(0)	,	S(0)	,	S(0)	,	S(0)	,	S(0)	, S(0)
			},
			{ // Knight
				S(-188)	,	S(-48)	,	S(-139)	,	S(-111)	,	S(-4)	,	S(-68)	,	S(-76)	,	S(-131)	,
				S(-139)	,	S(29)	,	S(-42)	,	S(37)	,	S(40)	,	S(55)	,	S(-6)	,	S(-47)	,
				S(-59)	,	S(-17)	,	S(11)	,	S(37)	,	S(65)	,	S(52)	,	S(70)	,	S(-46)	,
				S(-6)	,	S(80)	,	S(47)	,	S(95)	,	S(106)	,	S(109)	,	S(37)	,	S(14)	,
				S(-1)	,	S(55)	,	S(73)	,	S(195)	,	S(152)	,	S(229)	,	S(90)	,	S(106)	,
				S(-94)	,	S(78)	,	S(181)	,	S(129)	,	S(242)	,	S(108)	,	S(150)	,	S(5)	,
				S(-68)	,	S(-47)	,	S(129)	,	S(55)	,	S(6)	,	S(73)	,	S(-62)	,	S(-50)	,
				S(-217)	,	S(15)	,	S(-147)	,	S(-101)	,	S(-6)	,	S(-116)	,	S(-92)	,	S(-106)
			},
			{ // Bishop
				S(-14)  ,	S(-11)  ,   S(-20)  ,   S(-1)   ,   S(25)   ,   S(-28)  ,   S(-64)  ,	S(-7)    ,
				S(128)  ,	S(25)   ,   S(28)   ,   S(-1)   ,   S(24)   ,   S(15)   ,   S(79)   ,	S(9)     ,
				S(-17)  ,	S(33)   ,   S(34)   ,   S(28)   ,   S(43)   ,   S(67)   ,   S(23)   ,	S(35)    ,
				S(-107) ,	S(32)   ,   S(-1)   ,   S(54)   ,   S(76)   ,   S(-24)  ,   S(-31)  ,	S(81)    ,
				S(-16)  ,	S(-5)   ,   S(-11)  ,   S(55)   ,   S(49)   ,   S(54)   ,   S(-5)   ,	S(7)     ,
				S(-82)  ,	S(-4)   ,   S(-3)   ,   S(43)   ,   S(75)   ,   S(112)  ,   S(176)  ,	S(-81)   ,
				S(34)   ,	S(-20)  ,   S(13)   ,   S(74)   ,   S(10)   ,   S(154)  ,   S(-65)  ,	S(-226)  ,
				S(-88)  ,	S(-200) ,   S(79)   ,   S(-39)  ,   S(29)   ,   S(-199) ,   S(-258) ,	S(-182)
			},
			{ // Rook
				S(-61)  ,   S(-20)  ,   S(24)   ,   S(51)   ,   S(54)   ,   S(25)   ,   S(-112) ,  S(-94)   ,
				S(-115) ,   S(-42)  ,   S(-51)  ,   S(-19)  ,   S(18)   ,   S(43)   ,   S(-81)  ,  S(-192)  ,
				S(-116) ,   S(-41)  ,   S(-7)   ,   S(-55)  ,   S(6)    ,   S(5)    ,   S(-2)   ,  S(-111)  ,
				S(-114) ,   S(-51)  ,   S(-56)  ,   S(27)   ,   S(-28)  ,   S(-55)  ,   S(34)   ,  S(-73)   ,
				S(-113) ,   S(29)   ,   S(29)   ,   S(92)   ,   S(85)   ,   S(34)   ,   S(-66)  ,  S(-41)   ,
				S(-50)  ,   S(-8)   ,   S(108)  ,   S(68)   ,   S(55)   ,   S(-101) ,   S(-7)   ,  S(70)    ,
				S(-1)   ,   S(47)   ,   S(79)   ,   S(127)  ,   S(196)  ,   S(97)   ,   S(116)  ,  S(-45)   ,
				S(50)   ,   S(95)   ,   S(-15)  ,   S(46)   ,   S(120)  ,   S(88)   ,   S(91)   ,  S(-64)
			},
			{ // Queen
				S(-42)  ,   S(-47)  ,   S(-6)   ,   S(-4)   ,   S(-16)  ,   S(-96)  ,   S(-111) ,  S(-91)   ,
				S(-96)  ,   S(-37)  ,   S(-4)   ,   S(6)    ,   S(24)   ,   S(40)   ,   S(-49)  ,  S(4)     ,
				S(-34)  ,   S(-16)  ,   S(-1)   ,   S(6)    ,   S(37)   ,   S(57)   ,   S(96)   ,  S(70)    ,
				S(-42)  ,   S(-50)  ,   S(1)    ,   S(44)   ,   S(39)   ,   S(57)   ,   S(68)   ,  S(60)    ,
				S(-90)  ,   S(-68)  ,   S(-6)   ,   S(16)   ,   S(111)  ,   S(101)  ,   S(111)  ,  S(65)    ,
				S(-70)  ,   S(-62)  ,   S(45)   ,   S(88)   ,   S(226)  ,   S(196)  ,   S(147)  ,  S(159)   ,
				S(-103) ,   S(-109) ,   S(70)   ,   S(24)   ,   S(65)   ,   S(193)  ,   S(109)  ,  S(131)   ,
				S(-88)  ,   S(1)    ,   S(114)  ,   S(122)  ,   S(116)  ,   S(44)   ,   S(45)   ,  S(178)
			},
			{ // King
				S(-107) ,   S(32)   ,   S(0)    ,   S(-152) ,   S(3)    ,   S(-97)  ,   S(37)   ,  S(-38)   ,
				S(-93)  ,   S(-57)  ,   S(-25)  ,   S(-128) ,   S(-94)  ,   S(-71)  ,   S(-3)   ,  S(-24)   ,
				S(-125) ,   S(-69)  ,   S(-64)  ,   S(-86)  ,   S(-75)  ,   S(-75)  ,   S(-28)  ,  S(-69)   ,
				S(-124) ,   S(-21)  ,   S(-58)  ,   S(-77)  ,   S(-64)  ,   S(-60)  ,   S(-98)  ,  S(-134)  ,
				S(22)   ,   S(10)   ,   S(-6)   ,   S(-62)  ,   S(-82)  ,   S(-52)  ,   S(-35)  ,  S(-137)  ,
				S(-194) ,   S(-70)  ,   S(-54)  ,   S(-24)  ,   S(-37)  ,   S(-1)   ,   S(-56)  ,  S(-64)   ,
				S(-129) ,   S(-73)  ,   S(-174) ,   S(-138) ,   S(-197) ,   S(-92)  ,   S(-183) ,  S(-175)  ,
				S(-274) ,   S(-280) ,   S(-176) ,   S(-226) ,   S(-239) ,   S(-125) ,   S(-167) ,  S(-194)
			}
		);

		// Init Misc.
		tempo = static_cast<eValue>(26);
	}

}

#undef S