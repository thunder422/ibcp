// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: table.cpp - operator/command/function table source file
//	Copyright (C) 2010-2015  Thunder422
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
//	2010-02-18	initial version

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "table.h"
#include "token.h"
#include "utility.h"
#include "basic/basic.h"


// operands initializers
constexpr auto Operands_Dbl    = {DataType::Double};
constexpr auto Operands_DblDbl = {DataType::Double, DataType::Double};
constexpr auto Operands_DblInt = {DataType::Double, DataType::Integer};

constexpr auto Operands_Int    = {DataType::Integer};
constexpr auto Operands_IntDbl = {DataType::Integer, DataType::Double};
constexpr auto Operands_IntInt = {DataType::Integer, DataType::Integer};

constexpr auto Operands_Str    = {DataType::String};
constexpr auto Operands_StrInt = {DataType::String, DataType::Integer};
constexpr auto Operands_StrStr = {DataType::String, DataType::String};

constexpr auto Operands_StrIntInt = {
	DataType::String, DataType::Integer, DataType::Integer
};
constexpr auto Operands_StrStrInt = {
	DataType::String, DataType::String, DataType::Integer
};

// expression information structure instances
static ExprInfo Null_ExprInfo;
static ExprInfo Dbl_None_ExprInfo(DataType::Double);
static ExprInfo Dbl_Dbl_ExprInfo(DataType::Double, Operands_Dbl);
static ExprInfo Dbl_DblDbl_ExprInfo(DataType::Double, Operands_DblDbl);
static ExprInfo Dbl_DblInt_ExprInfo(DataType::Double, Operands_DblInt);
static ExprInfo Dbl_Int_ExprInfo(DataType::Double, Operands_Int);
static ExprInfo Dbl_IntDbl_ExprInfo(DataType::Double, Operands_IntDbl);
static ExprInfo Dbl_Str_ExprInfo(DataType::Double, Operands_Str);

static ExprInfo Int_None_ExprInfo(DataType::Integer);
static ExprInfo Int_Dbl_ExprInfo(DataType::Integer, Operands_Dbl);
static ExprInfo Int_DblDbl_ExprInfo(DataType::Integer, Operands_DblDbl);
static ExprInfo Int_DblInt_ExprInfo(DataType::Integer, Operands_DblInt);
static ExprInfo Int_Int_ExprInfo(DataType::Integer, Operands_Int);
static ExprInfo Int_IntInt_ExprInfo(DataType::Integer, Operands_IntInt);
static ExprInfo Int_IntDbl_ExprInfo(DataType::Integer, Operands_IntDbl);
static ExprInfo Int_Str_ExprInfo(DataType::Integer, Operands_Str);
static ExprInfo Int_StrInt_ExprInfo(DataType::Integer, Operands_StrInt);
static ExprInfo Int_StrStr_ExprInfo(DataType::Integer, Operands_StrStr);
static ExprInfo Int_StrStrInt_ExprInfo(DataType::Integer, Operands_StrStrInt);

static ExprInfo Str_None_ExprInfo(DataType::String);
static ExprInfo Str_Dbl_ExprInfo(DataType::String, Operands_Dbl);
static ExprInfo Str_Int_ExprInfo(DataType::String, Operands_Int);
static ExprInfo Str_Str_ExprInfo(DataType::String, Operands_Str);
static ExprInfo Str_StrInt_ExprInfo(DataType::String, Operands_StrInt);
static ExprInfo Str_StrStr_ExprInfo(DataType::String, Operands_StrStr);
static ExprInfo Str_StrIntInt_ExprInfo(DataType::String, Operands_StrIntInt);
static ExprInfo Str_StrStrInt_ExprInfo(DataType::String, Operands_StrStrInt);

static ExprInfo None_Dbl_ExprInfo(DataType::None, Operands_Dbl);
static ExprInfo None_Int_ExprInfo(DataType::None, Operands_Int);
static ExprInfo None_Str_ExprInfo(DataType::None, Operands_Str);


Table *Table::s_instance;			// pointer to single table instance
Table::NameMap Table::s_nameToEntry;		// name to code table map
std::unordered_map<TableEntry *, Table::EntryVectorArray> Table::s_alternate;
std::unordered_map<TableEntry *, DataType> Table::s_expectedDataType;


struct AlternateInfo
{
	Code primaryCode;					// primary code of alternates
	int index;							// alternate array index
	std::initializer_list<Code> codes;	// alternate codes
};

std::initializer_list<AlternateInfo> alternateInfo =
{
	// assignment alternate codes
	{Let_Code, 0, {Assign_Code}},
	{Assign_Code, 0, {AssignInt_Code, AssignStr_Code}},
	{Assign_Code, 1, {AssignList_Code}},
	{AssignInt_Code, 1, {AssignListInt_Code}},
	{AssignStr_Code, 0, {AssignKeepStr_Code}},
	{AssignStr_Code, 1, {AssignListStr_Code}},

	// sub-string assignment alternate codes
	{Left_Code, 1, {AssignLeft_Code}},
	{AssignLeft_Code, 0, {AssignKeepLeft_Code}},
	{Mid2_Code, 1, {AssignMid2_Code}},
	{AssignMid2_Code, 0, {AssignKeepMid2_Code}},
	{Mid3_Code, 1, {AssignMid3_Code}},
	{AssignMid3_Code, 0, {AssignKeepMid3_Code}},
	{Right_Code, 1, {AssignRight_Code}},
	{AssignRight_Code, 0, {AssignKeepRight_Code}},

	// internal command alternate codes
	{Input_Code, 0, {InputBegin_Code}},
	{Input_Code, 1, {InputAssign_Code}},
	{InputPrompt_Code, 0, {InputBeginStr_Code}},
	{InputPrompt_Code, 1, {InputAssign_Code}},
	{InputAssign_Code, 0, {InputAssignInt_Code, InputAssignStr_Code}},
	{InputAssign_Code, 1, {InputParse_Code}},
	{InputAssignInt_Code, 1, {InputParseInt_Code}},
	{InputAssignStr_Code, 1, {InputParseStr_Code}},
	{Print_Code, 0, {PrintDbl_Code}},
	{PrintDbl_Code, 0, {PrintInt_Code, PrintStr_Code}},
	{SemiColon_Code, 0, {Print_Code}},

	// codes with operands alternate codes
	{Const_Code, 0, {ConstInt_Code, ConstStr_Code}},
	{Var_Code, 0, {VarInt_Code, VarStr_Code}},
	{Var_Code, 1, {VarRef_Code}},
	{VarRef_Code, 0, {VarRefInt_Code, VarRefStr_Code}},
	{Array_Code, 0, {ArrayInt_Code, ArrayStr_Code}},
	{DefFuncN_Code, 0, {DefFuncNInt_Code, DefFuncNStr_Code}},
	{DefFuncP_Code, 0, {DefFuncPInt_Code, DefFuncPStr_Code}},
	{Function_Code, 0, {FunctionInt_Code, FunctionStr_Code}}
};


// code enumeration names in comments after opening brace
// (code enumeration generated from these by enums.awk)
static TableEntry tableEntries[] =
{
	// Code{} entry at beginning because Code{} == 0
	{	// Code{}
		Type{},
		"", "NULL", "",
		TableFlag{}, 0, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	//--------------
	//   Commands
	//--------------
	{	// Let_Code
		Type::Command,
		"LET", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		letTranslate, NULL, NULL, NULL, NULL
	},
	{	// Print_Code
		Type::Command,
		"PRINT", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		printTranslate, NULL, NULL, NULL, printRecreate
	},
	{	// Input_Code
		Type::Command,
		"INPUT", "", "Keep",
		Two_Flag, 4, &Null_ExprInfo,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// InputPrompt_Code
		Type::Command,
		"INPUT", "PROMPT", "Keep",
		Two_Flag, 4, &Null_ExprInfo,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// Dim_Code
		Type::Command,
		"DIM", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Def_Code
		Type::Command,
		"DEF", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Rem_Code
		Type::Command,
		"REM", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	{	// If_Code
		Type::Command,
		"IF", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Then_Code
		Type::Command,
		"THEN", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Else_Code
		Type::Command,
		"ELSE", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// End_Code
		Type::Command,
		"END", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// EndIf_Code
		Type::Command,
		"END", "IF", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// For_Code
		Type::Command,
		"FOR", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// To_Code
		Type::Command,
		"TO", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Step_Code
		Type::Command,
		"STEP", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Next_Code
		Type::Command,
		"NEXT", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Do_Code
		Type::Command,
		"DO", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoWhile_Code
		Type::Command,
		"DO", "WHILE", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoUntil_Code
		Type::Command,
		"DO", "UNTIL", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// While_Code
		Type::Command,
		"WHILE", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Until_Code
		Type::Command,
		"UNTIL", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Loop_Code
		Type::Command,
		"LOOP", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopWhile_Code
		Type::Command,
		"LOOP", "WHILE", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopUntil_Code
		Type::Command,
		"LOOP", "UNTIL", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{	// Rnd_Code
		Type::IntFunc,
		"RND", "", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//--------------------
	//   Word Operators
	//--------------------
	{	// Mod_Code
		Type::Operator,
		"MOD", "", "",
		TableFlag{}, 42, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// And_Code
		Type::Operator,
		"AND", "", "",
		TableFlag{}, 18, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Or_Code
		Type::Operator,
		"OR", "", "",
		TableFlag{}, 14, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Not_Code
		Type::Operator,
		"NOT", "", "",
		TableFlag{}, 20, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Eqv_Code
		Type::Operator,
		"EQV", "", "",
		TableFlag{}, 12, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Imp_Code
		Type::Operator,
		"IMP", "", "",
		TableFlag{}, 10, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Xor_Code
		Type::Operator,
		"XOR", "", "",
		TableFlag{}, 16, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{	// Abs_Code
		Type::IntFunc,
		"ABS(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Fix_Code
		Type::IntFunc,
		"FIX(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Frac_Code
		Type::IntFunc,
		"FRAC(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Int_Code
		Type::IntFunc,
		"INT(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate

	},
	{	// RndArg_Code
		Type::IntFunc,
		"RND(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sgn_Code
		Type::IntFunc,
		"SGN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cint_Code
		Type::IntFunc,
		"CINT(", "", "",
		TableFlag{}, 2, &Int_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cdbl_Code
		Type::IntFunc,
		"CDBL(", "", "",
		TableFlag{}, 2, &Dbl_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sqr_Code
		Type::IntFunc,
		"SQR(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Atn_Code
		Type::IntFunc,
		"ATN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cos_Code
		Type::IntFunc,
		"COS(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sin_Code
		Type::IntFunc,
		"SIN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tan_Code
		Type::IntFunc,
		"TAN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Exp_Code
		Type::IntFunc,
		"EXP(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Log_Code
		Type::IntFunc,
		"LOG(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tab_Code
		Type::IntFunc,
		"TAB(", "", "",
		Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Spc_Code
		Type::IntFunc,
		"SPC(", "", "",
		Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Asc_Code
		Type::IntFunc,
		"ASC(", "", "",
		TableFlag{}, 2, &Int_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Asc2_Code
		Type::IntFunc,
		"ASC(", "2", "",
		TableFlag{}, 2, &Int_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Chr_Code
		Type::IntFunc,
		"CHR$(", "", "",
		TableFlag{}, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr2_Code
		Type::IntFunc,
		"INSTR(", "2", "",
		TableFlag{}, 2, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr3_Code
		Type::IntFunc,
		"INSTR(", "3", "",
		TableFlag{}, 2, &Int_StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Left_Code
		Type::IntFunc,
		"LEFT$(", "", "",
		SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Len_Code
		Type::IntFunc,
		"LEN(", "", "",
		TableFlag{}, 2, &Int_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid2_Code
		Type::IntFunc,
		"MID$(", "2", "",
		SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid3_Code
		Type::IntFunc,
		"MID$(", "3", "",
		SubStr_Flag, 2, &Str_StrIntInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Repeat_Code
		Type::IntFunc,
		"REPEAT$(", "", "",
		TableFlag{}, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Right_Code
		Type::IntFunc,
		"RIGHT$(", "", "",
		SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Space_Code
		Type::IntFunc,
		"SPACE$(", "", "",
		TableFlag{}, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Str_Code
		Type::IntFunc,
		"STR$(", "", "",
		TableFlag{}, 2, &Str_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Val_Code
		Type::IntFunc,
		"VAL(", "", "",
		TableFlag{}, 2, &Dbl_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{	// Add_Code
		Type::Operator,
		"+", "", "",
		TableFlag{}, 40, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Neg_Code
		Type::Operator,
		"-", "U", "",
		UseConstAsIs_Flag, 48, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Mul_Code
		Type::Operator,
		"*", "", "",
		TableFlag{}, 46, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Div_Code
		Type::Operator,
		"/", "", "",
		TableFlag{}, 46, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// IntDiv_Code
		Type::Operator,
		"\\", "", "",
		TableFlag{}, 44, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Power_Code
		Type::Operator,
		"^", "", "",
		UseConstAsIs_Flag, 50, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Eq_Code
		Type::Operator,
		"=", "", "",
		TableFlag{}, 30, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Gt_Code
		Type::Operator,
		">", "", "",
		Two_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEq_Code
		Type::Operator,
		">=", "", "",
		TableFlag{}, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Lt_Code
		Type::Operator,
		"<", "", "",
		Two_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEq_Code
		Type::Operator,
		"<=", "", "",
		TableFlag{}, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEq_Code
		Type::Operator,
		"<>", "", "",
		Two_Flag, 30, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// OpenParen_Code
		Type::Operator,
		"(", "", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// CloseParen_Code
		Type::Operator,
		")", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, parenRecreate
	},
	{	// Comma_Code
		Type::Operator,
		",", "", "",
		Command_Flag, 6, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, printCommaRecreate
	},
	{	// SemiColon_Code
		Type::Operator,
		";", "", "",
		Command_Flag, 6, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, printSemicolonRecreate
	},
	{	// Colon_Code
		Type::Operator,
		":", "", "",
		EndStmt_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// RemOp_Code
		Type::Operator,
		"'", "", "",
		EndStmt_Flag, 2, &Null_ExprInfo,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	{	// Assign_Code
		Type{},
		"=", "Assign", "LET",
		Reference_Flag | Command_Flag, 4, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignInt_Code
		Type{},
		"=", "Assign%", "LET",
		Reference_Flag | Command_Flag, 4, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignStr_Code
		Type{},
		"=", "Assign$", "LET",
		Reference_Flag | Command_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignLeft_Code
		Type{},
		"LEFT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag | Command_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid2_Code
		Type{},
		"MID$(", "Assign2", "LET",
		Reference_Flag | SubStr_Flag | Command_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid3_Code
		Type{},
		"MID$(", "Assign3", "LET",
		Reference_Flag | SubStr_Flag | Command_Flag, 4, &Str_StrIntInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignRight_Code
		Type{},
		"RIGHT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignList_Code
		Type{},
		"=", "AssignList", "LET",
		Reference_Flag | Command_Flag, 4, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListInt_Code
		Type{},
		"=", "AssignList%", "LET",
		Reference_Flag | Command_Flag, 4, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListStr_Code
		Type{},
		"=", "AssignList$", "LET",
		Reference_Flag | Command_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignKeepStr_Code
		Type{},
		"", "AssignKeep$", "LET",
		Reference_Flag | Keep_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepLeft_Code
		Type{},
		"LEFT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid2_Code
		Type{},
		"MID$(", "AssignKeep2", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid3_Code
		Type{},
		"MID$(", "AssignKeep3", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepRight_Code
		Type{},
		"RIGHT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// EOL_Code
		Type::Operator,
		"", "EOL", "",
		EndStmt_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL
	},
	{	// AddI1_Code
		Type::Operator,
		"+", "%1", "",
		TableFlag{}, 40, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddI2_Code
		Type::Operator,
		"+", "%2", "",
		TableFlag{}, 40, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddInt_Code
		Type::Operator,
		"+", "%", "",
		TableFlag{}, 40, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// CatStr_Code
		Type::Operator,
		"+", "$", "",
		TableFlag{}, 40, &Str_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Sub_Code
		Type::Operator,
		"-", "", "",
		TableFlag{}, 40, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI1_Code
		Type::Operator,
		"-", "%1", "",
		TableFlag{}, 40, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI2_Code
		Type::Operator,
		"-", "%2", "",
		TableFlag{}, 40, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubInt_Code
		Type::Operator,
		"-", "%", "",
		TableFlag{}, 40, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NegInt_Code
		Type::Operator,
		"-", "U%", "",
		TableFlag{}, 48, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// MulI1_Code
		Type::Operator,
		"*", "%1", "",
		TableFlag{}, 46, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulI2_Code
		Type::Operator,
		"*", "%2", "",
		TableFlag{}, 46, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulInt_Code
		Type::Operator,
		"*", "%", "",
		TableFlag{}, 46, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI1_Code
		Type::Operator,
		"/", "%1", "",
		TableFlag{}, 46, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI2_Code
		Type::Operator,
		"/", "%2", "",
		TableFlag{}, 46, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivInt_Code
		Type::Operator,
		"/", "%", "",
		TableFlag{}, 46, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI1_Code
		Type::Operator,
		"MOD", "%1", "",
		TableFlag{}, 42, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI2_Code
		Type::Operator,
		"MOD", "%2", "",
		TableFlag{}, 42, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModInt_Code
		Type::Operator,
		"MOD", "%", "",
		TableFlag{}, 42, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerI1_Code
		Type::Operator,
		"^", "%1", "",
		TableFlag{}, 50, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerMul_Code
		Type::Operator,
		"^", "*", "",
		TableFlag{}, 50, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerInt_Code
		Type::Operator,
		"^", "%", "",
		TableFlag{}, 50, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI1_Code
		Type::Operator,
		"=", "%1", "",
		TableFlag{}, 30, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI2_Code
		Type::Operator,
		"=", "%2", "",
		TableFlag{}, 30, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqInt_Code
		Type::Operator,
		"=", "%", "",
		TableFlag{}, 30, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqStr_Code
		Type::Operator,
		"=", "$", "",
		TableFlag{}, 30, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI1_Code
		Type::Operator,
		">", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI2_Code
		Type::Operator,
		">", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtInt_Code
		Type::Operator,
		">", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtStr_Code
		Type::Operator,
		">", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI1_Code
		Type::Operator,
		">=", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI2_Code
		Type::Operator,
		">=", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqInt_Code
		Type::Operator,
		">=", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqStr_Code
		Type::Operator,
		">=", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI1_Code
		Type::Operator,
		"<", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI2_Code
		Type::Operator,
		"<", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtInt_Code
		Type::Operator,
		"<", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtStr_Code
		Type::Operator,
		"<", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI1_Code
		Type::Operator,
		"<=", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI2_Code
		Type::Operator,
		"<=", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqInt_Code
		Type::Operator,
		"<=", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqStr_Code
		Type::Operator,
		"<=", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI1_Code
		Type::Operator,
		"<>", "%1", "",
		TableFlag{}, 30, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI2_Code
		Type::Operator,
		"<>", "%2", "",
		TableFlag{}, 30, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqInt_Code
		Type::Operator,
		"<>", "%", "",
		TableFlag{}, 30, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqStr_Code
		Type::Operator,
		"<>", "$", "",
		TableFlag{}, 30, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AbsInt_Code
		Type::IntFunc,
		"ABS(", "%", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// RndArgInt_Code
		Type::IntFunc,
		"RND(", "%", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// SgnInt_Code
		Type::IntFunc,
		"SGN(", "%", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// CvtInt_Code
		Type::IntFunc,
		"", "CvtInt", "",
		Hidden_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// CvtDbl_Code
		Type::IntFunc,
		"", "CvtDbl", "",
		Hidden_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// StrInt_Code
		Type::IntFunc,
		"STR$(", "%", "",
		TableFlag{}, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// PrintDbl_Code
		Type{},
		"", "PrintDbl", "",
		Print_Flag | UseConstAsIs_Flag, 2, &None_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintInt_Code
		Type{},
		"", "PrintInt", "",
		Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintStr_Code
		Type{},
		"", "PrintStr", "",
		Print_Flag, 2, &None_Str_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// InputBegin_Code
		Type{},
		"", "InputBegin", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputBeginStr_Code
		Type{},
		"", "InputBeginStr", "Question",
		TableFlag{}, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, inputPromptBeginRecreate
	},
	{	// InputAssign_Code
		Type{},
		"", "InputAssign", "",
		Reference_Flag, 2, &None_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignInt_Code
		Type{},
		"", "InputAssignInt", "",
		Reference_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignStr_Code
		Type{},
		"", "InputAssignStr", "",
		Reference_Flag, 2, &None_Str_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputParse_Code
		Type{},
		"", "InputParse", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseInt_Code
		Type{},
		"", "InputParseInt", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseStr_Code
		Type{},
		"", "InputParseStr", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// Const_Code
		Type::Constant,
		"", "Const", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstInt_Code
		Type::Constant,
		"", "ConstInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstStr_Code
		Type::Constant,
		"", "ConstStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, constStrEncode, constStrOperandText, constStrRemove,
		constStrRecreate
	},
	{	// Var_Code
		Type::NoParen,
		"", "Var", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarInt_Code
		Type::NoParen,
		"", "VarInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarStr_Code
		Type::NoParen,
		"", "VarStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// VarRef_Code
		Type::NoParen,
		"", "VarRef", "",
		Reference_Flag, 2, &Dbl_None_ExprInfo,
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarRefInt_Code
		Type::NoParen,
		"", "VarRefInt", "",
		Reference_Flag, 2, &Int_None_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarRefStr_Code
		Type::NoParen,
		"", "VarRefStr", "",
		Reference_Flag, 2, &Str_None_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// Array_Code
		Type::Paren,
		"", "Array", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// ArrayInt_Code
		Type::Paren,
		"", "ArrayInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// ArrayStr_Code
		Type::Paren,
		"", "ArrayStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// DefFuncN_Code
		Type::DefFunc,
		"", "DefFuncN", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncNInt_Code
		Type::DefFunc,
		"", "DefFuncNInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncNStr_Code
		Type::DefFunc,
		"", "DefFuncNStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncP_Code
		Type::DefFunc,
		"", "DefFuncP", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncPInt_Code
		Type::DefFunc,
		"", "DefFuncPInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncPStr_Code
		Type::DefFunc,
		"", "DefFuncPStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// Function_Code
		Type::Paren,
		"", "Function", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, functionRecreate
	},
	{	// FunctionInt_Code
		Type::Paren,
		"", "FunctionInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, functionRecreate
	},
	{	// FunctionStr_Code
		Type::Paren,
		"", "FunctionStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, functionRecreate
	}
};


// static function to return a refernce to the single table instance
//
//   - creates the single table instance with the table entries
//   - constructor fails (abort application) if there are table errors

Table &Table::instance(void)
{
	if (!s_instance)
	try
	{
		s_instance = new Table(tableEntries, sizeof(tableEntries)
			/ sizeof(TableEntry));
	}
	catch (std::string &error)
	{
		std::cerr << "Table Bug: " << error << std::endl;
		abort();
	}
	return *s_instance;
}


// constructor function that initializes the table instance variables
//
//   - fatally aborts application if table entry errors were detected
//   - table entry errors are output before aborting

Table::Table(TableEntry *entry, int entryCount) :
	m_entry(entry)
{
	std::vector<std::string> errorList;

	// iterate entries and build alternate code map
	for (int i = 0; i < entryCount; i++)
	{
		try
		{
			add(m_entry[i]);
		}
		catch (std::string &error)
		{
			errorList.emplace_back(std::move(error));
		}
	}

	// manually set up other alternates (temporary)
	for (auto info : alternateInfo)
	{
		auto primary = &m_entry[info.primaryCode];
		for (Code code : info.codes)
		{
			auto alternate = &m_entry[code];
			s_alternate[primary][info.index].push_back(alternate);
		}
	}

	// if errors then output messages and abort
	if (!errorList.empty())
	{
		int n {};
		for (std::string &error : errorList)
		{
			std::cerr << "Table Error #" << ++n << ": " << error << std::endl;
		}
		abort();
	}
}


// function to added code entry info to the table
void Table::add(TableEntry &entry)
{
	int index = &entry - m_entry;

	// is code two-words?
	if (entry.hasFlag(Two_Flag) && !entry.m_name2.empty())
	{
		std::string name {entry.m_name + ' ' + entry.m_name2};
		auto iterator = s_nameToEntry.find(name);
		if (iterator != s_nameToEntry.end())  // already in map?
		{
			throw "Multiple two-word command '" + name + '\'';
		}
		s_nameToEntry.emplace(std::move(name), &entry);
		return;
	}

	if (!entry.m_name.empty())
	{
		auto iterator = s_nameToEntry.find(entry.m_name);
		if (iterator == s_nameToEntry.end())  // not in table?
		{
			if (entry.m_type == Type::Operator
				&& entry.m_exprInfo->m_operandCount == 2
				&& entry.m_exprInfo->m_operandDataType[0]
				!= entry.m_exprInfo->m_operandDataType[1])
			{
				throw "Binary operator '" + entry.m_name + entry.m_name2
					+ "' not homogeneous";
			}
			s_nameToEntry.emplace(entry.m_name, &entry);
			if (entry.m_exprInfo->m_operandCount > 0)
			{
				int index = entry.m_type == Type::Operator
					? entry.m_exprInfo->m_operandCount - 1 : 0;
				addExpectedDataType(&entry,
					entry.m_exprInfo->m_operandDataType[index]);
			}
			return;  // primary code, nothing more to do
		}
		ExprInfo *exprInfo {entry.m_exprInfo};
		if (exprInfo->m_operandCount > 0 && !entry.hasFlag(Reference_Flag))
		{
			TableEntry *primary = iterator->second;

			if (exprInfo->m_operandCount < primary->m_exprInfo->m_operandCount)
			{
				TableEntry *alternate = primary;
				iterator->second = &entry;
				s_alternate[&entry][alternate->m_exprInfo->m_operandCount - 1]
					.push_back(alternate);
				if (entry.m_type == Type::IntFunc)
				{
					// multiple codes; set multiple flag on primary code
					entry.m_flags |= Multiple_Flag;
					// erase original expected data type of function
					s_expectedDataType.erase(alternate);
				}
				addExpectedDataType(&entry,
					entry.m_exprInfo->m_operandDataType[0]);
				return;  // new primary code, nothing more to do
			}

			if (entry.m_type == Type::Operator
				&& exprInfo->m_operandCount
				> primary->m_exprInfo->m_operandCount)
			{
				// not the correct primary entry
				// need to get correct primary entry
				auto &vector = s_alternate[primary][exprInfo->m_operandCount
					- 1];
				if (vector.empty())
				{
					if (entry.m_type == Type::Operator
						&& entry.m_exprInfo->m_operandCount == 2
						&& entry.m_exprInfo->m_operandDataType[0]
						!= entry.m_exprInfo->m_operandDataType[1])
					{
						throw "First binary operator '" + entry.m_name
							+ entry.m_name2 + "' not homogeneous";
					}
					vector.push_back(&entry);
					addExpectedDataType(&entry,
						entry.m_exprInfo->m_operandDataType[0]);
					return;  // first alternate, nothing more to do
				}
				primary = vector.front();
			}

			for (int i = 0; i < primary->m_exprInfo->m_operandCount; ++i)
			{
				if (exprInfo->m_operandDataType[i]
					!= primary->m_exprInfo->m_operandDataType[i])
				{
					auto newEntry = &entry;
					do
					{
						TableEntry *newPrimary {};
						for (auto &alternate : s_alternate[primary][i])
						{
							if (exprInfo->m_operandDataType[i]
								== alternate->m_exprInfo->m_operandDataType[i])
							{
								if (entry.m_type == Type::Operator
									&& entry.m_exprInfo->m_operandCount == 2
									&& entry.m_exprInfo->m_operandDataType[0]
									== entry.m_exprInfo->m_operandDataType[1])
								{
									s_expectedDataType.erase(alternate);
									addExpectedDataType(newEntry, newEntry
										->m_exprInfo->m_operandDataType[i]);
									std::swap(newEntry, alternate);
								}
								newPrimary = alternate;
								++i;
								break;
							}
						}
						if (!newPrimary)
						{
							s_alternate[primary][i].push_back(newEntry);
							addExpectedDataType(i == 0
								&& newEntry->m_type == Type::Operator
								&& newEntry->m_exprInfo->m_operandCount == 2
								? newEntry : primary,
								newEntry->m_exprInfo->m_operandDataType[i]);
							return;  // alternate added, nothing more to do
						}
						primary = newPrimary;  // new primary, next operand
					}
					while (i < exprInfo->m_operandCount);
					break;  // no more operands, all operands match
				}
			}
			if (entry.m_type == Type::IntFunc && exprInfo->m_operandCount
				> primary->m_exprInfo->m_operandCount)
			{
				// multiple codes; set multiple flag on primary code
				primary->m_flags |= Multiple_Flag;
				s_alternate[primary][exprInfo->m_operandCount - 1]
					.push_back(&entry);
			}
			else
			{
				throw "Multiple entries with same operand data types ("
					+ std::to_string(index) + ','
					+ std::to_string(primary - m_entry) + ')';
			}
		}
	}
}


// function to add an expected data type for an entry
void Table::addExpectedDataType(TableEntry *entry, DataType dataType)
{
	DataType current = s_expectedDataType[entry];
	if (current == DataType{})
	{
		s_expectedDataType[entry] = dataType;
	}
	else if (current == DataType::Double || current == DataType::Integer)
	{
		s_expectedDataType[entry] = DataType::Number;
	}
}


//========================
//  TABLE ENTRY FUNCTIONS
//========================

// function to get code for entry
Code TableEntry::code() const
{
	return Code(this - tableEntries);
}

std::string TableEntry::commandName() const
{
	std::string string {m_name};
	if (hasFlag(Two_Flag) && !m_name2.empty())
	{
		string += ' ' + m_name2;
	}
	return string;
}

DataType TableEntry::expectedDataType()
{
	return Table::s_expectedDataType.at(this);
}

// function to get an alternate table entry for the data type specified
//
//   - if the data type does not match the return data type of the entry,
//     then searches its alternates if there are any
//   - if no matching alternate entry found, then returns the entry

TableEntry *TableEntry::alternate(DataType returnDataType)
{
	if (returnDataType != m_exprInfo->m_returnDataType
		&& Table::s_alternate.find(this) != Table::s_alternate.end())
	{
		for (TableEntry *alternateEntry : Table::s_alternate[this][0])
		{
			if (returnDataType == alternateEntry->returnDataType())
			{
				return alternateEntry;
			}
		}
	}
	return this;  // use entry if no alternate
}

// function to get count of alternate codes for an operand index
int TableEntry::alternateCount(int operandIndex)
{
	return Table::s_alternate[this][operandIndex].size();
}

// function to get first alternate entry for an operand index
TableEntry *TableEntry::alternate(int operandIndex)
{
    return Table::s_alternate[this][operandIndex].front();
}

// function to get alternate entry for specified operand index and data type
//
//   - if the data type does not match the operand data type of the code,
//     then the alternate codes are searched for a matching code
//   - if there are no alternate codes, or none is found, returns null
//   - upon success, returns alternate code table entry

TableEntry *TableEntry::alternate(int operandIndex, DataType operandDataType)
{
	// first check if data type already matches data type of code
	if (operandDataType == m_exprInfo->m_operandDataType[operandIndex])
	{
		return this;
	}
	// if not, see if data type of any alternate code matches
	if (Table::s_alternate.find(this) != Table::s_alternate.end())
	{
		for (TableEntry *alternateEntry
			: Table::s_alternate[this][operandIndex])
		{
			if (operandDataType
				== alternateEntry->m_exprInfo->m_operandDataType[operandIndex])
			{
				return alternateEntry;
			}
		}
	}
	return {};  // did not find an alternate code for data type
}


//============================
//  TABLE SPECIFIC FUNCTIONS
//============================

// function to return table entry pointer for a code index
TableEntry *Table::entry(int index)
{
	return &tableEntries[index];
}


// function to return table entry pointer for a code index and data type
TableEntry *Table::entry(int index, DataType dataType)
{
	return tableEntries[index].alternate(dataType);
}


// find function to look for a string in the table
//
//   - returns the index of the entry that is found
//   - returns Invalid_Code if the string was not found in the table

TableEntry *Table::find(const std::string &string)
{
	auto iterator = s_nameToEntry.find(string);
	if (iterator != s_nameToEntry.end())
	{
		return iterator->second;
	}
	return {};
}


// end: table.cpp
