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
//  2010-05-03  began to add support for initialization of expression
//                information structures for the table entries
//  2010-05-04  updated table entries for the expression information structures
//  2010-05-05  modified the assignment operator entries for data type handling
//              added entries for associated assignment operators
//              added Operands and AssocCode macros
//  2010-05-08  added expression information structures for unary operators 
//
//  2010-05-15  modified codes that return string to return TmpStr (Chr, Repeat,
//                Space, Str, CatStr, and StrInt) - new ExprInfos were added
//              added String_Flag to all codes that have string operands
//
//  2010-05-19  changed data type of LEFT, MID2, MID3 and RIGHT to SubStr
//              added new associated code AssignSubStr_Code to Assign_Code along
//                with their new table entries
//  2010-05-20  added code to check if the Max_Operands and Max_Assoc_Codes
//              agree with the tables entries, reporting errors if not
//
//  2010-05-22  removed String_Flag from codes with string operands and replaced
//                this with the automatic setting the flag by looking at the
//                entries during table initialization
//              added new associated code AssignListMixStr along with its new
//                table entry
//
//  2010-05-27  changed precedence values for CloseParen_Code and Comma_Code
//                from 4 to 6 so commands are not emptied from the hold stack
//              added precedence value of 4 (and NUll_Flag) to all commands
//  2010-05-28  added value for new token_mode to Let_Code
//              added values for new token_handler to Eq_Code, CloseParen_Code,
//                Comma_Code, and EOL_Code
//  2010-05-29  added Hidden_Flag to CvtInt_Code and CvtDbl_Code
//
//  2010-06-01  added print-only function support (Spc_Code and Tab_Code)
//              initiated PRINT command development
//  2010-06-02  added data type specific print hidden code entries
//              changed precedence and added token handler value to SemiColon
//  2010-06-05  added command handler function pointers for PRINT and assign
//              table entries
//  2010-06-06  added end expression flag to Comma, SemiColon and EOL
//  2010-06-09  swapped table entries of MID$, INSTR, and ASC so that the larger
//              number of arguments entry is first, which is necessary for the
//              new number of arguments checking at comma
//  2010-06-13  added command handler to Let entry
//  2010-06-14  removed AssignList_Flag from AssignList table entries
//
//  2010-06-25  Replaced TableError with generic Error template
//              renamed TableSearch enum to SearchType
//  2010-06-26  added end statment flag to EOL
//  2010-06-29  added expr_type value to PRINT code
//  2010-07-02  changed assign and assign list operators expression information
//                from two operands to one - the assign operators are no longer
//                handled by the standard operator routines
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "ibcp.h"


#define ArraySize(array)  (sizeof(array) / sizeof(array[0]))


// 2010-05-03: expression information for codes

// 2010-05-05: this macro produces two entries for the ExprInfo constructor,
//             one for the number of operands, one for the pointer to the
//             operand data type array

#define Operands(type)  (sizeof(type ## _OperandArray) \
	/ sizeof(type ## _OperandArray[0])), type ## _OperandArray

#define AssocCode(code)  (sizeof(code ## _AssocCode) \
	/ sizeof(code ## _AssocCode[0])), code ## _AssocCode


// 2010-05-05: operand data type arrays
DataType Dbl_OperandArray[] = {
	Double_DataType
};
DataType DblDbl_OperandArray[] = {
	Double_DataType, Double_DataType
};
DataType DblInt_OperandArray[] = {
	Double_DataType, Integer_DataType
};
DataType Int_OperandArray[] = {
	Integer_DataType
};
DataType IntInt_OperandArray[] = {
	Integer_DataType, Integer_DataType
};
DataType Str_OperandArray[] = {
	String_DataType
};
DataType StrInt_OperandArray[] = {
	String_DataType, Integer_DataType
};
DataType StrIntInt_OperandArray[] = {
	String_DataType, Integer_DataType, Integer_DataType
};
DataType StrStr_OperandArray[] = {
	String_DataType, String_DataType
};
DataType StrStrInt_OperandArray[] = {
	String_DataType, String_DataType, Integer_DataType
};
DataType Sub_OperandArray[] = {  // 2010-07-01
	SubStr_DataType
};
// 2010-05-19: added new operand data type array
DataType SubStr_OperandArray[] = {
	SubStr_DataType, String_DataType
};


// 2010-05-06: associated code data type arrays
Code Abs_AssocCode[]        = {AbsInt_Code};
Code Add_AssocCode[]        = {AddInt_Code, CatStr_Code};
// 2010-05-19: added AssignSubStr_Code to list
Code Assign_AssocCode[]     = {
	AssignInt_Code, AssignStr_Code, AssignSubStr_Code
};
// 2010-05-19: added AssignListMaxStr_Code to list
Code AssignList_AssocCode[] = {
	AssignListInt_Code, AssignListStr_Code, AssignListMixStr_Code
};
Code Div_AssocCode[]        = {DivInt_Code};
Code Eq_AssocCode[]         = {EqInt_Code, EqStr_Code};
Code Gt_AssocCode[]         = {GtInt_Code, GtStr_Code};
Code GtEq_AssocCode[]       = {GtEqInt_Code, GtEqStr_Code};
Code Lt_AssocCode[]         = {LtInt_Code, LtStr_Code};
Code LtEq_AssocCode[]       = {LtEqInt_Code, LtEqStr_Code};
Code Mod_AssocCode[]        = {ModInt_Code};
Code Mul_AssocCode[]        = {MulInt_Code};
Code Neg_AssocCode[]        = {NegInt_Code};
Code NotEq_AssocCode[]      = {NotEqInt_Code, NotEqStr_Code};
Code Power_AssocCode[]      = {PowerMul_Code, PowerInt_Code};
// 2010-06-02: added associated codes for data type specific print
Code Print_AssocCode[]		= {PrintInt_Code, PrintStr_Code};
Code RndArgs_AssocCode[]    = {RndArgInt_Code};
Code Sgn_AssocCode[]        = {SgnInt_Code};
Code Str_AssocCode[]        = {StrInt_Code};
Code Sub_AssocCode[]        = {SubInt_Code};


// 2010-05-06: standard expression information structures
ExprInfo Dbl_Dbl_ExprInfo(Double_DataType, Null_Code, Operands(Dbl));
ExprInfo Dbl_DblInt_ExprInfo(Double_DataType, Null_Code, Operands(DblInt));
ExprInfo Dbl_Int_ExprInfo(Double_DataType, Null_Code, Operands(Int));
ExprInfo Dbl_Str_ExprInfo(Double_DataType, Null_Code, Operands(Str));

ExprInfo Int_Dbl_ExprInfo(Integer_DataType, Null_Code, Operands(Dbl));
ExprInfo Int_DblDbl_ExprInfo(Integer_DataType, Null_Code, Operands(DblDbl));
ExprInfo Int_Int_ExprInfo(Integer_DataType, Null_Code, Operands(Int));
ExprInfo Int_IntInt_ExprInfo(Integer_DataType, Null_Code, Operands(IntInt));
ExprInfo Int_Str_ExprInfo(Integer_DataType, Null_Code, Operands(Str));
ExprInfo Int_StrInt_ExprInfo(Integer_DataType, Null_Code, Operands(StrInt));
ExprInfo Int_StrStr_ExprInfo(Integer_DataType, Null_Code, Operands(StrStr));
ExprInfo Int_StrStrInt_ExprInfo(Integer_DataType, Null_Code,
	Operands(StrStrInt));

ExprInfo Str_Str_ExprInfo(String_DataType, Null_Code, Operands(Str));
ExprInfo Str_StrStr_ExprInfo(String_DataType, Null_Code, Operands(StrStr));

// 2010-05-15: changed to return TmpStr
ExprInfo Tmp_Int_ExprInfo(TmpStr_DataType, Null_Code, Operands(Int));
ExprInfo Tmp_StrInt_ExprInfo(TmpStr_DataType, Null_Code, Operands(StrInt));
ExprInfo Tmp_StrStr_ExprInfo(TmpStr_DataType, Null_Code, Operands(StrStr));

// 2010-05-19: changed to return SubStr
ExprInfo Sub_StrInt_ExprInfo(SubStr_DataType, Null_Code, Operands(StrInt));
ExprInfo Sub_StrIntInt_ExprInfo(SubStr_DataType, Null_Code,
	Operands(StrIntInt));
ExprInfo Str_Sub_ExprInfo(String_DataType, Null_Code, Operands(Sub));
// 2010-05-19: added new expression info
ExprInfo Str_SubStr_ExprInfo(String_DataType, Null_Code, Operands(SubStr));

ExprInfo None_Int_ExprInfo(None_DataType, Null_Code, Operands(Int));
// 2010-06-02: added none with string operand expression info
ExprInfo None_Str_ExprInfo(None_DataType, Null_Code, Operands(Str));


static TableEntry table_entries[] = {
	//******************************
	//   IMMEDIATE COMMANDS FIRST
	//******************************
	// (these will go away once gui interface is implemented)
	{
		List_Code, ImmCmd_TokenType, OneWord_Multiple,
		"L", NULL, Blank_Flag | Line_Flag | Range_Flag
	},
	{
		Edit_Code, ImmCmd_TokenType, OneWord_Multiple,
		"E", NULL, Blank_Flag | Line_Flag
	},
	{
		Delete_Code, ImmCmd_TokenType, OneWord_Multiple,
		"D", NULL, Line_Flag | Range_Flag
	},
	{
		Run_Code, ImmCmd_TokenType, OneWord_Multiple,
		"R", NULL, Blank_Flag
	},
	{
		Renum_Code, ImmCmd_TokenType, OneWord_Multiple,
		"R", NULL, Range_Flag | RangeIncr_Flag
	},
	{
		Save_Code, ImmCmd_TokenType, OneWord_Multiple,
		"S", NULL, Blank_Flag | String_Flag
	},
	{
		Load_Code, ImmCmd_TokenType, OneWord_Multiple,
		"L", NULL, String_Flag
	},
	{
		New_Code, ImmCmd_TokenType, OneWord_Multiple,
		"N", NULL, Blank_Flag
	},
	{
		Auto_Code, ImmCmd_TokenType, OneWord_Multiple,
		"A", NULL, Blank_Flag | Line_Flag | LineIncr_Flag
	},
	{
		Cont_Code, ImmCmd_TokenType, OneWord_Multiple,
		"C", NULL, Blank_Flag
	},
	{
		Quit_Code, ImmCmd_TokenType, OneWord_Multiple,
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
		// 2010-05-28: added value for token_mode
		// 2010-06-13: added value for command handler
		Let_Code, Command_TokenType, OneWord_Multiple,
		"LET", NULL, Null_Flag, 4, NULL, NULL, Assignment_TokenMode,
		Let_CmdHandler
	},
	{
		// 2010-06-01: added value for token_mode
		// 2010-06-05: added value for command handler
		// 2010-06-29: added value for expr_type
		Print_Code, Command_TokenType, OneWord_Multiple,
		"PRINT", NULL, Null_Flag, 4, NULL, NULL, Expression_TokenMode,
		Print_CmdHandler, Any_ExprType
	},
	{
		Input_Code, Command_TokenType, OneWord_Multiple,
		"INPUT", NULL, Null_Flag, 4
	},
	{
		Dim_Code, Command_TokenType, OneWord_Multiple,
		"DIM", NULL, Null_Flag, 4
	},
	{
		Def_Code, Command_TokenType, OneWord_Multiple,
		"DEF", NULL, Null_Flag, 4
	},
	{
		Rem_Code, Command_TokenType, OneWord_Multiple,
		"REM", NULL, Null_Flag, 4
	},
	{
		If_Code, Command_TokenType, OneWord_Multiple,
		"IF", NULL, Null_Flag, 4
	},
	{
		Then_Code, Command_TokenType, OneWord_Multiple,
		"THEN", NULL, Null_Flag, 4
	},
	{
		Else_Code, Command_TokenType, OneWord_Multiple,
		"ELSE", NULL, Null_Flag, 4
	},
	{
		End_Code, Command_TokenType, TwoWord_Multiple,
		"END", NULL, Null_Flag, 4
	},
	{
		EndIf_Code, Command_TokenType, TwoWord_Multiple,
		"END", "IF", Null_Flag, 4
	},
	{
		For_Code, Command_TokenType, OneWord_Multiple,
		"FOR", NULL, Null_Flag, 4
	},
	{
		To_Code, Command_TokenType, OneWord_Multiple,
		"TO", NULL, Null_Flag, 4
	},
	{
		Step_Code, Command_TokenType, OneWord_Multiple,
		"STEP", NULL, Null_Flag, 4
	},
	{
		Next_Code, Command_TokenType, OneWord_Multiple,
		"NEXT", NULL, Null_Flag, 4
	},
	{
		Do_Code, Command_TokenType, TwoWord_Multiple,
		"Do", NULL, Null_Flag, 4
	},
	{
		DoWhile_Code, Command_TokenType, TwoWord_Multiple,
		"DO", "WHILE", Null_Flag, 4
	},
	{
		DoUntil_Code, Command_TokenType, TwoWord_Multiple,
		"DO", "UNTIL", Null_Flag, 4
	},
	{
		While_Code, Command_TokenType, TwoWord_Multiple,
		"WHILE", NULL, Null_Flag, 4
	},
	{
		Until_Code, Command_TokenType, TwoWord_Multiple,
		"UNTIL", NULL, Null_Flag, 4
	},
	{
		Loop_Code, Command_TokenType, TwoWord_Multiple,
		"LOOP", NULL, Null_Flag, 4
	},
	{
		LoopWhile_Code, Command_TokenType, TwoWord_Multiple,
		"LOOP", "WHILE", Null_Flag, 4
	},
	{
		LoopUntil_Code, Command_TokenType, TwoWord_Multiple,
		"LOOP", "UNTIL", Null_Flag, 4
	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{
		Rnd_Code, IntFuncN_TokenType, OneWord_Multiple,
		"RND", NULL, Null_Flag, 2,
		new ExprInfo(Double_DataType, Null_Code)

	},
	//--------------------
	//   Word Operators
	//--------------------
	{
		Mod_Code, Operator_TokenType, OneWord_Multiple,
		"MOD", NULL, Null_Flag, 42,
		new ExprInfo(Double_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Mod))
	},
	{
		And_Code, Operator_TokenType, OneWord_Multiple,
		"AND", NULL, Null_Flag, 18, &Int_IntInt_ExprInfo
	},
	{
		Or_Code, Operator_TokenType, OneWord_Multiple,
		"OR", NULL, Null_Flag, 14, &Int_IntInt_ExprInfo
	},
	// 2010-05-08: Not needs its own exprinfo struct
	{
		Not_Code, Operator_TokenType, OneWord_Multiple,
		"NOT", NULL, Null_Flag, 20,
		new ExprInfo(Integer_DataType, Not_Code, Operands(Int))
	},
	{
		Eqv_Code, Operator_TokenType, OneWord_Multiple,
		"EQV", NULL, Null_Flag, 12, &Int_IntInt_ExprInfo
	},
	{
		Imp_Code, Operator_TokenType, OneWord_Multiple,
		"IMP", NULL, Null_Flag, 10, &Int_IntInt_ExprInfo
	},
	{
		Xor_Code, Operator_TokenType, OneWord_Multiple,
		"XOR", NULL, Null_Flag, 16, &Int_IntInt_ExprInfo
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
		Abs_Code, IntFuncP_TokenType, OneWord_Multiple,
		"ABS(", NULL, Null_Flag, 2,
		new ExprInfo(Double_DataType, Null_Code, Operands(Dbl), AssocCode(Abs))
	},
	{
		Fix_Code, IntFuncP_TokenType, OneWord_Multiple,
		"FIX(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Frac_Code, IntFuncP_TokenType, OneWord_Multiple,
		"FRAC(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Int_Code, IntFuncP_TokenType, OneWord_Multiple,
		"INT(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo

	},
	{
		RndArg_Code, IntFuncP_TokenType, OneWord_Multiple,
		"RND(", NULL, Null_Flag, 2,
		new ExprInfo(Double_DataType, Null_Code, Operands(Dbl),
			AssocCode(RndArgs))
	},
	{
		Sgn_Code, IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", NULL, Null_Flag, 2,
		new ExprInfo(Double_DataType, Null_Code, Operands(Dbl),
			AssocCode(Sgn))
	},
	{
		Cint_Code, IntFuncP_TokenType, OneWord_Multiple,
		"CINT(", NULL, Null_Flag, 2, &Int_Dbl_ExprInfo
	},
	{
		Cdbl_Code, IntFuncP_TokenType, OneWord_Multiple,
		"CDBL(", NULL, Null_Flag, 2, &Dbl_Int_ExprInfo
	},
	{
		Sqr_Code, IntFuncP_TokenType, OneWord_Multiple,
		"SQR(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Atn_Code, IntFuncP_TokenType, OneWord_Multiple,
		"ATN(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Cos_Code, IntFuncP_TokenType, OneWord_Multiple,
		"COS(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Sin_Code, IntFuncP_TokenType, OneWord_Multiple,
		"SIN(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Tan_Code, IntFuncP_TokenType, OneWord_Multiple,
		"TAN(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Exp_Code, IntFuncP_TokenType, OneWord_Multiple,
		"EXP(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		Log_Code, IntFuncP_TokenType, OneWord_Multiple,
		"LOG(", NULL, Null_Flag, 2, &Dbl_Dbl_ExprInfo
	},
	{
		// 2010-06-01: added print-only flag
		Tab_Code, IntFuncP_TokenType, OneWord_Multiple,
		"TAB(", NULL, Print_Flag, 2, &None_Int_ExprInfo
	},
	{
		// 2010-06-01: added print-only flag
		Spc_Code, IntFuncP_TokenType, OneWord_Multiple,
		"SPC(", NULL, Print_Flag, 2, &None_Int_ExprInfo
	},
	// 2010-04-04: added entry for 2 argument ASC
	// 2010-06-09: swapped ASC and ASC2 entries
	{
		Asc2_Code, IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASC2(", Multiple_Flag, 2, &Int_StrInt_ExprInfo
	},
	{
		Asc_Code, IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", NULL, Null_Flag, 2, &Int_Str_ExprInfo
	},
	{
		Chr_Code, IntFuncP_TokenType, OneWord_Multiple,
		"CHR$(", NULL, Null_Flag, 2, &Tmp_Int_ExprInfo
	},
	// 2010-04-04: replaced INSTR entry with INSTR2 and INSTR3 entries
	// 2010-06-09: swapped INSTR2 and INSTR3 entries
	{
		Instr3_Code, IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3(", Multiple_Flag, 2, &Int_StrStrInt_ExprInfo
	},
	{
		Instr2_Code, IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2(", Null_Flag, 2,
		&Int_StrStr_ExprInfo
	},
	{
		Left_Code, IntFuncP_TokenType, OneWord_Multiple,
		"LEFT$(", NULL, Null_Flag, 2, &Sub_StrInt_ExprInfo
	},
	{
		Len_Code, IntFuncP_TokenType, OneWord_Multiple,
		"LEN(", NULL, Null_Flag, 2, &Int_Str_ExprInfo
	},
	// 2010-04-04: replaced MID entry with MID2 and MID3 entries
	// 2010-06-09: swapped MID2 and MID3 entries
	{
		Mid3_Code, IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID3$(", Multiple_Flag, 2, &Sub_StrIntInt_ExprInfo
	},
	{
		Mid2_Code, IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID2$(", Null_Flag, 2, &Sub_StrInt_ExprInfo
	},
	{
		Repeat_Code, IntFuncP_TokenType, OneWord_Multiple,
		"REPEAT$(", NULL, Null_Flag, 2, &Tmp_StrInt_ExprInfo
	},
	{
		Right_Code, IntFuncP_TokenType, OneWord_Multiple,
		"RIGHT$(", NULL, Null_Flag, 2, &Sub_StrInt_ExprInfo
	},
	{
		Space_Code, IntFuncP_TokenType, OneWord_Multiple,
		"SPACE$(", NULL, Null_Flag, 2, &Tmp_Int_ExprInfo
	},
	{
		// 2010-05-15: changed to return TmpStr
		Str_Code, IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", NULL, Null_Flag, 2,
		new ExprInfo(TmpStr_DataType, Null_Code, Operands(Dbl), AssocCode(Str))
	},
	{
		// 2010-04-02: changed name to all upper case
		Val_Code, IntFuncP_TokenType, OneWord_Multiple,
		"VAL(", NULL, Null_Flag, 2, &Dbl_Str_ExprInfo
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
		Add_Code, Operator_TokenType, OneChar_Multiple,
		"+", NULL, Null_Flag, 40,
		new ExprInfo(Double_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Add))
	},
	{
		Sub_Code, Operator_TokenType, OneChar_Multiple,
		"-", NULL, Null_Flag, 40,
		new ExprInfo(Double_DataType, Neg_Code, Operands(DblDbl),
			AssocCode(Sub))
	},
	{
		Mul_Code, Operator_TokenType, OneChar_Multiple,
		"*", NULL, Null_Flag, 46,
		new ExprInfo(Double_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Mul))
	},
	{
		Div_Code, Operator_TokenType, OneChar_Multiple,
		"/", NULL, Null_Flag, 46,
		new ExprInfo(Double_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Div))
	},
	{
		IntDiv_Code, Operator_TokenType, OneChar_Multiple,
		"\\", NULL, Null_Flag, 44, &Int_DblDbl_ExprInfo
	},
	{
		Power_Code, Operator_TokenType, OneChar_Multiple,
		"^", NULL, Null_Flag, 50,
		new ExprInfo(Double_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Power))
	},
	{
		// 2010-05-28: added value for token_handler
		Eq_Code, Operator_TokenType, OneChar_Multiple,
		"=", NULL, Null_Flag, 30,
		new ExprInfo(Integer_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Eq)),
		Equal_Handler
	},
	{
		Gt_Code, Operator_TokenType, TwoChar_Multiple,
		">", NULL, Null_Flag, 32,
		new ExprInfo(Integer_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Gt))
	},
	{
		GtEq_Code, Operator_TokenType, TwoChar_Multiple,
		">=", NULL, Null_Flag, 32,
		new ExprInfo(Integer_DataType, Null_Code, Operands(DblDbl),
			AssocCode(GtEq))
	},
	{
		Lt_Code, Operator_TokenType, TwoChar_Multiple,
		"<", NULL, Null_Flag, 32,
		new ExprInfo(Integer_DataType, Null_Code, Operands(DblDbl),
			AssocCode(Lt))
	},
	{
		LtEq_Code, Operator_TokenType, TwoChar_Multiple,
		"<=", NULL, Null_Flag, 32,
		new ExprInfo(Integer_DataType, Null_Code, Operands(DblDbl),
			AssocCode(LtEq))
	},
	{
		NotEq_Code, Operator_TokenType, TwoChar_Multiple,
		"<>", NULL, Null_Flag, 30,
		new ExprInfo(Integer_DataType, Null_Code, Operands(DblDbl),
			AssocCode(NotEq))
	},
	{
		// 2010-03-25: set unary code, changed precedence value
		OpenParen_Code, Operator_TokenType, OneChar_Multiple,
		"(", NULL, Null_Flag, 2,
		new ExprInfo(None_DataType, OpenParen_Code)
	},
	{
		// 2010-03-25: changed precedence value from 0 to 4
		// 2010-05-27: changed precedence value from 4 to 6
		// 2010-05-28: added value for token_handler
		CloseParen_Code, Operator_TokenType, OneChar_Multiple,
		")", NULL, Null_Flag, 4, NULL,
		CloseParen_Handler
	},
	{
		// 2010-04-02: changed precedence value from 0 to 4
		// 2010-05-27: changed precedence value from 4 to 6
		// 2010-05-28: added value for token_handler
		// 2010-06-06: added end expression flag
		Comma_Code, Operator_TokenType, OneChar_Multiple,
		",", NULL, EndExpr_Flag, 6,
		NULL,//new ExprInfo(None_DataType, Comma_Code),
		Comma_Handler
	},
	{
		// 2010-06-02: changed precedence from 0, added token_handler value
		// 2010-06-06: added end expression flag
		SemiColon_Code, Operator_TokenType, OneChar_Multiple,
		";", NULL, EndExpr_Flag, 6,
		NULL,//new ExprInfo(None_DataType, SemiColon_Code),
		SemiColon_Handler
	},
	{
		Colon_Code, Operator_TokenType, OneChar_Multiple,
		":", NULL, Null_Flag, 0
	},
	{
		RemOp_Code, Operator_TokenType, OneChar_Multiple,
		"'", NULL, Null_Flag, 0
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
		Neg_Code, Operator_TokenType, OneWord_Multiple,
		// 2010-03-21: temporarily replaced "-" with "Neq" for testing
		// 2010-03-21: changed unary_code from Null_Code to Neg_Code
		// 2010-04-02: set name to correct output, name2 to debug output name
		"-", "Neg", Null_Flag, 48,
		new ExprInfo(Double_DataType, Neg_Code, Operands(Dbl), AssocCode(Neg))
	},
	// 2010-04-11: added entries for assignment operators
	// 2010-05-05: added reference flag to assignment operators
	// 2010-06-05: added command handler function pointers to all assign entries
	// 2010-07-02: changed all assign operators from 2 to 1 operands
	{
		Assign_Code, Operator_TokenType, OneWord_Multiple,
		"=", "Assign", Reference_Flag, 4,
		new ExprInfo(Double_DataType, Null_Code, Operands(Dbl),
			AssocCode(Assign)), NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-05: added entries for assignment associated codes
	{
		AssignInt_Code, Operator_TokenType, OneWord_Multiple,
		"=", "Assign%", Reference_Flag, 4, &Int_Int_ExprInfo,
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{
		AssignStr_Code, Operator_TokenType, OneWord_Multiple,
		"=", "Assign$", Reference_Flag | String_Flag, 4, &Str_Str_ExprInfo,
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-19: added entries for assign sub-string associated code
	{
		AssignSubStr_Code, Operator_TokenType, OneWord_Multiple,
		"=", "AssignSub$", Reference_Flag | String_Flag, 4, &Str_Sub_ExprInfo,
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-05: added reference and assign list flags
	// 2010-06-14: removed assign list flags
	// 2010-07-02: changed all assign list operators from 2 to 1 operands
	{
		AssignList_Code, Operator_TokenType, OneWord_Multiple,
		"=", "AssignList", Reference_Flag, 4,
		new ExprInfo(Double_DataType, Null_Code, Operands(Dbl),
			AssocCode(AssignList)), NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-05: added entries for assignment associated codes
	{
		AssignListInt_Code, Operator_TokenType, OneWord_Multiple,
		"=", "AssignList%", Reference_Flag, 4,
		&Int_Int_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	{
		AssignListStr_Code, Operator_TokenType, OneWord_Multiple,
		"=", "AssignList$", Reference_Flag | String_Flag, 4,
		&Str_Str_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-22: added entry for assign mix string list associated code
	{
		AssignListMixStr_Code, Operator_TokenType, OneWord_Multiple,
		"=", "AssignListMix$", Reference_Flag | String_Flag, 4,
		&Str_Sub_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	{
		// 2010-05-28: added value for token_handler
		// 2010-06-06: added end expression flag
		// 2010-06-26: added end statment flag
		EOL_Code, Operator_TokenType, OneWord_Multiple,
		NULL, NULL, EndExpr_Flag | EndStatement_Flag, 4, NULL,
		EndOfLine_Handler
	},
	// 2010-04-24: added entries for associated codes
	{
		AddInt_Code, Operator_TokenType, OneChar_Multiple,
		"+", "+%", Null_Flag, 40, &Int_IntInt_ExprInfo
	},
	{
		CatStr_Code, Operator_TokenType, OneChar_Multiple,
		"+", "+$", Null_Flag, 40, &Tmp_StrStr_ExprInfo
	},
	{
		SubInt_Code, Operator_TokenType, OneChar_Multiple,
		"-", "-%", Null_Flag, 40, &Int_IntInt_ExprInfo
	},
	// 2010-05-08: NegInt needs its own exprinfo struct
	{
		NegInt_Code, Operator_TokenType, OneChar_Multiple,
		"-", "Neg%", Null_Flag, 40,
		new ExprInfo(Integer_DataType, NegInt_Code, Operands(Int))
	},
	{
		MulInt_Code, Operator_TokenType, OneChar_Multiple,
		"*", "*%", Null_Flag, 46, &Int_IntInt_ExprInfo
	},
	{
		DivInt_Code, Operator_TokenType, OneChar_Multiple,
		"/", "/%", Null_Flag, 46, &Int_IntInt_ExprInfo
	},
	{
		ModInt_Code, Operator_TokenType, OneWord_Multiple,
		"MOD", "MOD%", Null_Flag, 42, &Int_IntInt_ExprInfo
	},
	{
		PowerMul_Code, Operator_TokenType, OneChar_Multiple,
		"^", "^*", Null_Flag, 50, &Dbl_DblInt_ExprInfo
	},
	{
		PowerInt_Code, Operator_TokenType, OneChar_Multiple,
		"^", "^%", Null_Flag, 50, &Int_IntInt_ExprInfo
	},
	{
		EqInt_Code, Operator_TokenType, OneChar_Multiple,
		"=", "=%", Null_Flag, 30, &Int_IntInt_ExprInfo
	},
	{
		EqStr_Code, Operator_TokenType, OneChar_Multiple,
		"=", "=$", Null_Flag, 30, &Int_StrStr_ExprInfo
	},
	{
		GtInt_Code, Operator_TokenType, OneChar_Multiple,
		">", ">%", Null_Flag, 32, &Int_IntInt_ExprInfo
	},
	{
		GtStr_Code, Operator_TokenType, OneChar_Multiple,
		">", ">$", Null_Flag, 32, &Int_StrStr_ExprInfo
	},
	{
		GtEqInt_Code, Operator_TokenType, OneChar_Multiple,
		">=", ">=%", Null_Flag, 32, &Int_IntInt_ExprInfo
	},
	{
		GtEqStr_Code, Operator_TokenType, OneChar_Multiple,
		">=", ">=$", Null_Flag, 32, &Int_StrStr_ExprInfo
	},
	{
		LtInt_Code, Operator_TokenType, OneChar_Multiple,
		"<", "<%", Null_Flag, 32, &Int_IntInt_ExprInfo
	},
	{
		LtStr_Code, Operator_TokenType, OneChar_Multiple,
		"<", "<$", Null_Flag, 32, &Int_StrStr_ExprInfo
	},
	{
		LtEqInt_Code, Operator_TokenType, OneChar_Multiple,
		"<=", "<=%", Null_Flag, 32, &Int_IntInt_ExprInfo
	},
	{
		LtEqStr_Code, Operator_TokenType, OneChar_Multiple,
		"<=", "<=$", Null_Flag, 32, &Int_StrStr_ExprInfo
	},
	{
		NotEqInt_Code, Operator_TokenType, OneChar_Multiple,
		"<>", "<>%", Null_Flag, 30, &Int_IntInt_ExprInfo
	},
	{
		NotEqStr_Code, Operator_TokenType, OneChar_Multiple,
		"<>", "<>$", Null_Flag, 30, &Int_StrStr_ExprInfo
	},
	{
		AbsInt_Code, IntFuncP_TokenType, OneWord_Multiple,
		"ABS(", "ABS%(", Null_Flag, 2, &Int_Int_ExprInfo
	},
	{
		RndArgInt_Code, IntFuncP_TokenType, OneWord_Multiple,
		"RND(", "RND%(", Null_Flag, 2, &Int_Int_ExprInfo
	},
	{
		SgnInt_Code, IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", "SGN%(", Null_Flag, 2, &Int_Int_ExprInfo
	},
	{
		// 2010-05-29: added Hidden_Flag
		CvtInt_Code, IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtInt", Hidden_Flag, 2
	},
	{
		// 2010-05-29: added Hidden_Flag
		CvtDbl_Code, IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtDbl", Hidden_Flag, 2
	},
	{
		StrInt_Code, IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", "STR%$(", Null_Flag, 2, &Tmp_Int_ExprInfo
	},
	// 2010-06-02: added hidden print codes (main plus associated codes)
	{
		PrintDbl_Code, IntFuncP_TokenType, OneWord_Multiple,
		"", "PrintDbl", Null_Flag, 2,
		new ExprInfo(None_DataType, Null_Code, Operands(Dbl), AssocCode(Print))
	},
	{
		PrintInt_Code, IntFuncP_TokenType, OneWord_Multiple,
		"", "PrintInt", Null_Flag, 2, &None_Int_ExprInfo
	},
	{
		PrintStr_Code, IntFuncP_TokenType, OneWord_Multiple,
		"", "PrintStr", String_Flag, 2, &None_Str_ExprInfo
	},
	{
		Null_Code, Operator_TokenType
	}
};


// constructor function that initializes the table variables

Table::Table(void)
{
	List<Error<Code> > *error_list = new List<Error<Code> >;
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
	for (type = 0; type < sizeof_SearchType; type++)
	{
		range[type].beg = range[type].end = -1;
	}

	// scan table and record indexes
	bool error = false;
	int nentries = sizeof(table_entries) / sizeof(TableEntry);
	// 2010-05-20: find maximum number of operands and associated codes
	int max_operands = 0;
	int max_assoc_codes = 0;
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
			Error<Code> error(code, index_code[code], i);
			error_list->append(&error);
		}
		// 2010-05-20: check if found new maximums
		ExprInfo *exprinfo = entry[i].exprinfo;
		if (exprinfo != NULL)
		{
			if (max_operands < exprinfo->noperands)
			{
				max_operands = exprinfo->noperands;
			}
			if (max_assoc_codes < exprinfo->nassoc_codes)
			{
				max_assoc_codes = exprinfo->nassoc_codes;
			}

			// 2010-05-22: set String_Flag in entries automatically
			for (int j = 0; j < exprinfo->noperands; j++)
			{
				if (exprinfo->operand_datatype[j] == String_DataType)
				{
					entry[i].flags |= String_Flag;
					break;
				}
			}
		}
	}
	
	// 2010-05-20: check maximums found against constants
	if (max_operands > Max_Operands)
	{
		Error<Code> error(MaxOperands_ErrorType, max_operands);
		error_list->append(&error);
	}
	if (max_assoc_codes > Max_Assoc_Codes)
	{
		Error<Code> error(MaxAssocCodes_ErrorType, max_assoc_codes);
		error_list->append(&error);
	}

	// check for missing codes
	for (i = 0; i < sizeof_Code; i++)
	{
		if (index_code[i] == -1)
		{
			// record code missing error
			Error<Code> error((Code)i);
			error_list->append(&error);
		}
	}

	// setup indexes for bracketing codes
	// (will be set to -1 if missing - missing errors were recorded above)
	range[PlainWord_SearchType].beg = index_code[BegPlainWord_Code];
	range[PlainWord_SearchType].end = index_code[EndPlainWord_Code];
	range[ParenWord_SearchType].beg = index_code[BegParenWord_Code];
	range[ParenWord_SearchType].end = index_code[EndParenWord_Code];
	range[DataTypeWord_SearchType].beg = index_code[BegDataTypeWord_Code];
	range[DataTypeWord_SearchType].end = index_code[EndDataTypeWord_Code];
	range[Symbol_SearchType].beg = index_code[BegSymbol_Code];
	range[Symbol_SearchType].end = index_code[EndSymbol_Code];

	// check for missing bracketing codes and if properly positioned in table
	// (missing codes recorded above, however,
	// need to make sure all types were set, i.e. no missing assignments above)
	for (type = 0; type < sizeof_SearchType; type++)
	{
		if (range[type].beg == -1 || range[type].end == -1
			|| range[type].beg > range[type].end)
		{
			// record bracket range error
			Error<Code> error((SearchType)type, range[type].beg,
				range[type].end);
			error_list->append(&error);
		}
		else
		{
			// check to make sure no bracketing codes overlap
			for (int type2 = 0; type2 < sizeof_SearchType; type2++)
			{
				if (type != type2
					&& range[type2].beg != -1 && range[type2].end != -1
					&& (range[type].beg > range[type2].beg
					&& range[type].beg < range[type2].end
					|| range[type].end > range[type2].beg
					&& range[type].end < range[type2].end))
				{
					// record bracket overlap error
					Error<Code> error((SearchType)type, (SearchType)type2,
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

int Table::search(SearchType type, const char *string, int len)
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
	for (int i = range[PlainWord_SearchType].beg;
		i < range[PlainWord_SearchType].end; i++)
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
int Table::search(int index, int _noperands)
{
	for (int i = index + 1; entry[i].name != NULL; i++)
	{
		if (strcmp(entry[index].name, entry[i].name) == 0
			&& _noperands == noperands(i))
		{
			return i;
		}
	}
	return -1;
}


// this search function will look for a code matching the same
// operand data types as specified in the argument, the main code is
// checked first and then each of the associated codes
//
//     - returns the index of the code that is found
//     - returns -1 if there is not matching code
//     - the code specified must have associated codes
//     - the number of data types must match that of the code

// 2010-07-02: new function implemented
int Table::search(Code code, DataType *datatype)
{
	int i;
	int main_index = index(code);

	if (match(main_index, datatype))
	{
		return main_index;  // main code matches
	}

	for (int n = nassoc_codes(main_index); --n >= 0;)
	{
		int assoc_index = index(assoc_code(main_index, n));
		if (match(assoc_index, datatype))
		{
			return assoc_index;  // associated code matches
		}
	}
	return -1;  // no matches
}

// this function checks to see if data types specified match the data
// types of the code at the index specified
//
//    - returns true if there is match, otherwise returns false
//

// 2010-07-02: new function implemented
bool Table::match(int index, DataType *datatype)
{
	for (int n = noperands(index); --n >= 0;)
	{
		if (datatype[n] != operand_datatype(index, n))
		{
			return false;
		}
	}
	return true;
}


// end: table.cpp
