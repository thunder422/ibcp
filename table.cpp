// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: table.cpp - contains operator/command/function table
//	Copyright (C) 2010  Thunder422
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	For a copy of the GNU General Public License,
//	see <http://www.gnu.org/licenses/>.
// 
//
//  Change History:
//
//  2010-02-18  initial release
//
//  2010-03-01  renamed ImCommand_TokenType to ImmCmd_TokenType
//
//  2010-03-06  changed None_DataType to String_DataType for the internal string
//              functions
//
//  2010-03-10  corrected multiple entry for operators <, <=, and <>
//              (was OneChar, changed to TwoChar)
//
//  2010-03-11  split IntFunc_TokenType into IntFuncN_TokenType and
//              IntFuncP_TokenType; changed tables entries accordingly
//
//  2010-03-17  added data for new unary_code field with all set to Null_Code
//                except for Sub_Code and Not_Code
//              added new table entries for Null_Code, Neg_Code and EOL_Code
//
//  2010-03-20  added precedence values to the table entries
//
//  2010-03-21  for Neg_Code temporarily replaced "-" with "Neq" for testing
//              and changed unary_code from Null_Code to Neq_Code
//
//  2010-03-25  added unary code for (, changed precedence values for ( & )
//
//  2010-04-02  changed precedence value in comma entry
//              corrected name for VAL( from "Val(" to "VAL("
//              for operators, name is actual (recreator) output string,
//                name2 is debug output string (only affects Neg for now)
//
//  2010-04-04  added values for number of arguments variable
//              added entry for Asc2_Code
//              replaced Mid_Code entry Mid2_Code and Mid3_Code entries
//              replaced Instr_Code entry Instr2_Code and Instr3_Code entries
//              added Multiple_Flag for Asc_Code, Mid2_Code and Instr2_Code
//              implemented new number of arguments search function
//
//  2010-04-11  added table entries for assignment operators
//              set correct precedence for internal functions
//              removed unnecessary 0 initializers from entries
//
//  2010-04-24  added values for the number of operands to the operator entries
//              added values for new operand_datatype[] and assoc_code[] arrays
//              corrected data type for many operators and internal functions
//              added entries for the associated codes
//              moved the Null_Code to the end of the table
//              added entries for FRAC, CDBL, CvtDbl and CvtDbl
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "ibcp.h"


static TableEntry table_entries[] = {
	//******************************
	//   IMMEDIATE COMMANDS FIRST
	//******************************
	// (these will go away once gui interface is implemented)
	{
		List_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"L", NULL, Blank_Flag | Line_Flag | Range_Flag
	},
	{
		Edit_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"E", NULL, Blank_Flag | Line_Flag
	},
	{
		Delete_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"D", NULL, Line_Flag | Range_Flag
	},
	{
		Run_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"R", NULL, Blank_Flag
	},
	{
		Renum_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"R", NULL, Range_Flag | RangeIncr_Flag
	},
	{
		Save_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"S", NULL, Blank_Flag | String_Flag
	},
	{
		Load_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"L", NULL, String_Flag
	},
	{
		New_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"N", NULL, Blank_Flag
	},
	{
		Auto_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"A", NULL, Blank_Flag | Line_Flag | LineIncr_Flag
	},
	{
		Cont_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"C", NULL, Blank_Flag
	},
	{
		Quit_Code, ImmCmd_TokenType, None_DataType, OneWord_Multiple,
		"Q", NULL, Blank_Flag
	},
	// end of immediate commands marked by NULL name (next entry)
	//***********************
	//   BEGIN PLAIN WORDS
	//***********************
	{
		BegPlainWord_Code, Error_TokenType
	},
	//--------------
	//   Commands
	//--------------
	{
		Let_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"LET", NULL
	},
	{
		Print_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"PRINT", NULL
	},
	{
		Input_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"INPUT", NULL
	},
	{
		Dim_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"DIM", NULL
	},
	{
		Def_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"DEF", NULL
	},
	{
		Rem_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"REM", NULL
	},
	{
		If_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"IF", NULL
	},
	{
		Then_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"THEN", NULL
	},
	{
		Else_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"ELSE", NULL
	},
	{
		End_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"END", NULL
	},
	{
		EndIf_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"END", "IF"
	},
	{
		For_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"FOR", NULL
	},
	{
		To_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"TO", NULL
	},
	{
		Step_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"STEP", NULL
	},
	{
		Next_Code, Command_TokenType, None_DataType, OneWord_Multiple,
		"NEXT", NULL
	},
	{
		Do_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"Do", NULL
	},
	{
		DoWhile_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"DO", "WHILE"
	},
	{
		DoUntil_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"DO", "UNTIL"
	},
	{
		While_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"WHILE", NULL
	},
	{
		Until_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"UNTIL", NULL
	},
	{
		Loop_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", NULL
	},
	{
		LoopWhile_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", "WHILE"
	},
	{
		LoopUntil_Code, Command_TokenType, None_DataType, TwoWord_Multiple,
		"LOOP", "UNTIL"
	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{
		Rnd_Code, IntFuncN_TokenType, Double_DataType, OneWord_Multiple,
		"RND", NULL, Null_Flag, Null_Code, 2, 0
	},
	//--------------------
	//   Word Operators
	//--------------------
	{
		Mod_Code, Operator_TokenType, Double_DataType, OneWord_Multiple,
		"MOD", NULL, Null_Flag, Null_Code, 42,
		2, {Double_DataType, Double_DataType}, {ModInt_Code}
	},
	{
		And_Code, Operator_TokenType, Integer_DataType, OneWord_Multiple,
		"AND", NULL, Null_Flag, Null_Code, 18,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		Or_Code, Operator_TokenType, Integer_DataType, OneWord_Multiple,
		"OR", NULL, Null_Flag, Null_Code, 14,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		Not_Code, Operator_TokenType, Integer_DataType, OneWord_Multiple,
		"NOT", NULL, Null_Flag, Not_Code, 20,
		1, {Integer_DataType}
	},
	{
		Eqv_Code, Operator_TokenType, Integer_DataType, OneWord_Multiple,
		"EQV", NULL, Null_Flag, Null_Code, 12,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		Imp_Code, Operator_TokenType, Integer_DataType, OneWord_Multiple,
		"IMP", NULL, Null_Flag, Null_Code, 10,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		Xor_Code, Operator_TokenType, Integer_DataType, OneWord_Multiple,
		"XOR", NULL, Null_Flag, Null_Code, 16,
		2, {Integer_DataType, Integer_DataType}
	},
	//*********************
	//   END PLAIN WORDS
	//*********************
	{
		EndPlainWord_Code, Error_TokenType
	},
	//*****************************
	//   BEGIN PARENTHESES WORDS
	//*****************************
	{
		BegParenWord_Code, Error_TokenType
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{
		Abs_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"ABS(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}, {AbsInt_Code}
	},
	{
		Fix_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"FIX(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Frac_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"FRAC(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Int_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"INT(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}

	},
	{
		RndArg_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"RND(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}, {RndArgInt_Code}
	},
	{
		Sgn_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"SGN(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}, {SgnInt_Code}
	},
	{
		Cint_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"CINT(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Cdbl_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"CDBL(", NULL, Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		Sqr_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"SQR(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Atn_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"ATN(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Cos_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"COS(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Sin_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"SIN(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Tan_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"TAN(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Exp_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"EXP(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Log_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"LOG(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		Tab_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"TAB(", NULL, Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		Spc_Code, IntFuncP_TokenType, None_DataType, OneWord_Multiple,
		"SPC(", NULL, Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		Asc_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"ASC(", NULL, Multiple_Flag, Null_Code, 2,
		1, {String_DataType}
	},
	// 2010-04-04: added entry for 2 argument ASC
	{
		Asc2_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"ASC(", "ASC2(", Null_Flag, Null_Code, 2,
		2, {String_DataType, Integer_DataType}
	},
	{
		Chr_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"CHR$(", NULL, Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	// 2010-04-04: replaced INSTR entry with INSTR2 and INSTR3 entries
	{
		Instr2_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"INSTR(", "INSTR2(", Multiple_Flag, Null_Code, 2,
		2, {String_DataType, String_DataType}
	},
	{
		Instr3_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"INSTR(", "INSTR3(", Null_Flag, Null_Code, 2,
		3, {String_DataType, String_DataType, Integer_DataType}
	},
	{
		Left_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"LEFT$(", NULL, Null_Flag, Null_Code, 2,
		2, {String_DataType, Integer_DataType}
	},
	{
		Len_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"LEN(", NULL, Null_Flag, Null_Code, 2,
		1, {String_DataType}
	},
	// 2010-04-04: replaced MID entry with MID2 and MID3 entries
	{
		Mid2_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"MID$(", "MID2$(", Multiple_Flag, Null_Code, 2,
		2, {String_DataType, Integer_DataType}
	},
	{
		Mid3_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"MID$(", "MID3$(", Null_Flag, Null_Code, 2,
		3, {String_DataType, Integer_DataType, Integer_DataType}
	},
	{
		Repeat_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"REPEAT$(", NULL, Null_Flag, Null_Code, 2,
		2, {String_DataType, Integer_DataType}
	},
	{
		Right_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"RIGHT$(", NULL, Null_Flag, Null_Code, 2,
		2, {String_DataType, Integer_DataType}
	},
	{
		Space_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"SPACE$(", NULL, Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		Str_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"STR$(", NULL, Null_Flag, Null_Code, 2,
		1, {Double_DataType}, {StrInt_Code}
	},
	{
		// 2010-04-02: changed name to all upper case
		Val_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"VAL(", NULL, Null_Flag, Null_Code, 2,
		1, {String_DataType}
	},
	//***************************
	//   END PARENTHESES WORDS
	//***************************
	{
		EndParenWord_Code, Error_TokenType
	},
	//***************************
	//   BEGIN DATA TYPE WORDS
	//***************************
	{
		BegDataTypeWord_Code, Error_TokenType
	},
	// Currently None
	
	//*************************
	//   END DATA TYPE WORDS
	//*************************
	{
		EndDataTypeWord_Code, Error_TokenType
	},
	//*******************
	//   BEGIN SYMBOLS
	//*******************
	{
		BegSymbol_Code, Error_TokenType
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{
		Add_Code, Operator_TokenType, Double_DataType, OneChar_Multiple,
		"+", NULL, Null_Flag, Null_Code, 40,
		2, {Double_DataType, Double_DataType}, {AddInt_Code, CatStr_Code}
	},
	{
		Sub_Code, Operator_TokenType, Double_DataType, OneChar_Multiple,
		"-", NULL, Null_Flag, Neg_Code, 40,
		2, {Double_DataType, Double_DataType}, {SubInt_Code}
	},
	{
		Mul_Code, Operator_TokenType, Double_DataType, OneChar_Multiple,
		"*", NULL, Null_Flag, Null_Code, 46,
		2, {Double_DataType, Double_DataType}, {MulInt_Code}
	},
	{
		Div_Code, Operator_TokenType, Double_DataType, OneChar_Multiple,
		"/", NULL, Null_Flag, Null_Code, 46,
		2, {Double_DataType, Double_DataType}, {DivInt_Code}
	},
	{
		IntDiv_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"\\", NULL, Null_Flag, Null_Code, 44,
		2, {Double_DataType, Double_DataType}
	},
	{
		Power_Code, Operator_TokenType, Double_DataType, OneChar_Multiple,
		"^", NULL, Null_Flag, Null_Code, 50,
		2, {Double_DataType, Double_DataType}, {PowerMul_Code, PowerInt_Code}
	},
	{
		Eq_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"=", NULL, Null_Flag, Null_Code, 30,
		2, {Double_DataType, Double_DataType}, {EqInt_Code, EqStr_Code}
	},
	{
		Gt_Code, Operator_TokenType, Integer_DataType, TwoChar_Multiple,
		">", NULL, Null_Flag, Null_Code, 32,
		2, {Double_DataType, Double_DataType}, {GtInt_Code, GtStr_Code}
	},
	{
		GtEq_Code, Operator_TokenType, Integer_DataType, TwoChar_Multiple,
		">=", NULL, Null_Flag, Null_Code, 32,
		2, {Double_DataType, Double_DataType}, {GtEqInt_Code, GtEqStr_Code}
	},
	{
		Lt_Code, Operator_TokenType, Integer_DataType, TwoChar_Multiple,
		"<", NULL, Null_Flag, Null_Code, 32,
		2, {Double_DataType, Double_DataType}, {LtInt_Code, LtStr_Code}
	},
	{
		LtEq_Code, Operator_TokenType, Integer_DataType, TwoChar_Multiple,
		"<=", NULL, Null_Flag, Null_Code, 32,
		2, {Double_DataType, Double_DataType}, {LtEqInt_Code, LtEqStr_Code}
	},
	{
		NotEq_Code, Operator_TokenType, Integer_DataType, TwoChar_Multiple,
		"<>", NULL, Null_Flag, Null_Code, 30,
		2, {Double_DataType, Double_DataType}, {NotEqInt_Code, NotEqStr_Code}
	},
	{
		// 2010-03-25: set unary code, changed precedence value
		OpenParen_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"(", NULL, Null_Flag, OpenParen_Code, 2, 0
	},
	{
		// 2010-03-25: changed precedence value
		CloseParen_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		")", NULL, Null_Flag, Null_Code, 4, 0
	},
	{
		// 2010-04-02: changed precedence value
		Comma_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		",", NULL, Null_Flag, Null_Code, 4, 0
	},
	{
		SemiColon_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		";", NULL, Null_Flag, Null_Code, 0, 0
	},
	{
		Colon_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		":", NULL, Null_Flag, Null_Code, 0, 0
	},
	{
		RemOp_Code, Operator_TokenType, None_DataType, OneChar_Multiple,
		"'", NULL, Null_Flag, Null_Code, 0, 0
	},
	//*****************
	//   END SYMBOLS
	//*****************
	{
		EndSymbol_Code, Error_TokenType
	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	// 2010-03-16: added entries for new codes
	// 2010-04-24: moved null code entry to end
	{
		Neg_Code, Operator_TokenType, Double_DataType, OneWord_Multiple,
		// 2010-03-21: temporarily replaced "-" with "Neq" for testing
		// 2010-03-21: changed unary_code from Null_Code to Neg_Code
		// 2010-04-02: set name to correct output, name2 to debug output name
		"-", "Neg", Null_Flag, Neg_Code, 48,
		1, {Double_DataType}, {NegInt_Code}
	},
	// 2010-04-11: added entries for assignment operators
	{
		Assign_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"=", "Assign", Null_Flag, Null_Code, 4, 0
	},
	{
		AssignList_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		"=", "AssignList", Null_Flag, Null_Code, 4, 0
	},
	{
		EOL_Code, Operator_TokenType, None_DataType, OneWord_Multiple,
		NULL, NULL, Null_Flag, Null_Code, 4, 0
	},
	// 2010-04-24: added entries for associated codes
	{
		AddInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"+", "+%", Null_Flag, Null_Code, 40,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		CatStr_Code, Operator_TokenType, String_DataType, OneChar_Multiple,
		"+", "+$", Null_Flag, Null_Code, 40,
		2, {String_DataType, String_DataType}
	},
	{
		SubInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"-", "-%", Null_Flag, Null_Code, 40,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		NegInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"-", "Neg%", Null_Flag, Null_Code, 40,
		1, {Integer_DataType}
	},
	{
		MulInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"*", "*%", Null_Flag, Null_Code, 46,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		DivInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"/", "/%", Null_Flag, Null_Code, 46,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		ModInt_Code, Operator_TokenType, Integer_DataType, OneWord_Multiple,
		"MOD", "MOD%", Null_Flag, Null_Code, 42,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		PowerMul_Code, Operator_TokenType, Double_DataType, OneChar_Multiple,
		"^", "^*", Null_Flag, Null_Code, 50,
		2, {Double_DataType, Integer_DataType}
	},
	{
		PowerInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"^", "^%", Null_Flag, Null_Code, 50,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		EqInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"=", "=%", Null_Flag, Null_Code, 30,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		EqStr_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"=", "=$", Null_Flag, Null_Code, 30,
		2, {String_DataType, String_DataType}
	},
	{
		GtInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		">", ">%", Null_Flag, Null_Code, 32,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		GtStr_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		">", ">$", Null_Flag, Null_Code, 32,
		2, {String_DataType, String_DataType}
	},
	{
		GtEqInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		">=", ">=%", Null_Flag, Null_Code, 32,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		GtEqStr_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		">=", ">=$", Null_Flag, Null_Code, 32,
		2, {String_DataType, String_DataType}
	},
	{
		LtInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"<", "<%", Null_Flag, Null_Code, 32,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		LtStr_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"<", "<$", Null_Flag, Null_Code, 32,
		2, {String_DataType, String_DataType}
	},
	{
		LtEqInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"<=", "<=%", Null_Flag, Null_Code, 32,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		LtEqStr_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"<=", "<=$", Null_Flag, Null_Code, 32,
		2, {String_DataType, String_DataType}
	},
	{
		NotEqInt_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"<>", "<>%", Null_Flag, Null_Code, 30,
		2, {Integer_DataType, Integer_DataType}
	},
	{
		NotEqStr_Code, Operator_TokenType, Integer_DataType, OneChar_Multiple,
		"<>", "<>$", Null_Flag, Null_Code, 30,
		2, {String_DataType, String_DataType}
	},
	{
		AbsInt_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"ABS(", "ABS%(", Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		RndArgInt_Code, IntFuncP_TokenType, Double_DataType, OneWord_Multiple,
		"RND(", "RND%(", Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		SgnInt_Code, IntFuncP_TokenType, Integer_DataType, OneWord_Multiple,
		"SGN(", "SGN%(", Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		CvtInt_Code, IntFuncN_TokenType, Integer_DataType, OneWord_Multiple,
		NULL, "CvtInt", Null_Flag, Null_Code, 2,
		1, {Double_DataType}
	},
	{
		CvtDbl_Code, IntFuncN_TokenType, Double_DataType, OneWord_Multiple,
		NULL, "CvtDbl", Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		StrInt_Code, IntFuncP_TokenType, String_DataType, OneWord_Multiple,
		"STR$(", "STR%$(", Null_Flag, Null_Code, 2,
		1, {Integer_DataType}
	},
	{
		Null_Code, Operator_TokenType
	}
};


// constructor function that initializes the table variables

Table::Table(void)
{
	List<TableError> *error_list = new List<TableError>;
	int i;
	int type;

	entry = table_entries;

	// allocate and initialize code to index conversion array
	index_code = new int[sizeof_Code];
	for (i = 0; i < sizeof_Code; i++)
	{
		index_code[i] = -1;
	}
	// initialize bracketing code range indexes
	for (type = 0; type < sizeof_TableSearch; type++)
	{
		range[type].beg = range[type].end = -1;
	}

	// scan table and record indexes
	bool error = false;
	int nentries = sizeof(table_entries) / sizeof(TableEntry);
	for (i = 0; i < nentries; i++)
	{
		Code code = entry[i].code;
		if (index_code[code] == -1)
		{
			index_code[code] = i;
		}
		else  // already assigned
		{
			// record duplicated code error
			TableError error(code, index_code[code], i);
			error_list->append(&error);
		}
	}

	// check for missing codes
	for (i = 0; i < sizeof_Code; i++)
	{
		if (index_code[i] == -1)
		{
			// record code missing error
			TableError error((Code)i);
			error_list->append(&error);
		}
	}

	// setup indexes for bracketing codes
	// (will be set to -1 if missing - missing errors were recorded above)
	range[PlainWord_TableSearch].beg = index_code[BegPlainWord_Code];
	range[PlainWord_TableSearch].end = index_code[EndPlainWord_Code];
	range[ParenWord_TableSearch].beg = index_code[BegParenWord_Code];
	range[ParenWord_TableSearch].end = index_code[EndParenWord_Code];
	range[DataTypeWord_TableSearch].beg = index_code[BegDataTypeWord_Code];
	range[DataTypeWord_TableSearch].end = index_code[EndDataTypeWord_Code];
	range[Symbol_TableSearch].beg = index_code[BegSymbol_Code];
	range[Symbol_TableSearch].end = index_code[EndSymbol_Code];

	// check for missing bracketing codes and if properly positioned in table
	// (missing codes recorded above, however,
	// need to make sure all types were set, i.e. no missing assignments above)
	for (type = 0; type < sizeof_TableSearch; type++)
	{
		if (range[type].beg == -1 || range[type].end == -1
			|| range[type].beg > range[type].end)
		{
			// record bracket range error
			TableError error((TableSearch)type, range[type].beg,
				range[type].end);
			error_list->append(&error);
		}
		else
		{
			// check to make sure no bracketing codes overlap
			for (int type2 = 0; type2 < sizeof_TableSearch; type2++)
			{
				if (type != type2
					&& range[type2].beg != -1 && range[type2].end != -1
					&& (range[type].beg > range[type2].beg
					&& range[type].beg < range[type2].end
					|| range[type].end > range[type2].beg
					&& range[type].end < range[type2].end))
				{
					// record bracket overlap error
					TableError error((TableSearch)type, (TableSearch)type2,
						range[type].beg, range[type].end);
					error_list->append(&error);
				}
			}
		}
	}

	// throw exception if error_list is not empty
	if (!error_list->empty())
	{
		throw error_list;
	}

	delete error_list;
}


// this search function will look for an immediate command (located at
// the beginning of the table), which are only one letter
//
// the flags argument is for searching for an immediate command with a
// particular form (the letter and the appropriate flag in the table
// needs to match)
//
//     - returns -1 if the letter (and flags) is not found

int Table::search(char letter, int flag)
{
	for (int i = 0; entry[i].name != NULL; i++)
	{
		if (toupper(letter) == entry[i].name[0]
			&& (flag == Null_Flag || flag & entry[i].flags))
		{
			return i;
		}
	}
	return -1;  // not found
}


// this search function will look for a string of a particular type in
// the Table, the search is case insensitive
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

int Table::search(TableSearch type, const char *string, int len)
{
	int i = range[type].beg;
	int end = range[type].end;
	while (++i < end)
	{
		if (strncasecmp(string, entry[i].name, len) == 0
			&& entry[i].name[len] == '\0')
		{
			return i;
		}
	}
	return -1;
}


// this search function will look for a two word command in the Table,
// the search is case insensitive an only entries containing a second
// word is checked
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

int Table::search(const char *word1, int len1, const char *word2, int len2)
{
	for (int i = range[PlainWord_TableSearch].beg;
		i < range[PlainWord_TableSearch].end; i++)
	{
		if (entry[i].name2 != NULL
			&& strncasecmp(word1, entry[i].name, len1) == 0
			&& entry[i].name[len1] == '\0'
			&& strncasecmp(word2, entry[i].name2, len2) == 0
			&& entry[i].name2[len2] == '\0')
		{
			return i;
		}
	}
	return -1;
}


// this search function will look for a function matching the same
// function name as the index specified and matching the same number of
// arguments specified
//
//     - returns the index of the entry that is found
//     - returns -1 if the function with same noperands was not found
//     - case sensitive compare used (all entry names must match)
//     - name of function found at index specified
//     - search begins at entry after index
//     - search ends at end of section

// 2010-04-04: new function implemented
int Table::search(int index, int noperands)
{
	for (int i = index + 1; entry[i].name != NULL; i++)
	{
		if (strcmp(entry[index].name, entry[i].name) == 0
			&& noperands == entry[i].noperands)
		{
			return i;
		}
	}
	return -1;
}


// end: table.cpp
