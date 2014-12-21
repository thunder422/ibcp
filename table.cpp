// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: table.cpp - operator/command/function table source file
//	Copyright (C) 2010-2013  Thunder422
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
#include "utility.h"
#include "basic/basic.h"


// expression information for operators and internal functions
struct ExprInfo
{
	DataType m_returnDataType;		// return data type of operator/function
	short m_operandCount;			// number of operands (operators/functions)
	const DataType *m_operandDataType;	// data type of each operand

	ExprInfo(DataType returnDataType = DataType::None,
		const std::initializer_list<const DataType> &operands = {}) :
		m_returnDataType {returnDataType},
		m_operandCount(operands.size()),
		m_operandDataType {operands.begin()}
	{
	}
};

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


struct TableEntry
{
	Token::Type type;				// type of token for entry
	const std::string name;			// name for table entry
	const std::string name2;		// name of second word of command
	const std::string option;		// name of option sub-code
	unsigned flags;					// flags for entry
	int precedence;					// precedence of code
	ExprInfo *exprInfo;				// expression info pointer (NULL for none)
	TranslateFunction translate;	// pointer to translate function
	EncodeFunction encode;			// pointer to encode function
	OperandTextFunction operandText;// pointer to operand text function
	RemoveFunction remove;			// pointer to remove function
	RecreateFunction recreate;		// pointer to recreate function
};


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
	{InputAssign_Code, 0, {InputAssignInt_Code, InputAssignStr_Code}},
	{InputAssign_Code, 1, {InputParse_Code}},
	{InputAssignInt_Code, 1, {InputParseInt_Code}},
	{InputAssignStr_Code, 1, {InputParseStr_Code}},
	{PrintDbl_Code, 0, {PrintInt_Code, PrintStr_Code}},

	// codes with operands alternate codes
	{Const_Code, 0, {ConstInt_Code, ConstStr_Code}},
	{Var_Code, 0, {VarInt_Code, VarStr_Code}},
	{VarRef_Code, 0, {VarRefInt_Code, VarRefStr_Code}}
};


// code enumeration names in comments after opening brace
// (code enumeration generated from these by enums.awk)
static TableEntry tableEntries[] =
{
	// Null_Code entry at beginning so Null_Code == 0
	{	// Null_Code
		Token::Type{},
		"", "NULL", "",
		TableFlag{}, 0, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	//--------------
	//   Commands
	//--------------
	{	// Let_Code
		Token::Type::Command,
		"LET", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		letTranslate, NULL, NULL, NULL, NULL
	},
	{	// Print_Code
		Token::Type::Command,
		"PRINT", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		printTranslate, NULL, NULL, NULL, printRecreate
	},
	{	// Input_Code
		Token::Type::Command,
		"INPUT", "", "Keep",
		Two_Flag, 4, &Null_ExprInfo,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// InputPrompt_Code
		Token::Type::Command,
		"INPUT", "PROMPT", "Keep",
		Two_Flag, 4, &Null_ExprInfo,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// Dim_Code
		Token::Type::Command,
		"DIM", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Def_Code
		Token::Type::Command,
		"DEF", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Rem_Code
		Token::Type::Command,
		"REM", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	{	// If_Code
		Token::Type::Command,
		"IF", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Then_Code
		Token::Type::Command,
		"THEN", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Else_Code
		Token::Type::Command,
		"ELSE", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// End_Code
		Token::Type::Command,
		"END", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// EndIf_Code
		Token::Type::Command,
		"END", "IF", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// For_Code
		Token::Type::Command,
		"FOR", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// To_Code
		Token::Type::Command,
		"TO", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Step_Code
		Token::Type::Command,
		"STEP", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Next_Code
		Token::Type::Command,
		"NEXT", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Do_Code
		Token::Type::Command,
		"DO", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoWhile_Code
		Token::Type::Command,
		"DO", "WHILE", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoUntil_Code
		Token::Type::Command,
		"DO", "UNTIL", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// While_Code
		Token::Type::Command,
		"WHILE", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Until_Code
		Token::Type::Command,
		"UNTIL", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Loop_Code
		Token::Type::Command,
		"LOOP", "", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopWhile_Code
		Token::Type::Command,
		"LOOP", "WHILE", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopUntil_Code
		Token::Type::Command,
		"LOOP", "UNTIL", "",
		Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{	// Rnd_Code
		Token::Type::IntFuncN,
		"RND", "", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//--------------------
	//   Word Operators
	//--------------------
	{	// Mod_Code
		Token::Type::Operator,
		"MOD", "", "",
		TableFlag{}, 42, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// And_Code
		Token::Type::Operator,
		"AND", "", "",
		TableFlag{}, 18, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Or_Code
		Token::Type::Operator,
		"OR", "", "",
		TableFlag{}, 14, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Not_Code
		Token::Type::Operator,
		"NOT", "", "",
		TableFlag{}, 20, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Eqv_Code
		Token::Type::Operator,
		"EQV", "", "",
		TableFlag{}, 12, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Imp_Code
		Token::Type::Operator,
		"IMP", "", "",
		TableFlag{}, 10, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Xor_Code
		Token::Type::Operator,
		"XOR", "", "",
		TableFlag{}, 16, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{	// Abs_Code
		Token::Type::IntFuncP,
		"ABS(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Fix_Code
		Token::Type::IntFuncP,
		"FIX(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Frac_Code
		Token::Type::IntFuncP,
		"FRAC(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Int_Code
		Token::Type::IntFuncP,
		"INT(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate

	},
	{	// RndArg_Code
		Token::Type::IntFuncP,
		"RND(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sgn_Code
		Token::Type::IntFuncP,
		"SGN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cint_Code
		Token::Type::IntFuncP,
		"CINT(", "", "",
		TableFlag{}, 2, &Int_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cdbl_Code
		Token::Type::IntFuncP,
		"CDBL(", "", "",
		TableFlag{}, 2, &Dbl_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sqr_Code
		Token::Type::IntFuncP,
		"SQR(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Atn_Code
		Token::Type::IntFuncP,
		"ATN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cos_Code
		Token::Type::IntFuncP,
		"COS(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sin_Code
		Token::Type::IntFuncP,
		"SIN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tan_Code
		Token::Type::IntFuncP,
		"TAN(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Exp_Code
		Token::Type::IntFuncP,
		"EXP(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Log_Code
		Token::Type::IntFuncP,
		"LOG(", "", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tab_Code
		Token::Type::IntFuncP,
		"TAB(", "", "",
		Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Spc_Code
		Token::Type::IntFuncP,
		"SPC(", "", "",
		Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Asc_Code
		Token::Type::IntFuncP,
		"ASC(", "", "",
		TableFlag{}, 2, &Int_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Asc2_Code
		Token::Type::IntFuncP,
		"ASC(", "2", "",
		TableFlag{}, 2, &Int_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Chr_Code
		Token::Type::IntFuncP,
		"CHR$(", "", "",
		TableFlag{}, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr2_Code
		Token::Type::IntFuncP,
		"INSTR(", "2", "",
		TableFlag{}, 2, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr3_Code
		Token::Type::IntFuncP,
		"INSTR(", "3", "",
		TableFlag{}, 2, &Int_StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Left_Code
		Token::Type::IntFuncP,
		"LEFT$(", "", "",
		SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Len_Code
		Token::Type::IntFuncP,
		"LEN(", "", "",
		TableFlag{}, 2, &Int_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid2_Code
		Token::Type::IntFuncP,
		"MID$(", "2", "",
		SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid3_Code
		Token::Type::IntFuncP,
		"MID$(", "3", "",
		SubStr_Flag, 2, &Str_StrIntInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Repeat_Code
		Token::Type::IntFuncP,
		"REPEAT$(", "", "",
		TableFlag{}, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Right_Code
		Token::Type::IntFuncP,
		"RIGHT$(", "", "",
		SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Space_Code
		Token::Type::IntFuncP,
		"SPACE$(", "", "",
		TableFlag{}, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Str_Code
		Token::Type::IntFuncP,
		"STR$(", "", "",
		TableFlag{}, 2, &Str_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Val_Code
		Token::Type::IntFuncP,
		"VAL(", "", "",
		TableFlag{}, 2, &Dbl_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{	// Add_Code
		Token::Type::Operator,
		"+", "", "",
		TableFlag{}, 40, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Neg_Code
		Token::Type::Operator,
		"-", "U", "",
		UseConstAsIs_Flag, 48, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Mul_Code
		Token::Type::Operator,
		"*", "", "",
		TableFlag{}, 46, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Div_Code
		Token::Type::Operator,
		"/", "", "",
		TableFlag{}, 46, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// IntDiv_Code
		Token::Type::Operator,
		"\\", "", "",
		TableFlag{}, 44, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Power_Code
		Token::Type::Operator,
		"^", "", "",
		UseConstAsIs_Flag, 50, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Eq_Code
		Token::Type::Operator,
		"=", "", "",
		TableFlag{}, 30, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Gt_Code
		Token::Type::Operator,
		">", "", "",
		Two_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEq_Code
		Token::Type::Operator,
		">=", "", "",
		TableFlag{}, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Lt_Code
		Token::Type::Operator,
		"<", "", "",
		Two_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEq_Code
		Token::Type::Operator,
		"<=", "", "",
		TableFlag{}, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEq_Code
		Token::Type::Operator,
		"<>", "", "",
		Two_Flag, 30, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// OpenParen_Code
		Token::Type::Operator,
		"(", "", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// CloseParen_Code
		Token::Type::Operator,
		")", "", "",
		TableFlag{}, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, parenRecreate
	},
	{	// Comma_Code
		Token::Type::Operator,
		",", "", "",
		TableFlag{}, 6, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, printCommaRecreate
	},
	{	// SemiColon_Code
		Token::Type::Operator,
		";", "", "",
		TableFlag{}, 6, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, printSemicolonRecreate
	},
	{	// Colon_Code
		Token::Type::Operator,
		":", "", "",
		EndStmt_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// RemOp_Code
		Token::Type::Operator,
		"'", "", "",
		EndStmt_Flag, 2, &Null_ExprInfo,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	{	// Assign_Code
		Token::Type{},
		"=", "Assign", "LET",
		Reference_Flag, 4, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignInt_Code
		Token::Type{},
		"=", "Assign%", "LET",
		Reference_Flag, 4, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignStr_Code
		Token::Type{},
		"=", "Assign$", "LET",
		Reference_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignLeft_Code
		Token::Type{},
		"LEFT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid2_Code
		Token::Type{},
		"MID$(", "Assign2", "LET",
		Reference_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid3_Code
		Token::Type{},
		"MID$(", "Assign3", "LET",
		Reference_Flag | SubStr_Flag | Multiple_Flag, 4, &Str_StrIntInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignRight_Code
		Token::Type{},
		"RIGHT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignList_Code
		Token::Type{},
		"=", "AssignList", "LET",
		Reference_Flag, 4, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListInt_Code
		Token::Type{},
		"=", "AssignList%", "LET",
		Reference_Flag, 4, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListStr_Code
		Token::Type{},
		"=", "AssignList$", "LET",
		Reference_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignKeepStr_Code
		Token::Type{},
		"", "AssignKeep$", "LET",
		Reference_Flag | Keep_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepLeft_Code
		Token::Type{},
		"LEFT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid2_Code
		Token::Type{},
		"MID$(", "AssignKeep2", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid3_Code
		Token::Type{},
		"MID$(", "AssignKeep3", "LET",
		Reference_Flag | SubStr_Flag | Multiple_Flag | Keep_Flag, 4,
		&Str_StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepRight_Code
		Token::Type{},
		"RIGHT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// EOL_Code
		Token::Type::Operator,
		"", "EOL", "",
		EndStmt_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL
	},
	{	// AddI1_Code
		Token::Type::Operator,
		"+", "%1", "",
		TableFlag{}, 40, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddI2_Code
		Token::Type::Operator,
		"+", "%2", "",
		TableFlag{}, 40, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddInt_Code
		Token::Type::Operator,
		"+", "%", "",
		TableFlag{}, 40, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// CatStr_Code
		Token::Type::Operator,
		"+", "$", "",
		TableFlag{}, 40, &Str_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Sub_Code
		Token::Type::Operator,
		"-", "", "",
		TableFlag{}, 40, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI1_Code
		Token::Type::Operator,
		"-", "%1", "",
		TableFlag{}, 40, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI2_Code
		Token::Type::Operator,
		"-", "%2", "",
		TableFlag{}, 40, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubInt_Code
		Token::Type::Operator,
		"-", "%", "",
		TableFlag{}, 40, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NegInt_Code
		Token::Type::Operator,
		"-", "U%", "",
		TableFlag{}, 48, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// MulI1_Code
		Token::Type::Operator,
		"*", "%1", "",
		TableFlag{}, 46, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulI2_Code
		Token::Type::Operator,
		"*", "%2", "",
		TableFlag{}, 46, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulInt_Code
		Token::Type::Operator,
		"*", "%", "",
		TableFlag{}, 46, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI1_Code
		Token::Type::Operator,
		"/", "%1", "",
		TableFlag{}, 46, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI2_Code
		Token::Type::Operator,
		"/", "%2", "",
		TableFlag{}, 46, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivInt_Code
		Token::Type::Operator,
		"/", "%", "",
		TableFlag{}, 46, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI1_Code
		Token::Type::Operator,
		"MOD", "%1", "",
		TableFlag{}, 42, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI2_Code
		Token::Type::Operator,
		"MOD", "%2", "",
		TableFlag{}, 42, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModInt_Code
		Token::Type::Operator,
		"MOD", "%", "",
		TableFlag{}, 42, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerI1_Code
		Token::Type::Operator,
		"^", "%1", "",
		TableFlag{}, 50, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerMul_Code
		Token::Type::Operator,
		"^", "*", "",
		TableFlag{}, 50, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerInt_Code
		Token::Type::Operator,
		"^", "%", "",
		TableFlag{}, 50, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI1_Code
		Token::Type::Operator,
		"=", "%1", "",
		TableFlag{}, 30, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI2_Code
		Token::Type::Operator,
		"=", "%2", "",
		TableFlag{}, 30, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqInt_Code
		Token::Type::Operator,
		"=", "%", "",
		TableFlag{}, 30, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqStr_Code
		Token::Type::Operator,
		"=", "$", "",
		TableFlag{}, 30, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI1_Code
		Token::Type::Operator,
		">", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI2_Code
		Token::Type::Operator,
		">", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtInt_Code
		Token::Type::Operator,
		">", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtStr_Code
		Token::Type::Operator,
		">", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI1_Code
		Token::Type::Operator,
		">=", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI2_Code
		Token::Type::Operator,
		">=", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqInt_Code
		Token::Type::Operator,
		">=", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqStr_Code
		Token::Type::Operator,
		">=", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI1_Code
		Token::Type::Operator,
		"<", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI2_Code
		Token::Type::Operator,
		"<", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtInt_Code
		Token::Type::Operator,
		"<", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtStr_Code
		Token::Type::Operator,
		"<", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI1_Code
		Token::Type::Operator,
		"<=", "%1", "",
		TableFlag{}, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI2_Code
		Token::Type::Operator,
		"<=", "%2", "",
		TableFlag{}, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqInt_Code
		Token::Type::Operator,
		"<=", "%", "",
		TableFlag{}, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqStr_Code
		Token::Type::Operator,
		"<=", "$", "",
		TableFlag{}, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI1_Code
		Token::Type::Operator,
		"<>", "%1", "",
		TableFlag{}, 30, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI2_Code
		Token::Type::Operator,
		"<>", "%2", "",
		TableFlag{}, 30, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqInt_Code
		Token::Type::Operator,
		"<>", "%", "",
		TableFlag{}, 30, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqStr_Code
		Token::Type::Operator,
		"<>", "$", "",
		TableFlag{}, 30, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AbsInt_Code
		Token::Type::IntFuncP,
		"ABS(", "%", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// RndArgInt_Code
		Token::Type::IntFuncP,
		"RND(", "%", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// SgnInt_Code
		Token::Type::IntFuncP,
		"SGN(", "%", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// CvtInt_Code
		Token::Type::IntFuncN,
		"", "CvtInt", "",
		Hidden_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// CvtDbl_Code
		Token::Type::IntFuncN,
		"", "CvtDbl", "",
		Hidden_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// StrInt_Code
		Token::Type::IntFuncP,
		"STR$(", "%", "",
		TableFlag{}, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// PrintDbl_Code
		Token::Type{},
		"", "PrintDbl", "",
		Print_Flag | UseConstAsIs_Flag, 2, &None_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintInt_Code
		Token::Type{},
		"", "PrintInt", "",
		Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintStr_Code
		Token::Type{},
		"", "PrintStr", "",
		Print_Flag, 2, &None_Str_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// InputBegin_Code
		Token::Type{},
		"", "InputBegin", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputBeginStr_Code
		Token::Type{},
		"", "InputBeginStr", "Question",
		TableFlag{}, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, inputPromptBeginRecreate
	},
	{	// InputAssign_Code
		Token::Type{},
		"", "InputAssign", "",
		Reference_Flag, 2, &None_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignInt_Code
		Token::Type{},
		"", "InputAssignInt", "",
		Reference_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignStr_Code
		Token::Type{},
		"", "InputAssignStr", "",
		Reference_Flag, 2, &None_Str_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputParse_Code
		Token::Type{},
		"", "InputParse", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseInt_Code
		Token::Type{},
		"", "InputParseInt", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseStr_Code
		Token::Type{},
		"", "InputParseStr", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// Const_Code
		Token::Type::Constant,
		"", "Const", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstInt_Code
		Token::Type::Constant,
		"", "ConstInt", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstStr_Code
		Token::Type::Constant,
		"", "ConstStr", "",
		TableFlag{}, 2, &Str_Str_ExprInfo,
		NULL, constStrEncode, constStrOperandText, constStrRemove,
		constStrRecreate
	},
	{	// Var_Code
		Token::Type::NoParen,
		"", "Var", "",
		TableFlag{}, 2, &Dbl_Dbl_ExprInfo,
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarInt_Code
		Token::Type::NoParen,
		"", "VarInt", "",
		TableFlag{}, 2, &Int_Int_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarStr_Code
		Token::Type::NoParen,
		"", "VarStr", "",
		TableFlag{}, 2, &Str_Str_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// VarRef_Code
		Token::Type::NoParen,
		"", "VarRef", "",
		Reference_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarRefInt_Code
		Token::Type::NoParen,
		"", "VarRefInt", "",
		Reference_Flag, 2, &Int_Int_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarRefStr_Code
		Token::Type::NoParen,
		"", "VarRefStr", "",
		Reference_Flag, 2, &Str_Str_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// Array_Code
		// TODO preliminary until full array support is implemented
		Token::Type::Paren,
		"", "Array", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// DefFuncN_Code
		// TODO preliminary until full define function support is implemented
		Token::Type::DefFuncN,
		"", "DefFuncN", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncP_Code
		// TODO preliminary until full define function support is implemented
		Token::Type::DefFuncP,
		"", "DefFuncP", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// Function_Code
		// TODO preliminary until full user function support is implemented
		Token::Type::Paren,
		"", "Function", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
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
	if (hasFlag((Code)index, Two_Flag) && !entry.name2.empty())
	{
		std::string name {entry.name + ' ' + entry.name2};
		auto iterator = s_nameToEntry.find(name);
		if (iterator != s_nameToEntry.end())  // already in map?
		{
			throw "Multiple two-word command '" + name + '\'';
		}
		s_nameToEntry.emplace(std::move(name), &entry);
		return;
	}

	if (!entry.name.empty())
	{
		auto iterator = s_nameToEntry.find(entry.name);
		if (iterator == s_nameToEntry.end())  // not in table?
		{
			if (entry.type == Token::Type::Operator
				&& entry.exprInfo->m_operandCount == 2
				&& entry.exprInfo->m_operandDataType[0]
				!= entry.exprInfo->m_operandDataType[1])
			{
				throw "Binary operator '" + entry.name + entry.name2
					+ "' not homogeneous";
			}
			s_nameToEntry.emplace(entry.name, &entry);
			if (entry.exprInfo->m_operandCount > 0)
			{
				int index = entry.type == Token::Type::Operator
					? entry.exprInfo->m_operandCount - 1 : 0;
				addExpectedDataType(&entry,
					entry.exprInfo->m_operandDataType[index]);
			}
			return;  // primary code, nothing more to do
		}
		ExprInfo *exprInfo {entry.exprInfo};
		if (exprInfo->m_operandCount > 0
			&& !hasFlag((Code)index, Reference_Flag))
		{
			TableEntry *primary = iterator->second;

			if (exprInfo->m_operandCount < primary->exprInfo->m_operandCount)
			{
				TableEntry *alternate = primary;
				iterator->second = &entry;
				s_alternate[&entry][alternate->exprInfo->m_operandCount - 1]
					.push_back(alternate);
				if (entry.type == Token::Type::IntFuncP)
				{
					// multiple codes; set multiple flag on primary code
					entry.flags |= Multiple_Flag;
					// erase original expected data type of function
					s_expectedDataType.erase(alternate);
				}
				addExpectedDataType(&entry,
					entry.exprInfo->m_operandDataType[0]);
				return;  // new primary code, nothing more to do
			}

			if (entry.type == Token::Type::Operator
				&& exprInfo->m_operandCount > primary->exprInfo->m_operandCount)
			{
				// not the correct primary entry
				// need to get correct primary entry
				auto &vector = s_alternate[primary][exprInfo->m_operandCount
					- 1];
				if (vector.empty())
				{
					if (entry.type == Token::Type::Operator
						&& entry.exprInfo->m_operandCount == 2
						&& entry.exprInfo->m_operandDataType[0]
						!= entry.exprInfo->m_operandDataType[1])
					{
						throw "First binary operator '" + entry.name
							+ entry.name2 + "' not homogeneous";
					}
					vector.push_back(&entry);
					addExpectedDataType(&entry,
						entry.exprInfo->m_operandDataType[0]);
					return;  // first alternate, nothing more to do
				}
				primary = vector.front();
			}

			for (int i = 0; i < primary->exprInfo->m_operandCount; ++i)
			{
				if (exprInfo->m_operandDataType[i]
					!= primary->exprInfo->m_operandDataType[i])
				{
					auto newEntry = &entry;
					do
					{
						TableEntry *newPrimary {};
						for (auto &alternate : s_alternate[primary][i])
						{
							if (exprInfo->m_operandDataType[i]
								== alternate->exprInfo->m_operandDataType[i])
							{
								if (entry.type == Token::Type::Operator
									&& entry.exprInfo->m_operandCount == 2
									&& entry.exprInfo->m_operandDataType[0]
									== entry.exprInfo->m_operandDataType[1])
								{
									s_expectedDataType.erase(alternate);
									addExpectedDataType(newEntry, newEntry
										->exprInfo->m_operandDataType[i]);
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
								&& newEntry->type == Token::Type::Operator
								&& newEntry->exprInfo->m_operandCount == 2
								? newEntry : primary,
								newEntry->exprInfo->m_operandDataType[i]);
							return;  // alternate added, nothing more to do
						}
						primary = newPrimary;  // new primary, next operand
					}
					while (i < exprInfo->m_operandCount);
					break;  // no more operands, all operands match
				}
			}
			if (entry.type == Token::Type::IntFuncP
				&& exprInfo->m_operandCount > primary->exprInfo->m_operandCount)
			{
				// multiple codes; set multiple flag on primary code
				primary->flags |= Multiple_Flag;
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


//================================
//  CODE RELATED TABLE FUNCTIONS
//================================

// returns token type for code
Token::Type Table::type(Code code) const
{
	return m_entry[code].type;
}

// returns primary name for code
const std::string Table::name(Code code) const
{
	return m_entry[code].name;
}

// returns second name of a two word command for code
const std::string Table::name2(Code code) const
{
	return m_entry[code].name2;
}

const std::string Table::optionName(Code code) const
{
	return m_entry[code].option;
}

// returns the debug name for code, which is the primary name
// except for internal functions with multiple argument footprints
const std::string Table::debugName(Code code) const
{
	return code == Invalid_Code
		? "<NotSet>" : m_entry[code].name + m_entry[code].name2;
}

// returns if the flag is set to the code
bool Table::hasFlag(Code code, unsigned flag) const
{
	return m_entry[code].flags & flag ? true : false;
}

// returns the precedence for code
int Table::precedence(Code code) const
{
	return m_entry[code].precedence;
}

// returns data type for code
DataType Table::returnDataType(Code code) const
{
	return m_entry[code].exprInfo->m_returnDataType;
}

// returns the number of operators (arguments) for code
int Table::operandCount(Code code) const
{
	return m_entry[code].exprInfo->m_operandCount;
}

// returns the data type for a specific operator for code
DataType Table::operandDataType(Code code, int operand) const
{
	return m_entry[code].exprInfo->m_operandDataType[operand];
}

// returns the expected data type for table entry
DataType Table::expectedDataType(Code code) const
{
	return s_expectedDataType.at(&m_entry[code]);
}

// temporary function to get alternate code
Code Table::alternateCode(Code code, int index) const
{
	TableEntry *primary = &m_entry[code];
	TableEntry *alternate = s_alternate[primary][index].front();
	return Code(alternate - m_entry);
}

// temporary function to get count of alternate codes
int Table::alternateCodeCount(Code code, int index) const
{
	TableEntry *primary = &m_entry[code];
	return s_alternate[primary][index].size();
}

// returns the pointer to the translate function (if any) for code
TranslateFunction Table::translateFunction(Code code) const
{
	return m_entry[code].translate;
}

// returns the pointer to the encode function (if any) for code
EncodeFunction Table::encodeFunction(Code code) const
{
	return m_entry[code].encode;
}

// returns whether the code has an operand
bool Table::hasOperand(Code code) const
{
	return m_entry[code].operandText;
}

// returns the pointer to the operand text function (if any) for code
OperandTextFunction Table::operandTextFunction(Code code) const
{
	return m_entry[code].operandText;
}

// returns the pointer to the remove function (if any) for code
RemoveFunction Table::removeFunction(Code code) const
{
	return m_entry[code].remove;
}

// returns the pointer to the recreate function (if any) for code
RecreateFunction Table::recreateFunction(Code code) const
{
	return m_entry[code].recreate;
}

//=================================
//  TOKEN RELATED TABLE FUNCTIONS
//=================================

// returns the unary operator code (or Null_Code if none) for token code
Code Table::unaryCode(const TokenPtr &token) const
{
	return token->isType(Token::Type::Operator)
		&& m_entry[token->code()].exprInfo->m_operandCount == 1
		? token->code() : Null_Code;
}

// returns whether the token contains a unary operator code
// (convenience function to avoid confusion)
bool Table::isUnaryOperator(const TokenPtr &token) const
{
	return token->isType(Token::Type::Operator)
		? operandCount(token) == 1 : false;
}

// returns whether the token is a unary or binary operator
// (token type must be operator and have operands)
bool Table::isUnaryOrBinaryOperator(const TokenPtr &token) const
{
	return token->isType(Token::Type::Operator)
		? operandCount(token) > 0 : false;
}

// returns the precedence of the code contained in a token
//
//   - the precedence is obtained from the token
//   - if this is -1 then the precedences if obtained for the token's code
int Table::precedence(const TokenPtr &token) const
{
	int prec {token->precedence()};
	return prec != -1 ? prec : precedence(token->code());
}

// returns the flags of the code contained in a token
//
//   - returns default table flag if the token does not contain a code
bool Table::hasFlag(const TokenPtr &token, unsigned flag) const
{
	// (invalid code tokens have no flags)
	return token->hasValidCode() ? hasFlag(token->code(), flag) : false;
}

// returns number of operands expected for code in token token
int Table::operandCount(const TokenPtr &token) const
{
	return operandCount(token->code());
}

// returns the expected data type for table entry in token
DataType Table::expectedDataType(const TokenPtr &token) const
{
	return expectedDataType(token->code());
}

// function to set a token for a code (code, token type and data type)
void Table::setToken(TokenPtr &token, Code code)
{
	token->setCode(code);
	token->setType(type(code));
	token->setDataType(returnDataType(code));
}

// function to return text for an command, operator or function code in a token
std::string Table::name(const TokenPtr &token) const
{
	TableEntry &entry = m_entry[token->code()];
	std::string string {entry.name};
	if (hasFlag(token, Two_Flag) && !entry.name2.empty())
	{
		string += ' ' + entry.name2;
	}
	return string;
}


// function to find and set code in token for a data type
// and possibly return a conversion code if data type is convertible
//
//   - if data type matches expected data type for operand, no action
//   - else finds associated code that matches data type
//   - if no associated code, returns possible conversion code for data type
//   - if no associated code, returns expected data type

Code Table::findCode(TokenPtr &token, TokenPtr &operandToken, int operandIndex)
{
	DataType expectedDataType {operandDataType(token->code(), operandIndex)};

	if (operandToken->dataType() == expectedDataType     // exact match?
		|| (operandIndex == operandCount(token) - 1      // last operand?
		&& operandToken->isType(Token::Type::Constant)
		&& (expectedDataType == DataType::Double
		|| operandToken->isDataType(DataType::Integer))
		&& !hasFlag(token, UseConstAsIs_Flag)
		&& !operandToken->isDataType(DataType::String)))
	{
		// force token to expected data type and remove double sub-code
		// (applies only to constant token type, but safe for all token types)
		operandToken->setDataType(expectedDataType);
		operandToken->removeSubCode(Double_SubCode);
		if (operandToken->isType(Token::Type::Constant))
		{
			setTokenCode(operandToken, Const_Code);
		}
		return Null_Code;
	}

	// see if any associated code's data types match
	if (setTokenCode(token, token->code(), operandToken->dataType(),
		operandIndex))
	{
		if (operandToken->isType(Token::Type::Constant))
		{
			setTokenCode(operandToken, Const_Code);
		}
		return Null_Code;
	}

	// get a conversion code if no associated code was found
	Code cvtCode {operandToken->convertCode(expectedDataType)};

	// did not find an associated code, return conversion code
	if (cvtCode == Invalid_Code)
	{
		// set operand token to return expected data type
		operandToken->setDataType(expectedDataType);
	}
	else if (operandToken->isType(Token::Type::Constant))
	{
		if (cvtCode == CvtInt_Code
			&& operandToken->isDataType(DataType::Double))
		{
			// double constant can't be converted to an integer
			operandToken->setDataType(DataType{});
			cvtCode = Invalid_Code;
		}
		else
		{
			// numeric constants can be either type, so no conversion is needed
			// force to expected data type
			operandToken->setDataType(expectedDataType);
			operandToken->removeSubCode(Double_SubCode);
			setTokenCode(operandToken, Const_Code);
			cvtCode = Null_Code;
		}
	}
	return cvtCode;  // convert code or invalid
}


// function to set the code for of token for the specified operand
// appropriate for the data type specified
//
//   - if the data type does not match the operand data type of the code,
//     then the associated codes of the code searched for a matching code
//   - if there are no associated codes, or none is found, returns false
//   - upon success, sets the code, type and data type of the token

bool Table::setTokenCode(TokenPtr token, Code code, DataType dataType,
	int operandIndex)
{
	// first check if data type already matches data type of code
	if (dataType == operandDataType(code, operandIndex))
	{
		setToken(token, code);
		return true;
	}
	// if not, see if data type of any associated code matches
	if (s_alternate.find(&m_entry[code]) != s_alternate.end())
	{
		for (auto alternateEntry : s_alternate[&m_entry[code]][operandIndex])
		{
			Code alternateCode = Code(alternateEntry - m_entry);
			if (dataType == operandDataType(alternateCode, operandIndex))
			{
				setToken(token, alternateCode);
				return true;
			}
		}
	}
	return false;  // did not find an alternate code for data type
}


//============================
//  TABLE SPECIFIC FUNCTIONS
//============================

// find function to look for a string in the table
//
//   - returns the index of the entry that is found
//   - returns Invalid_Code if the string was not found in the table

Code Table::find(const std::string &string)
{
	auto iterator = s_nameToEntry.find(string);
	if (iterator != s_nameToEntry.end())
	{
		return Code(iterator->second - tableEntries);
	}
	return Invalid_Code;
}


// end: table.cpp
