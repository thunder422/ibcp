// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: table.cpp - contains operator/command/function table
//	Copyright (C) 2010-2011  Thunder422
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
//	Change History:
//
//	2010-02-18	initial release
//
//	2010-03-01	renamed ImCommand_TokenType to ImmCmd_TokenType
//
//	2010-03-06	changed None_DataType to String_DataType for the internal string
//				functions
//
//	2010-03-10	corrected multiple entry for operators <, <=, and <>
//				(was OneChar, changed to TwoChar)
//
//	2010-03-11	split IntFunc_TokenType into IntFuncN_TokenType and
//				IntFuncP_TokenType; changed tables entries accordingly
//
//	2010-03-17	added data for new unary_code field with all set to Null_Code
//				  except for Sub_Code and Not_Code
//				added new table entries for Null_Code, Neg_Code and EOL_Code
//
//	2010-03-20	added precedence values to the table entries
//
//	2010-03-21	for Neg_Code temporarily replaced "-" with "Neq" for testing
//				and changed unary_code from Null_Code to Neq_Code
//
//	2010-03-25	added unary code for (, changed precedence values for ( & )
//
//	2010-04-02	changed precedence value in comma entry
//				corrected name for VAL( from "Val(" to "VAL("
//				for operators, name is actual (recreator) output string,
//				  name2 is debug output string (only affects Neg for now)
//
//	2010-04-04	added values for number of arguments variable
//				added entry for Asc2_Code
//				replaced Mid_Code entry Mid2_Code and Mid3_Code entries
//				replaced Instr_Code entry Instr2_Code and Instr3_Code entries
//				added Multiple_Flag for Asc_Code, Mid2_Code and Instr2_Code
//				implemented new number of arguments search function
//
//	2010-04-11	added table entries for assignment operators
//				set correct precedence for internal functions
//				removed unnecessary 0 initializers from entries
//
//	2010-04-24	added values for the number of operands to the operator entries
//				added values for new operand_datatype[] and assoc_code[] arrays
//				corrected data type for many operators and internal functions
//				added entries for the associated codes
//				moved the Null_Code to the end of the table
//				added entries for FRAC, CDBL, CvtDbl and CvtDbl
//
//	2010-05-03	began to add support for initialization of expression
//				  information structures for the table entries
//	2010-05-04	updated table entries for the expression information structures
//	2010-05-05	modified the assignment operator entries for data type handling
//				added entries for associated assignment operators
//				added Operands and AssocCode macros
//	2010-05-08	added expression information structures for unary operators
//
//	2010-05-15	modified codes that return string to return TmpStr (Chr, Repeat,
//				  Space, Str, CatStr, and StrInt) - new ExprInfos were added
//				added String_Flag to all codes that have string operands
//
//	2010-05-19	changed data type of LEFT, MID2, MID3 and RIGHT to SubStr
//				added new associated code AssignSubStr_Code to Assign_Code along
//				  with their new table entries
//	2010-05-20	added code to check if the Max_Operands and Max_Assoc_Codes
//				agree with the tables entries, reporting errors if not
//
//	2010-05-22	removed String_Flag from codes with string operands and replaced
//				  this with the automatic setting the flag by looking at the
//				  entries during table initialization
//				added new associated code AssignListMixStr along with its new
//				  table entry
//
//	2010-05-27	changed precedence values for CloseParen_Code and Comma_Code
//				  from 4 to 6 so commands are not emptied from the hold stack
//				added precedence value of 4 (and NUll_Flag) to all commands
//	2010-05-28	added value for new token_mode to Let_Code
//				added values for new token_handler to Eq_Code, CloseParen_Code,
//				  Comma_Code, and EOL_Code
//	2010-05-29	added Hidden_Flag to CvtInt_Code and CvtDbl_Code
//
//	2010-06-01	added print-only function support (Spc_Code and Tab_Code)
//				initiated PRINT command development
//	2010-06-02	added data type specific print hidden code entries
//				changed precedence and added token handler value to SemiColon
//	2010-06-05	added command handler function pointers for PRINT and assign
//				table entries
//	2010-06-06	added end expression flag to Comma, SemiColon and EOL
//	2010-06-09	swapped table entries of MID$, INSTR, and ASC so that the larger
//				number of arguments entry is first, which is necessary for the
//				new number of arguments checking at comma
//	2010-06-13	added command handler to Let entry
//	2010-06-14	removed AssignList_Flag from AssignList table entries
//
//	2010-06-25	Replaced TableError with generic Error template
//				renamed TableSearch enum to SearchType
//	2010-06-26	added end statment flag to EOL
//	2010-06-29	added expr_type value to PRINT code
//	2010-07-02	changed assign and assign list operators expression information
//				  from two operands to one - the assign operators are no longer
//				  handled by the standard operator routines
//
//	2010-07-18	added second associate code index to support checking each
//				  operand when first processed
//	2010-08-10	swapped function with multiple arguments (ASC, INSTR, MID) back
//				  where least number of argument form is first
//				validate multiple argument codes (ASC, INSTR, MID)
//	2010-12-23	automatically generate new table entry number of string
//				  arguments
//
//	2010-12-29	added Print_Flag to the print type codes
//	2011-01-05	removed expr_type from table entries, added datatype values
//	2011-01-07	moved all datatype values from exprinfo to main entry
//				updated all ExprInfo constants (and removed duplicates)
//	2011-02-03	started implementing associated entries for TmpStr type
//	2011-02-04	continue implementing TmpStr associated entries
//	2011-02-05	changed all AssocCode2 macro calls with 0 for index with the
//				  AssocCode macro call, which is the same for index = 0
//				changed table initialization to not count temporary strings
//				changed table initialization to allow assoc2_index to equal
//				  nassoc_codes so that no associated codes are searched for
//				  matching the second operand if not desired (for assigns)
//				changed assign string code entries to have their own expression
//				  info instances because the first string operand does not get
//				  get counted for the number of strings
//				modified table initialization to not could the first string
//				  operand if reference flag is set (assignment operators)
//
//	2011-02-26	removed code from first memory in table entries to comment at
//				  beginning of entry, which is used by codes.awk to generated
//				  codes.h containing the Code enumeration
//				moved Null_Code entry from end to beginning so that Null_Code
//				  enumeration value will be zero
//				removed table index/code duplicate and missing checking (not
//				  necessary since codes are new generated directly from table)
//				added bracketing around immediate commands
//				updated search() functions to return Code enumeration value
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

// 2010-07-18: modified to set assoc2code to zero
#define AssocCode(code)  (sizeof(code ## _AssocCode) \
	/ sizeof(code ## _AssocCode[0])), 0, code ## _AssocCode

// 2010-07-18: new macro with second operand assoc code index
#define AssocCode2(code, index)  (sizeof(code ## _AssocCode) \
	/ sizeof(code ## _AssocCode[0])), index, code ## _AssocCode


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
DataType IntDbl_OperandArray[] = {
	Integer_DataType, Double_DataType
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
DataType StrTmp_OperandArray[] = {
	String_DataType, TmpStr_DataType
};
DataType StrStrInt_OperandArray[] = {
	String_DataType, String_DataType, Integer_DataType
};
DataType StrTmpInt_OperandArray[] = {
	String_DataType, TmpStr_DataType, Integer_DataType
};
DataType Sub_OperandArray[] = {  // 2010-07-01
	SubStr_DataType
};
// 2010-05-19: added new operand data type array
DataType SubStr_OperandArray[] = {
	SubStr_DataType, String_DataType
};
DataType SubTmp_OperandArray[] = {
	SubStr_DataType, TmpStr_DataType
};

DataType Tmp_OperandArray[] = {
	TmpStr_DataType
};
DataType TmpInt_OperandArray[] = {
	TmpStr_DataType, Integer_DataType
};
DataType TmpStr_OperandArray[] = {
	TmpStr_DataType, String_DataType
};
DataType TmpStrInt_OperandArray[] = {
	TmpStr_DataType, String_DataType, Integer_DataType
};
DataType TmpTmp_OperandArray[] = {
	TmpStr_DataType, TmpStr_DataType
};
DataType TmpTmpInt_OperandArray[] = {
	TmpStr_DataType, TmpStr_DataType, Integer_DataType
};


// 2010-05-06: associated code data type arrays
Code Abs_AssocCode[]			= {AbsInt_Code};
Code Add_AssocCode[]			= {
	AddI1_Code, CatStr_Code, CatStrT1_Code, AddI2_Code
};
Code AddI1_AssocCode[]			= {AddInt_Code};
Code Asc_AssocCode[]			= {AscTmp_Code};
Code Asc2_AssocCode[]			= {Asc2Tmp_Code};
// 2010-05-19: added AssignSubStr_Code to list
Code Assign_AssocCode[]			= {
	AssignInt_Code, AssignStr_Code, AssignSubStr_Code
};
Code AssignStr_AssocCode[]		= {AssignTmp_Code};
Code AssignSubStr_AssocCode[]	= {AssignSubTmp_Code};
// 2010-05-19: added AssignListMaxStr_Code to list
Code AssignList_AssocCode[] = {
	AssignListInt_Code, AssignListStr_Code, AssignListMix_Code
};
Code AssignListStr_AssocCode[]	= {AssignListTmp_Code};
Code AssignListMix_AssocCode[]	= {AssignListMixTmp_Code};
Code CatStr_AssocCode[]			= {CatStrT2_Code};
Code CatStrT1_AssocCode[]		= {CatStrTT_Code};
Code Div_AssocCode[]			= {DivI1_Code, DivI2_Code};
Code DivI1_AssocCode[]			= {DivInt_Code};
Code Eq_AssocCode[]				= {
	EqI1_Code, EqStr_Code, EqStrT1_Code, EqI2_Code
};
Code EqI1_AssocCode[]			= {EqInt_Code};
Code EqStr_AssocCode[]			= {EqStrT2_Code};
Code EqStrT1_AssocCode[]		= {EqStrTT_Code};
Code Gt_AssocCode[]				= {
	GtI1_Code, GtStr_Code, GtStrT1_Code, GtI2_Code
};
Code GtI1_AssocCode[]			= {GtInt_Code};
Code GtStr_AssocCode[]			= {GtStrT2_Code};
Code GtStrT1_AssocCode[]		= {GtStrTT_Code};
Code GtEq_AssocCode[]			= {
	GtEqI1_Code, GtEqStr_Code, GtEqStrT1_Code, GtEqI2_Code
};
Code GtEqI1_AssocCode[]			= {GtEqInt_Code};
Code GtEqStr_AssocCode[]		= {GtEqStrT2_Code};
Code GtEqStrT1_AssocCode[]		= {GtEqStrTT_Code};
Code Instr2_AssocCode[]			= {Instr2T1_Code, Instr2T2_Code};
Code Instr2T1_AssocCode[]		= {Instr2TT_Code};
Code Instr3_AssocCode[]			= {Instr3T1_Code, Instr3T2_Code};
Code Instr3T1_AssocCode[]		= {Instr3TT_Code};
Code Len_AssocCode[]			= {LenTmp_Code};
Code Lt_AssocCode[]				= {
	LtI1_Code, LtStr_Code, LtStrT1_Code, LtI2_Code
};
Code LtI1_AssocCode[]			= {LtInt_Code};
Code LtStr_AssocCode[]			= {LtStrT2_Code};
Code LtStrT1_AssocCode[]		= {LtStrTT_Code};
Code LtEq_AssocCode[]			= {
	LtEqI1_Code, LtEqStr_Code, LtEqStrT1_Code, LtEqI2_Code
};
Code LtEqI1_AssocCode[]			= {LtEqInt_Code};
Code LtEqStr_AssocCode[]		= {LtEqStrT2_Code};
Code LtEqStrT1_AssocCode[]		= {LtEqStrTT_Code};
Code Mod_AssocCode[]			= {ModI1_Code, ModI2_Code};
Code ModI1_AssocCode[]			= {ModInt_Code};
Code Mul_AssocCode[]			= {MulI1_Code, MulI2_Code};
Code MulI1_AssocCode[]			= {MulInt_Code};
Code Neg_AssocCode[]			= {NegInt_Code};
Code NotEq_AssocCode[]			= {
	NotEqI1_Code, NotEqStr_Code, NotEqStrT1_Code, NotEqI2_Code
};
Code NotEqI1_AssocCode[]    	= {NotEqInt_Code};
Code NotEqStr_AssocCode[]		= {NotEqStrT2_Code};
Code NotEqStrT1_AssocCode[]		= {NotEqStrTT_Code};
Code Power_AssocCode[]			= {PowerI1_Code, PowerMul_Code};
Code PowerI1_AssocCode[]		= {PowerInt_Code};
// 2010-06-02: added associated codes for data type specific print
Code Print_AssocCode[]			= {PrintInt_Code, PrintStr_Code, PrintTmp_Code};
Code Repeat_AssocCode[]			= {RepeatTmp_Code};
Code RndArgs_AssocCode[]		= {RndArgInt_Code};
Code Sgn_AssocCode[]			= {SgnInt_Code};
Code Str_AssocCode[]			= {StrInt_Code};
Code Sub_AssocCode[]			= {SubI1_Code, SubI2_Code};
Code SubI1_AssocCode[]			= {SubInt_Code};
Code Val_AssocCode[]			= {ValTmp_Code};


// 2010-05-06: standard expression information structures
// 2011-01-07: removed return datatype, removed resulting duplicates
ExprInfo Dbl_ExprInfo(Null_Code, Operands(Dbl));
ExprInfo DblDbl_ExprInfo(Null_Code, Operands(DblDbl));
ExprInfo DblInt_ExprInfo(Null_Code, Operands(DblInt));

ExprInfo Int_ExprInfo(Null_Code, Operands(Int));
ExprInfo IntInt_ExprInfo(Null_Code, Operands(IntInt));

ExprInfo Str_ExprInfo(Null_Code, Operands(Str));
ExprInfo StrInt_ExprInfo(Null_Code, Operands(StrInt));
ExprInfo StrStr_ExprInfo(Null_Code, Operands(StrStr));  //?
ExprInfo StrStrInt_ExprInfo(Null_Code, Operands(StrStrInt));
ExprInfo StrTmp_ExprInfo(Null_Code, Operands(StrTmp));
// special copy for assign temporary string operators (2011-02-05)
ExprInfo Assign_StrTmp_ExprInfo(Null_Code, Operands(StrTmp));
ExprInfo StrTmpInt_ExprInfo(Null_Code, Operands(StrTmpInt));
ExprInfo Assign_SubTmp_ExprInfo(Null_Code, Operands(SubTmp));

ExprInfo Tmp_ExprInfo(Null_Code, Operands(Tmp));
ExprInfo TmpInt_ExprInfo(Null_Code, Operands(TmpInt));
ExprInfo TmpStr_ExprInfo(Null_Code, Operands(TmpStr));
ExprInfo TmpStrInt_ExprInfo(Null_Code, Operands(TmpStrInt));
ExprInfo TmpTmp_ExprInfo(Null_Code, Operands(TmpTmp));
ExprInfo TmpTmpInt_ExprInfo(Null_Code, Operands(TmpTmpInt));

// 2010-05-19: changed to return SubStr
ExprInfo StrIntInt_ExprInfo(Null_Code, Operands(StrIntInt));
ExprInfo Sub_ExprInfo(Null_Code, Operands(Sub));

// 2011-02-26: moved code from first member to comment
static TableEntry table_entries[] =
{
	// 2011-02-26: moved Null_Code entry to beginning (so Null_Code == 0)
	{	// Null_Code
		Operator_TokenType
	},
	//******************************
	//   IMMEDIATE COMMANDS FIRST
	//******************************
	{	// BegImmCmd_Code
		Error_TokenType
	},
	// (these will go away once gui interface is implemented)
	{	// List_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"L", NULL, Blank_Flag | Line_Flag | Range_Flag
	},
	{	// Edit_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"E", NULL, Blank_Flag | Line_Flag
	},
	{	// Delete_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"D", NULL, Line_Flag | Range_Flag
	},
	{	// Run_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"R", NULL, Blank_Flag
	},
	{	// Renum_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"R", NULL, Range_Flag | RangeIncr_Flag
	},
	{	// Save_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"S", NULL, Blank_Flag | String_Flag
	},
	{	// Load_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"L", NULL, String_Flag
	},
	{	// New_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"N", NULL, Blank_Flag
	},
	{	// Auto_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"A", NULL, Blank_Flag | Line_Flag | LineIncr_Flag
	},
	{	// Cont_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"C", NULL, Blank_Flag
	},
	{	// Quit_Code
		ImmCmd_TokenType, OneWord_Multiple,
		"Q", NULL, Blank_Flag
	},
	{	// EndImmCmd_Code
		Error_TokenType
	},
	// end of immediate commands marked by NULL name (next entry)
	//***********************
	//   BEGIN PLAIN WORDS
	//***********************
	{	// BegPlainWord_Code
		Error_TokenType
	},
	//--------------
	//   Commands
	//--------------
	{	// Let_Code
		// 2010-05-28: added value for token_mode
		// 2010-06-13: added value for command handler
		Command_TokenType, OneWord_Multiple,
		"LET", NULL, Null_Flag, 4, None_DataType, NULL, NULL,
		Assignment_TokenMode, Let_CmdHandler
	},
	{	// Print_Code
		// 2010-06-01: added value for token_mode
		// 2010-06-05: added value for command handler
		// 2010-06-29: added value for expr_type
		Command_TokenType, OneWord_Multiple,
		"PRINT", NULL, Null_Flag, 4, None_DataType, NULL, NULL,
		Expression_TokenMode, Print_CmdHandler  //Any_ExprType
	},
	{	// Input_Code
		Command_TokenType, OneWord_Multiple,
		"INPUT", NULL, Null_Flag, 4, None_DataType
	},
	{	// Dim_Code
		Command_TokenType, OneWord_Multiple,
		"DIM", NULL, Null_Flag, 4, None_DataType
	},
	{	// Def_Code
		Command_TokenType, OneWord_Multiple,
		"DEF", NULL, Null_Flag, 4, None_DataType
	},
	{	// Rem_Code
		Command_TokenType, OneWord_Multiple,
		"REM", NULL, Null_Flag, 4, None_DataType
	},
	{	// If_Code
		Command_TokenType, OneWord_Multiple,
		"IF", NULL, Null_Flag, 4, None_DataType
	},
	{	// Then_Code
		Command_TokenType, OneWord_Multiple,
		"THEN", NULL, Null_Flag, 4, None_DataType
	},
	{	// Else_Code
		Command_TokenType, OneWord_Multiple,
		"ELSE", NULL, Null_Flag, 4, None_DataType
	},
	{	// End_Code
		Command_TokenType, TwoWord_Multiple,
		"END", NULL, Null_Flag, 4, None_DataType
	},
	{	// EndIf_Code
		Command_TokenType, TwoWord_Multiple,
		"END", "IF", Null_Flag, 4, None_DataType
	},
	{	// For_Code
		Command_TokenType, OneWord_Multiple,
		"FOR", NULL, Null_Flag, 4, None_DataType
	},
	{	// To_Code
		Command_TokenType, OneWord_Multiple,
		"TO", NULL, Null_Flag, 4, None_DataType
	},
	{	// Step_Code
		Command_TokenType, OneWord_Multiple,
		"STEP", NULL, Null_Flag, 4, None_DataType
	},
	{	// Next_Code
		Command_TokenType, OneWord_Multiple,
		"NEXT", NULL, Null_Flag, 4, None_DataType
	},
	{	// Do_Code
		Command_TokenType, TwoWord_Multiple,
		"Do", NULL, Null_Flag, 4, None_DataType
	},
	{	// DoWhile_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", "WHILE", Null_Flag, 4, None_DataType
	},
	{	// DoUntil_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", "UNTIL", Null_Flag, 4, None_DataType
	},
	{	// While_Code
		Command_TokenType, TwoWord_Multiple,
		"WHILE", NULL, Null_Flag, 4, None_DataType
	},
	{	// Until_Code
		Command_TokenType, TwoWord_Multiple,
		"UNTIL", NULL, Null_Flag, 4, None_DataType
	},
	{	// Loop_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", NULL, Null_Flag, 4, None_DataType
	},
	{	// LoopWhile_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", "WHILE", Null_Flag, 4, None_DataType
	},
	{	// LoopUntil_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", "UNTIL", Null_Flag, 4, None_DataType
	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{	// Rnd_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"RND", NULL, Null_Flag, 2, Double_DataType, new ExprInfo()

	},
	//--------------------
	//   Word Operators
	//--------------------
	{	// Mod_Code
		Operator_TokenType, OneWord_Multiple,
		"MOD", NULL, Null_Flag, 42, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Mod, 1))
	},
	{	// And_Code
		Operator_TokenType, OneWord_Multiple,
		"AND", NULL, Null_Flag, 18, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Or_Code
		Operator_TokenType, OneWord_Multiple,
		"OR", NULL, Null_Flag, 14, Integer_DataType, &IntInt_ExprInfo
	},
	// 2010-05-08: Not needs its own exprinfo struct
	{	// Not_Code
		Operator_TokenType, OneWord_Multiple,
		"NOT", NULL, Null_Flag, 20, Integer_DataType,
		new ExprInfo(Not_Code, Operands(Int))
	},
	{	// Eqv_Code
		Operator_TokenType, OneWord_Multiple,
		"EQV", NULL, Null_Flag, 12, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Imp_Code
		Operator_TokenType, OneWord_Multiple,
		"IMP", NULL, Null_Flag, 10, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Xor_Code
		Operator_TokenType, OneWord_Multiple,
		"XOR", NULL, Null_Flag, 16, Integer_DataType, &IntInt_ExprInfo
	},
	//*********************
	//   END PLAIN WORDS
	//*********************
	{	// EndPlainWord_Code
		Error_TokenType
	},
	//*****************************
	//   BEGIN PARENTHESES WORDS
	//*****************************
	{	// BegParenWord_Code
		Error_TokenType
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{	// Abs_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ABS(", NULL, Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Abs))
	},
	{	// Fix_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"FIX(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Frac_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"FRAC(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Int_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INT(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo

	},
	{	// RndArg_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RND(", NULL, Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(RndArgs))
	},
	{	// Sgn_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", NULL, Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Sgn))
	},
	{	// Cint_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CINT(", NULL, Null_Flag, 2, Integer_DataType, &Dbl_ExprInfo
	},
	{	// Cdbl_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CDBL(", NULL, Null_Flag, 2, Double_DataType, &Int_ExprInfo
	},
	{	// Sqr_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SQR(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Atn_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ATN(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Cos_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"COS(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Sin_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SIN(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Tan_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"TAN(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Exp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"EXP(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Log_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LOG(", NULL, Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Tab_Code
		// 2010-06-01: added print-only flag
		IntFuncP_TokenType, OneWord_Multiple,
		"TAB(", NULL, Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// Spc_Code
		// 2010-06-01: added print-only flag
		IntFuncP_TokenType, OneWord_Multiple,
		"SPC(", NULL, Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	// 2010-04-04: added entry for 2 argument ASC
	// 2010-06-09: swapped ASC and ASC2 entries
	// 2010-08-10: swapped ASC and ASC2 entries back
	{	// Asc_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", NULL, Multiple_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(Str), AssocCode(Asc))
	},
	{	// Asc2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASC2(", Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrInt), AssocCode(Asc2))
	},
	{	// Chr_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CHR$(", NULL, Null_Flag, 2, TmpStr_DataType, &Int_ExprInfo
	},
	// 2010-04-04: replaced INSTR entry with INSTR2 and INSTR3 entries
	// 2010-06-09: swapped INSTR2 and INSTR3 entries
	// 2010-08-10: swapped INSTR2 and INSTR3 entries back
	{	// Instr2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2(", Multiple_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode2(Instr2, 1))
	},
	{	// Instr3_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3(", Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStrInt), AssocCode2(Instr3, 1))
	},
	{	// Left_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEFT$(", NULL, Null_Flag, 2, SubStr_DataType, &StrInt_ExprInfo
	},
	{	// Len_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEN(", NULL, Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(Str), AssocCode(Len))
	},
	// 2010-04-04: replaced MID entry with MID2 and MID3 entries
	// 2010-06-09: swapped MID2 and MID3 entries
	// 2010-08-10: swapped MID2 and MID3 entries back
	{	// Mid2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID2$(", Multiple_Flag, 2, SubStr_DataType, &StrInt_ExprInfo
	},
	{	// Mid3_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID3$(", Null_Flag, 2, SubStr_DataType, &StrIntInt_ExprInfo
	},
	{	// Repeat_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"REPEAT$(", NULL, Null_Flag, 2, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(StrInt), AssocCode(Repeat))
	},
	{	// Right_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RIGHT$(", NULL, Null_Flag, 2, SubStr_DataType, &StrInt_ExprInfo
	},
	{	// Space_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SPACE$(", NULL, Null_Flag, 2, TmpStr_DataType, &Int_ExprInfo
	},
	{	// Str_Code
		// 2010-05-15: changed to return TmpStr
		IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", NULL, Null_Flag, 2, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Str))
	},
	{	// Val_Code
		// 2010-04-02: changed name to all upper case
		IntFuncP_TokenType, OneWord_Multiple,
		"VAL(", NULL, Null_Flag, 2, Double_DataType, &Str_ExprInfo
	},
	//***************************
	//   END PARENTHESES WORDS
	//***************************
	{	// EndParenWord_Code
		Error_TokenType
	},
	//***************************
	//   BEGIN DATA TYPE WORDS
	//***************************
	{	// BegDataTypeWord_Code
		Error_TokenType
	},
	// Currently None

	//*************************
	//   END DATA TYPE WORDS
	//*************************
	{	// EndDataTypeWord_Code
		Error_TokenType
	},
	//*******************
	//   BEGIN SYMBOLS
	//*******************
	{	// BegSymbol_Code
		Error_TokenType
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{	// Add_Code
		Operator_TokenType, OneChar_Multiple,
		"+", NULL, Null_Flag, 40, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Add, 3))
	},
	{	// Sub_Code
		Operator_TokenType, OneChar_Multiple,
		"-", NULL, Null_Flag, 40, Double_DataType,
		new ExprInfo(Neg_Code, Operands(DblDbl), AssocCode2(Sub, 1))
	},
	{	// Mul_Code
		Operator_TokenType, OneChar_Multiple,
		"*", NULL, Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Mul, 1))
	},
	{	// Div_Code
		Operator_TokenType, OneChar_Multiple,
		"/", NULL, Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Div, 1))
	},
	{	// IntDiv_Code
		Operator_TokenType, OneChar_Multiple,
		"\\", NULL, Null_Flag, 44, Integer_DataType, &DblDbl_ExprInfo
	},
	{	// Power_Code
		Operator_TokenType, OneChar_Multiple,
		"^", NULL, Null_Flag, 50, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Power, 1))
	},
	{	// Eq_Code
		// 2010-05-28: added value for token_handler
		Operator_TokenType, OneChar_Multiple,
		"=", NULL, Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Eq, 3)),
		Equal_Handler
	},
	{	// Gt_Code
		Operator_TokenType, TwoChar_Multiple,
		">", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Gt, 3))
	},
	{	// GtEq_Code
		Operator_TokenType, TwoChar_Multiple,
		">=", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(GtEq, 3))
	},
	{	// Lt_Code
		Operator_TokenType, TwoChar_Multiple,
		"<", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Lt, 3))
	},
	{	// LtEq_Code
		Operator_TokenType, TwoChar_Multiple,
		"<=", NULL, Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(LtEq, 3))
	},
	{	// NotEq_Code
		Operator_TokenType, TwoChar_Multiple,
		"<>", NULL, Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(NotEq, 3))
	},
	{	// OpenParen_Code
		// 2010-03-25: set unary code, changed precedence value
		Operator_TokenType, OneChar_Multiple,
		"(", NULL, Null_Flag, 2, None_DataType,
		new ExprInfo(OpenParen_Code)
	},
	{	// CloseParen_Code
		// 2010-03-25: changed precedence value from 0 to 4
		// 2010-05-27: changed precedence value from 4 to 6
		// 2010-05-28: added value for token_handler
		Operator_TokenType, OneChar_Multiple,
		")", NULL, Null_Flag, 4, None_DataType, NULL,
		CloseParen_Handler
	},
	{	// Comma_Code
		// 2010-04-02: changed precedence value from 0 to 4
		// 2010-05-27: changed precedence value from 4 to 6
		// 2010-05-28: added value for token_handler
		// 2010-06-06: added end expression flag
		Operator_TokenType, OneChar_Multiple,
		",", NULL, EndExpr_Flag, 6, None_DataType,
		NULL,//new ExprInfo(Comma_Code),
		Comma_Handler
	},
	{	// SemiColon_Code
		// 2010-06-02: changed precedence from 0, added token_handler value
		// 2010-06-06: added end expression flag
		Operator_TokenType, OneChar_Multiple,
		";", NULL, EndExpr_Flag, 6, None_DataType,
		NULL,//new ExprInfo(SemiColon_Code),
		SemiColon_Handler
	},
	{	// Colon_Code
		Operator_TokenType, OneChar_Multiple,
		":", NULL, Null_Flag, 0, None_DataType
	},
	{	// RemOp_Code
		Operator_TokenType, OneChar_Multiple,
		"'", NULL, Null_Flag, 0, None_DataType
	},
	//*****************
	//   END SYMBOLS
	//*****************
	{	// EndSymbol_Code
		Error_TokenType
	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	// 2010-03-16: added entries for new codes
	// 2010-04-24: moved null code entry to end
	{	// Neg_Code
		Operator_TokenType, OneWord_Multiple,
		// 2010-03-21: temporarily replaced "-" with "Neq" for testing
		// 2010-03-21: changed unary_code from Null_Code to Neg_Code
		// 2010-04-02: set name to correct output, name2 to debug output name
		"-", "Neg", Null_Flag, 48, Double_DataType,
		new ExprInfo(Neg_Code, Operands(Dbl), AssocCode(Neg))
	},
	// 2010-04-11: added entries for assignment operators
	// 2010-05-05: added reference flag to assignment operators
	// 2010-06-05: added command handler function pointers to all assign entries
	// 2010-07-02: changed all assign operators from 2 to 1 operands
	{	// Assign_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign", Reference_Flag, 4, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Assign, 3)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-05: added entries for assignment associated codes
	{	// AssignInt_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign%", Reference_Flag, 4, Integer_DataType, &IntInt_ExprInfo,
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign$", Reference_Flag | String_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignStr)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign$T", Reference_Flag | String_Flag, 4, TmpStr_DataType,
		&Assign_StrTmp_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-19: added entries for assign sub-string associated code
	{	// AssignSubStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignSub$", Reference_Flag | String_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(SubStr), AssocCode(AssignSubStr)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignSubTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignSub$T", Reference_Flag | String_Flag, 4, TmpStr_DataType,
		&Assign_SubTmp_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-05: added reference and assign list flags
	// 2010-06-14: removed assign list flags
	// 2010-07-02: changed all assign list operators from 2 to 1 operands
	{	// AssignList_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList", Reference_Flag, 4, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(AssignList, 3)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-05: added entries for assignment associated codes
	{	// AssignListInt_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList%", Reference_Flag, 4, Integer_DataType,
		&IntInt_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList$", Reference_Flag | String_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignListStr)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList$T", Reference_Flag | String_Flag, 4, TmpStr_DataType,
		&Assign_StrTmp_ExprInfo, NULL, Null_TokenMode, Assign_CmdHandler
	},
	// 2010-05-22: added entry for assign mix string list associated code
	{	// AssignListMix_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignListMix$", Reference_Flag | String_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(SubStr), AssocCode(AssignListMix)),
		NULL, Null_TokenMode, Assign_CmdHandler
	},
	{	// AssignListMixTmp_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignListMix$T", Reference_Flag | String_Flag, 4,
		TmpStr_DataType, &Assign_SubTmp_ExprInfo, NULL, Null_TokenMode,
		Assign_CmdHandler
	},
	{	// EOL_Code
		// 2010-05-28: added value for token_handler
		// 2010-06-06: added end expression flag
		// 2010-06-26: added end statment flag
		Operator_TokenType, OneWord_Multiple,
		NULL, NULL, EndExpr_Flag | EndStatement_Flag, 4, None_DataType, NULL,
		EndOfLine_Handler
	},
	// 2011-02-04: added entries for associated string functions
	{	// AscTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASCT(", Multiple_Flag, 2, Integer_DataType, &Tmp_ExprInfo
	},
	{	// Asc2Tmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASC2T(", Null_Flag, 2, Integer_DataType, &TmpInt_ExprInfo
	},
	{	// Instr2T1_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2T1(", Multiple_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(Instr2T1))
	},
	{	// Instr3T1_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3T1(", Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStrInt), AssocCode(Instr3T1))
	},
	{	// Instr2T2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2T2(", Multiple_Flag, 2, Integer_DataType,
		&StrTmp_ExprInfo
	},
	{	// Instr3T2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3T2(", Null_Flag, 2, Integer_DataType,
		&StrTmpInt_ExprInfo
	},
	{	// Instr2TT_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2TT(", Multiple_Flag, 2, Integer_DataType,
		&TmpTmp_ExprInfo
	},
	{	// Instr3TT_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3TT(", Null_Flag, 2, Integer_DataType,
		&TmpTmpInt_ExprInfo
	},
	{	// LenTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEN(", "LENT(", Null_Flag, 2, Integer_DataType, &Tmp_ExprInfo
	},
	{	// RepeatTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"REPEAT$(", "REPEAT$T(", Null_Flag, 2, TmpStr_DataType, &TmpInt_ExprInfo
	},
	{	// ValTmp_Code
		// 2010-04-02: changed name to all upper case
		IntFuncP_TokenType, OneWord_Multiple,
		"VAL(", "VALT(", Null_Flag, 2, Double_DataType, &Str_ExprInfo
	},
	// 2010-04-24: added entries for associated codes
	// 2010-07-17: added secondary operand entries for associated codes
	{	// AddI1_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%1", Null_Flag, 40, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(AddI1))
	},
	{	// AddI2_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%2", Null_Flag, 40, Double_DataType, &DblInt_ExprInfo
	},
	{	// AddInt_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%", Null_Flag, 40, Integer_DataType, &IntInt_ExprInfo
	},
	{	// CatStr_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$", Null_Flag, 40, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(CatStr))
	},
	{	// CatStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$1", Null_Flag, 40, TmpStr_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(CatStrT1))
	},
	{	// CatStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$2", Null_Flag, 40, TmpStr_DataType, &StrTmp_ExprInfo
	},
	{	// CatStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$T", Null_Flag, 40, TmpStr_DataType, &TmpTmp_ExprInfo
	},
	{	// SubI1_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%1", Null_Flag, 40, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(SubI1))
	},
	{	// SubI2_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%2", Null_Flag, 40, Double_DataType, &DblInt_ExprInfo
	},
	{	// SubInt_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%", Null_Flag, 40, Integer_DataType, &IntInt_ExprInfo
	},
	// 2010-05-08: NegInt needs its own exprinfo struct
	{	// NegInt_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "Neg%", Null_Flag, 40, Integer_DataType,
		new ExprInfo(NegInt_Code, Operands(Int))
	},
	{	// MulI1_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%1", Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(MulI1))
	},
	{	// MulI2_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%2", Null_Flag, 46, Double_DataType, &DblInt_ExprInfo
	},
	{	// MulInt_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%", Null_Flag, 46, Integer_DataType, &IntInt_ExprInfo
	},
	{	// DivI1_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%1", Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(DivI1))
	},
	{	// DivI2_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%2", Null_Flag, 46, Double_DataType, &DblInt_ExprInfo
	},
	{	// DivInt_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%", Null_Flag, 46, Integer_DataType, &IntInt_ExprInfo
	},
	{	// ModI1_Code
		Operator_TokenType, OneChar_Multiple,
		"MOD", "MOD%1", Null_Flag, 42, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(ModI1))
	},
	{	// ModI2_Code
		Operator_TokenType, OneChar_Multiple,
		"MOD", "MOD%2", Null_Flag, 42, Double_DataType, &DblInt_ExprInfo
	},
	{	// ModInt_Code
		Operator_TokenType, OneWord_Multiple,
		"MOD", "MOD%", Null_Flag, 42, Integer_DataType, &IntInt_ExprInfo
	},
	{	// PowerI1_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^%1", Null_Flag, 50, Double_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(PowerI1))
	},
	{	// PowerMul_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^*", Null_Flag, 50, Double_DataType, &DblInt_ExprInfo
	},
	{	// PowerInt_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^%", Null_Flag, 50, Integer_DataType, &IntInt_ExprInfo
	},
	{	// EqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(EqI1))
	},
	{	// EqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%2", Null_Flag, 30, Integer_DataType, &DblInt_ExprInfo
	},
	{	// EqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%", Null_Flag, 30, Integer_DataType, &IntInt_ExprInfo
	},
	{	// EqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(EqStr))
	},
	{	// EqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(EqStrT1))
	},
	{	// EqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$2", Null_Flag, 30, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// EqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$T", Null_Flag, 30, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// GtI1_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(GtI1))
	},
	{	// GtI2_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// GtInt_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// GtStr_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(GtStr))
	},
	{	// GtStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(GtStrT1))
	},
	{	// GtStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// GtStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// GtEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(GtEqI1))
	},
	{	// GtEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// GtEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// GtEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(GtEqStr))
	},
	{	// GtEqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(GtEqStrT1))
	},
	{	// GtEqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// GtEqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// LtI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(LtI1))
	},
	{	// LtI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// LtInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// LtStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(LtStr))
	},
	{	// LtStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(LtStrT1))
	},
	{	// LtStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// LtStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// LtEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(LtEqI1))
	},
	{	// LtEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%2", Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// LtEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%", Null_Flag, 32, Integer_DataType, &IntInt_ExprInfo
	},
	{	// LtEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(LtEqStr))
	},
	{	// LtEqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$1", Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(LtEqStrT1))
	},
	{	// LtEqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$2", Null_Flag, 32, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// LtEqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$T", Null_Flag, 32, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// NotEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntDbl), AssocCode(NotEqI1))
	},
	{	// NotEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%2", Null_Flag, 30, Integer_DataType, &DblInt_ExprInfo
	},
	{	// NotEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%", Null_Flag, 30, Integer_DataType, &IntInt_ExprInfo
	},
	{	// NotEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(NotEqStr))
	},
	{	// NotEqStrT1_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$1", Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(TmpStr), AssocCode(NotEqStrT1))
	},
	{	// NotEqStrT2_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$2", Null_Flag, 30, Integer_DataType, &StrTmp_ExprInfo
	},
	{	// NotEqStrTT_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$T", Null_Flag, 30, Integer_DataType, &TmpTmp_ExprInfo
	},
	{	// AbsInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ABS(", "ABS%(", Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// RndArgInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RND(", "RND%(", Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// SgnInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", "SGN%(", Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// CvtInt_Code
		// 2010-05-29: added Hidden_Flag
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtInt", Hidden_Flag, 2, None_DataType
	},
	{	// CvtDbl_Code
		// 2010-05-29: added Hidden_Flag
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtDbl", Hidden_Flag, 2, None_DataType
	},
	{	// StrInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", "STR%$(", Null_Flag, 2, TmpStr_DataType, &Int_ExprInfo
	},
	// 2010-06-02: added hidden print codes (main plus associated codes)
	// 2010-12-29: added Print_Flag to these codes
	{	// PrintDbl_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"", "PrintDbl", Print_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Print))
	},
	{	// PrintInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"", "PrintInt", Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// PrintStr_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"", "PrintStr", Print_Flag | String_Flag, 2, None_DataType,
		&Str_ExprInfo
	},
	{	// PrintTmp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"", "PrintTmp", Print_Flag | String_Flag, 2, None_DataType,
		&Tmp_ExprInfo
	}
};


// constructor function that initializes the table variables

Table::Table(void)
{
	List<Error<Code> > *error_list = new List<Error<Code> >;
	int i;
	int type;

	entry = table_entries;

	// 2011-02-26: removed index_code[] initialization

	// scan table and record indexes
	bool error = false;
	int nentries = sizeof(table_entries) / sizeof(TableEntry);
	// 2010-05-20: find maximum number of operands and associated codes
	int max_operands = 0;
	int max_assoc_codes = 0;
	for (i = 0; i < nentries; i++)
	{
		// 2011-02-26: removed duplicate checking

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

			// 2010-07-18: check in assoc2_index is valid
			// 2011-02-05: allow assoc2_index to equal nassoc_codes
			if (exprinfo->assoc2_index > 0
				&& exprinfo->assoc2_index > exprinfo->nassoc_codes)
			{
				// Assoc2Code_ErrorType
				Error<Code> error(i, exprinfo->assoc2_index,
					exprinfo->nassoc_codes);
				error_list->append(&error);
			}

			// 2010-12-23: generate number of string arguments value
			exprinfo->nstrings = 0;
			for (int j = 0; j < exprinfo->noperands; j++)
			{
				// 2011-02-05: don't count temporary strings
				// 2011-02-05: don't count first string of assignment operators
				if (exprinfo->operand_datatype[j] == String_DataType
					&& (j > 0 || !(entry[i].flags & Reference_Flag)))
				{
					exprinfo->nstrings++;
				}
			}
			// 2010-05-22: set String_Flag in entries automatically
			if (exprinfo->nstrings > 0)
			{
				entry[i].flags |= String_Flag;
			}

			// 2010-08-10: validate multiple entries
			if (entry[i].flags & Multiple_Flag != 0)
			{
				ExprInfo *exprinfo2 = entry[i + 1].exprinfo;
				if (strcmp(entry[i].name, entry[i + 1].name) != 0)
				{
					// MultName_ErrorType
					Error<Code> error(entry[i].name, entry[i + 1].name);
					error_list->append(&error);
				}
				else if (exprinfo2 == NULL)
				{
					// MultExprInfo_ErrorType
					Error<Code> error(entry[i + 1].name);
					error_list->append(&error);
				}
				else if (exprinfo2->noperands != exprinfo->noperands + 1)
				{
					// MultNOperands_ErrorType
					Error<Code> error(entry[i].name, exprinfo->noperands,
						exprinfo2->noperands);
					error_list->append(&error);
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

	// 2011-02-26: removed missing checking

	// setup indexes for bracketing codes
	// (will be set to -1 if missing - missing errors were recorded above)
	range[ImmCmd_SearchType].beg = BegImmCmd_Code;
	range[ImmCmd_SearchType].end = EndImmCmd_Code;
	range[PlainWord_SearchType].beg = BegPlainWord_Code;
	range[PlainWord_SearchType].end = EndPlainWord_Code;
	range[ParenWord_SearchType].beg = BegParenWord_Code;
	range[ParenWord_SearchType].end = EndParenWord_Code;
	range[DataTypeWord_SearchType].beg = BegDataTypeWord_Code;
	range[DataTypeWord_SearchType].end = EndDataTypeWord_Code;
	range[Symbol_SearchType].beg = BegSymbol_Code;
	range[Symbol_SearchType].end = EndSymbol_Code;

	// check for missing bracketing codes and if properly positioned in table
	// (missing codes recorded above, however,
	// need to make sure all types were set, i.e. no missing assignments above)
	for (type = 0; type < sizeof_SearchType; type++)
	{
		if (range[type].beg > range[type].end)
		{
			// record bracket range error Range_ErrorType
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
					&& (range[type].beg > range[type2].beg
					&& range[type].beg < range[type2].end
					|| range[type].end > range[type2].beg
					&& range[type].end < range[type2].end))
				{
					// record bracket overlap error Overlap_ErrorType
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

Code Table::search(char letter, int flag)
{
	for (Code i = BegImmCmd_Code + 1; i < EndImmCmd_Code; i++)
	{
		if (toupper(letter) == entry[i].name[0]
			&& (flag == Null_Flag || flag & entry[i].flags))
		{
			return i;
		}
	}
	return Invalid_Code;  // not found
}


// this search function will look for a string of a particular type in
// the Table, the search is case insensitive
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

Code Table::search(SearchType type, const char *string, int len)
{
	Code i = range[type].beg;
	Code end = range[type].end;
	while (++i < end)
	{
		if (strncasecmp(string, entry[i].name, len) == 0
			&& entry[i].name[len] == '\0')
		{
			return i;
		}
	}
	return Invalid_Code;
}


// this search function will look for a two word command in the Table,
// the search is case insensitive an only entries containing a second
// word is checked
//
//     - returns the index of the entry that is found
//     - returns -1 if the string was not found in the table

Code Table::search(const char *word1, int len1, const char *word2, int len2)
{
	for (Code i = range[PlainWord_SearchType].beg;
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
	return Invalid_Code;
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
Code Table::search(Code index, int _noperands)
{
	for (Code i = index + 1; entry[i].name != NULL; i++)
	{
		if (strcmp(entry[index].name, entry[i].name) == 0
			&& _noperands == noperands(i))
		{
			return i;
		}
	}
	return Invalid_Code;
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
Code Table::search(Code code, DataType *datatype)
{
	if (match(code, datatype))
	{
		return code;  // main code matches
	}

	for (int n = nassoc_codes(code); --n >= 0;)
	{
		Code assoc_code = this->assoc_code(code, n);
		if (match(assoc_code, datatype))
		{
			return assoc_code;  // associated code matches
		}
	}
	return Invalid_Code;  // no matches
}

// this function checks to see if data types specified match the data
// types of the code at the index specified
//
//    - returns true if there is match, otherwise returns false
//

// 2010-07-02: new function implemented
bool Table::match(Code code, DataType *datatype)
{
	for (int n = noperands(code); --n >= 0;)
	{
		if (datatype[n] != operand_datatype(code, n))
		{
			return false;
		}
	}
	return true;
}


// end: table.cpp
