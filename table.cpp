// vim:ts=4:sw=4:
//
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

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "erector.h"
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
ExprInfo Null_ExprInfo;
ExprInfo Dbl_None_ExprInfo(DataType::Double);
ExprInfo Dbl_Dbl_ExprInfo(DataType::Double, Operands_Dbl);
static ExprInfo Dbl_DblDbl_ExprInfo(DataType::Double, Operands_DblDbl);
static ExprInfo Dbl_DblInt_ExprInfo(DataType::Double, Operands_DblInt);
static ExprInfo Dbl_Int_ExprInfo(DataType::Double, Operands_Int);
static ExprInfo Dbl_IntDbl_ExprInfo(DataType::Double, Operands_IntDbl);
static ExprInfo Dbl_Str_ExprInfo(DataType::Double, Operands_Str);

ExprInfo Int_None_ExprInfo(DataType::Integer);
static ExprInfo Int_Dbl_ExprInfo(DataType::Integer, Operands_Dbl);
static ExprInfo Int_DblDbl_ExprInfo(DataType::Integer, Operands_DblDbl);
static ExprInfo Int_DblInt_ExprInfo(DataType::Integer, Operands_DblInt);
ExprInfo Int_Int_ExprInfo(DataType::Integer, Operands_Int);
static ExprInfo Int_IntInt_ExprInfo(DataType::Integer, Operands_IntInt);
static ExprInfo Int_IntDbl_ExprInfo(DataType::Integer, Operands_IntDbl);
static ExprInfo Int_Str_ExprInfo(DataType::Integer, Operands_Str);
static ExprInfo Int_StrInt_ExprInfo(DataType::Integer, Operands_StrInt);
static ExprInfo Int_StrStr_ExprInfo(DataType::Integer, Operands_StrStr);
static ExprInfo Int_StrStrInt_ExprInfo(DataType::Integer, Operands_StrStrInt);

ExprInfo Str_None_ExprInfo(DataType::String);
static ExprInfo Str_Dbl_ExprInfo(DataType::String, Operands_Dbl);
static ExprInfo Str_Int_ExprInfo(DataType::String, Operands_Int);
ExprInfo Str_Str_ExprInfo(DataType::String, Operands_Str);
static ExprInfo Str_StrInt_ExprInfo(DataType::String, Operands_StrInt);
static ExprInfo Str_StrStr_ExprInfo(DataType::String, Operands_StrStr);
static ExprInfo Str_StrIntInt_ExprInfo(DataType::String, Operands_StrIntInt);
static ExprInfo Str_StrStrInt_ExprInfo(DataType::String, Operands_StrStrInt);

ExprInfo None_Dbl_ExprInfo(DataType::None, Operands_Dbl);
ExprInfo None_Int_ExprInfo(DataType::None, Operands_Int);
ExprInfo None_Str_ExprInfo(DataType::None, Operands_Str);


Table::EntryVector Table::s_indexToEntry;
Table::NameMap Table::s_nameToEntry;
Table::CodeMap Table::s_codeToEntry;
Table::AlternateMap Table::s_alternate;
Table::ExpectedDataTypeMap Table::s_expectedDataType;


// TODO temporary code index enumeration
enum CodeIndex
{
	Rnd_Code = 1,
	Mod_Code,
	And_Code,
	Or_Code,
	Not_Code,
	Eqv_Code,
	Imp_Code,
	Xor_Code,
	Abs_Code,
	Fix_Code,
	Frac_Code,
	Int_Code,
	RndArg_Code,
	Sgn_Code,
	Cint_Code,
	Cdbl_Code,
	Sqr_Code,
	Atn_Code,
	Cos_Code,
	Sin_Code,
	Tan_Code,
	Exp_Code,
	Log_Code,
	Tab_Code,
	Spc_Code,
	Asc_Code,
	Asc2_Code,
	Chr_Code,
	Instr2_Code,
	Instr3_Code,
	Left_Code,
	Len_Code,
	Mid2_Code,
	Mid3_Code,
	Repeat_Code,
	Right_Code,
	Space_Code,
	Str_Code,
	Val_Code,
	Add_Code,
	Neg_Code,
	Mul_Code,
	Div_Code,
	IntDiv_Code,
	Power_Code,
	Eq_Code,
	Gt_Code,
	GtEq_Code,
	Lt_Code,
	LtEq_Code,
	NotEq_Code,
	OpenParen_Code,
	CloseParen_Code,
	Colon_Code,
	AssignLeft_Code,
	AssignMid2_Code,
	AssignMid3_Code,
	AssignRight_Code,
	AssignKeepLeft_Code,
	AssignKeepMid2_Code,
	AssignKeepMid3_Code,
	AssignKeepRight_Code,
	EOL_Code,
	AddI1_Code,
	AddI2_Code,
	AddInt_Code,
	CatStr_Code,
	Sub_Code,
	SubI1_Code,
	SubI2_Code,
	SubInt_Code,
	NegInt_Code,
	MulI1_Code,
	MulI2_Code,
	MulInt_Code,
	DivI1_Code,
	DivI2_Code,
	DivInt_Code,
	ModI1_Code,
	ModI2_Code,
	ModInt_Code,
	PowerI1_Code,
	PowerMul_Code,
	PowerInt_Code,
	EqI1_Code,
	EqI2_Code,
	EqInt_Code,
	EqStr_Code,
	GtI1_Code,
	GtI2_Code,
	GtInt_Code,
	GtStr_Code,
	GtEqI1_Code,
	GtEqI2_Code,
	GtEqInt_Code,
	GtEqStr_Code,
	LtI1_Code,
	LtI2_Code,
	LtInt_Code,
	LtStr_Code,
	LtEqI1_Code,
	LtEqI2_Code,
	LtEqInt_Code,
	LtEqStr_Code,
	NotEqI1_Code,
	NotEqI2_Code,
	NotEqInt_Code,
	NotEqStr_Code,
	AbsInt_Code,
	RndArgInt_Code,
	SgnInt_Code,
	CvtInt_Code,
	CvtDbl_Code,
	StrInt_Code,
	Array_Code,
	ArrayInt_Code,
	ArrayStr_Code,
	DefFuncN_Code,
	DefFuncNInt_Code,
	DefFuncNStr_Code,
	DefFuncP_Code,
	DefFuncPInt_Code,
	DefFuncPStr_Code,
	Function_Code,
	FunctionInt_Code,
	FunctionStr_Code
};

struct AlternateInfo
{
	CodeIndex primaryCode;					// primary code of alternates
	int index;								// alternate array index
	std::initializer_list<CodeIndex> codes;	// alternate codes
};

std::initializer_list<AlternateInfo> alternateInfo =
{
	// sub-string assignment alternate codes
	{Left_Code, 1, {AssignLeft_Code}},
	{AssignLeft_Code, 0, {AssignKeepLeft_Code}},
	{Mid2_Code, 1, {AssignMid2_Code}},
	{AssignMid2_Code, 0, {AssignKeepMid2_Code}},
	{Mid3_Code, 1, {AssignMid3_Code}},
	{AssignMid3_Code, 0, {AssignKeepMid3_Code}},
	{Right_Code, 1, {AssignRight_Code}},
	{AssignRight_Code, 0, {AssignKeepRight_Code}},

	// codes with operands alternate codes
	{Array_Code, 0, {ArrayInt_Code, ArrayStr_Code}},
	{DefFuncN_Code, 0, {DefFuncNInt_Code, DefFuncNStr_Code}},
	{DefFuncP_Code, 0, {DefFuncPInt_Code, DefFuncPStr_Code}},
	{Function_Code, 0, {FunctionInt_Code, FunctionStr_Code}}
};


// code enumeration names in comments after opening brace
// (code enumeration generated from these by enums.awk)
static Table tableEntries[] =
{
	// CodeIndex{} entry at beginning because CodeIndex{} == 0
	{	// Code{}
		Code::Null,
		"", "NULL", "",
		TableFlag{}, 0, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{	// Rnd_Code
		Code{},
		"RND", "", "",
		Function_Flag, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//--------------------
	//   Word Operators
	//--------------------
	{	// Mod_Code
		Code{},
		"MOD", "", "",
		Operator_Flag, 42, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// And_Code
		Code{},
		"AND", "", "",
		Operator_Flag, 18, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Or_Code
		Code{},
		"OR", "", "",
		Operator_Flag, 14, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Not_Code
		Code{},
		"NOT", "", "",
		Operator_Flag, 20, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Eqv_Code
		Code{},
		"EQV", "", "",
		Operator_Flag, 12, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Imp_Code
		Code{},
		"IMP", "", "",
		Operator_Flag, 10, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Xor_Code
		Code{},
		"XOR", "", "",
		Operator_Flag, 16, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{	// Abs_Code
		Code{},
		"ABS(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Fix_Code
		Code{},
		"FIX(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Frac_Code
		Code{},
		"FRAC(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Int_Code
		Code{},
		"INT(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate

	},
	{	// RndArg_Code
		Code{},
		"RND(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sgn_Code
		Code{},
		"SGN(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cint_Code
		Code{},
		"CINT(", "", "",
		Function_Flag, 2, &Int_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cdbl_Code
		Code{},
		"CDBL(", "", "",
		Function_Flag, 2, &Dbl_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sqr_Code
		Code{},
		"SQR(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Atn_Code
		Code{},
		"ATN(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cos_Code
		Code{},
		"COS(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sin_Code
		Code{},
		"SIN(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tan_Code
		Code{},
		"TAN(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Exp_Code
		Code{},
		"EXP(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Log_Code
		Code{},
		"LOG(", "", "",
		Function_Flag, 2, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tab_Code
		Code{},
		"TAB(", "", "",
		Function_Flag | Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Spc_Code
		Code{},
		"SPC(", "", "",
		Function_Flag | Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Asc_Code
		Code{},
		"ASC(", "", "",
		Function_Flag, 2, &Int_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Asc2_Code
		Code{},
		"ASC(", "2", "",
		Function_Flag, 2, &Int_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Chr_Code
		Code{},
		"CHR$(", "", "",
		Function_Flag, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr2_Code
		Code{},
		"INSTR(", "2", "",
		Function_Flag, 2, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr3_Code
		Code{},
		"INSTR(", "3", "",
		Function_Flag, 2, &Int_StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Left_Code
		Code{},
		"LEFT$(", "", "",
		Function_Flag | SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Len_Code
		Code{},
		"LEN(", "", "",
		Function_Flag, 2, &Int_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid2_Code
		Code{},
		"MID$(", "2", "",
		Function_Flag | SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid3_Code
		Code{},
		"MID$(", "3", "",
		Function_Flag | SubStr_Flag, 2, &Str_StrIntInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Repeat_Code
		Code{},
		"REPEAT$(", "", "",
		Function_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Right_Code
		Code{},
		"RIGHT$(", "", "",
		Function_Flag | SubStr_Flag, 2, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Space_Code
		Code{},
		"SPACE$(", "", "",
		Function_Flag, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Str_Code
		Code{},
		"STR$(", "", "",
		Function_Flag, 2, &Str_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Val_Code
		Code{},
		"VAL(", "", "",
		Function_Flag, 2, &Dbl_Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{	// Add_Code
		Code{},
		"+", "", "",
		Operator_Flag, 40, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Neg_Code
		Code{},
		"-", "U", "",
		Operator_Flag | UseConstAsIs_Flag, 48, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Mul_Code
		Code{},
		"*", "", "",
		Operator_Flag, 46, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Div_Code
		Code{},
		"/", "", "",
		Operator_Flag, 46, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// IntDiv_Code
		Code{},
		"\\", "", "",
		Operator_Flag, 44, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Power_Code
		Code{},
		"^", "", "",
		Operator_Flag | UseConstAsIs_Flag, 50, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Eq_Code
		Code::Equal,
		"=", "", "",
		Operator_Flag, 30, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Gt_Code
		Code{},
		">", "", "",
		Operator_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEq_Code
		Code{},
		">=", "", "",
		Operator_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Lt_Code
		Code{},
		"<", "", "",
		Operator_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEq_Code
		Code{},
		"<=", "", "",
		Operator_Flag, 32, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEq_Code
		Code{},
		"<>", "", "",
		Operator_Flag, 30, &Int_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// OpenParen_Code
		Code::OpenParen,
		"(", "", "",
		Operator_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// CloseParen_Code
		Code::CloseParen,
		")", "", "",
		Operator_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, parenRecreate
	},
	{	// Colon_Code
		Code::Colon,
		":", "", "",
		Operator_Flag | EndStmt_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	{	// AssignLeft_Code
		Code{}, "LEFT$(", "Assign", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid2_Code
		Code{}, "MID$(", "Assign2", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid3_Code
		Code{}, "MID$(", "Assign3", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag, 4, &Str_StrIntInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignRight_Code
		Code{}, "RIGHT$(", "Assign", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepLeft_Code
		Code{}, "LEFT$(", "AssignKeep", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid2_Code
		Code{}, "MID$(", "AssignKeep2", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid3_Code
		Code{}, "MID$(", "AssignKeep3", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepRight_Code
		Code{}, "RIGHT$(", "AssignKeep", "LET",
		Reference_Flag | Command_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// EOL_Code
		Code::EOL,
		"", "EOL", "",
		Operator_Flag | EndStmt_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL
	},
	{	// AddI1_Code
		Code{},
		"+", "%1", "",
		Operator_Flag, 40, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddI2_Code
		Code{},
		"+", "%2", "",
		Operator_Flag, 40, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddInt_Code
		Code{},
		"+", "%", "",
		Operator_Flag, 40, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// CatStr_Code
		Code{},
		"+", "$", "",
		Operator_Flag, 40, &Str_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Sub_Code
		Code{},
		"-", "", "",
		Operator_Flag, 40, &Dbl_DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI1_Code
		Code{},
		"-", "%1", "",
		Operator_Flag, 40, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI2_Code
		Code{},
		"-", "%2", "",
		Operator_Flag, 40, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubInt_Code
		Code{},
		"-", "%", "",
		Operator_Flag, 40, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NegInt_Code
		Code{},
		"-", "U%", "",
		Operator_Flag, 48, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// MulI1_Code
		Code{},
		"*", "%1", "",
		Operator_Flag, 46, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulI2_Code
		Code{},
		"*", "%2", "",
		Operator_Flag, 46, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulInt_Code
		Code{},
		"*", "%", "",
		Operator_Flag, 46, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI1_Code
		Code{},
		"/", "%1", "",
		Operator_Flag, 46, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI2_Code
		Code{},
		"/", "%2", "",
		Operator_Flag, 46, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivInt_Code
		Code{},
		"/", "%", "",
		Operator_Flag, 46, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI1_Code
		Code{},
		"MOD", "%1", "",
		Operator_Flag, 42, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI2_Code
		Code{},
		"MOD", "%2", "",
		Operator_Flag, 42, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModInt_Code
		Code{},
		"MOD", "%", "",
		Operator_Flag, 42, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerI1_Code
		Code{},
		"^", "%1", "",
		Operator_Flag, 50, &Dbl_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerMul_Code
		Code{},
		"^", "*", "",
		Operator_Flag, 50, &Dbl_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerInt_Code
		Code{},
		"^", "%", "",
		Operator_Flag, 50, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI1_Code
		Code{},
		"=", "%1", "",
		Operator_Flag, 30, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI2_Code
		Code{},
		"=", "%2", "",
		Operator_Flag, 30, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqInt_Code
		Code{},
		"=", "%", "",
		Operator_Flag, 30, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqStr_Code
		Code{},
		"=", "$", "",
		Operator_Flag, 30, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI1_Code
		Code{},
		">", "%1", "",
		Operator_Flag, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI2_Code
		Code{},
		">", "%2", "",
		Operator_Flag, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtInt_Code
		Code{},
		">", "%", "",
		Operator_Flag, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtStr_Code
		Code{},
		">", "$", "",
		Operator_Flag, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI1_Code
		Code{},
		">=", "%1", "",
		Operator_Flag, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI2_Code
		Code{},
		">=", "%2", "",
		Operator_Flag, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqInt_Code
		Code{},
		">=", "%", "",
		Operator_Flag, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqStr_Code
		Code{},
		">=", "$", "",
		Operator_Flag, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI1_Code
		Code{},
		"<", "%1", "",
		Operator_Flag, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI2_Code
		Code{},
		"<", "%2", "",
		Operator_Flag, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtInt_Code
		Code{},
		"<", "%", "",
		Operator_Flag, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtStr_Code
		Code{},
		"<", "$", "",
		Operator_Flag, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI1_Code
		Code{},
		"<=", "%1", "",
		Operator_Flag, 32, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI2_Code
		Code{},
		"<=", "%2", "",
		Operator_Flag, 32, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqInt_Code
		Code{},
		"<=", "%", "",
		Operator_Flag, 32, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqStr_Code
		Code{},
		"<=", "$", "",
		Operator_Flag, 32, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI1_Code
		Code{},
		"<>", "%1", "",
		Operator_Flag, 30, &Int_IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI2_Code
		Code{},
		"<>", "%2", "",
		Operator_Flag, 30, &Int_DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqInt_Code
		Code{},
		"<>", "%", "",
		Operator_Flag, 30, &Int_IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqStr_Code
		Code{},
		"<>", "$", "",
		Operator_Flag, 30, &Int_StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AbsInt_Code
		Code{},
		"ABS(", "%", "",
		Function_Flag, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// RndArgInt_Code
		Code{},
		"RND(", "%", "",
		Function_Flag, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// SgnInt_Code
		Code{},
		"SGN(", "%", "",
		Function_Flag, 2, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// CvtInt_Code
		Code::CvtInt,
		"", "CvtInt", "",
		Function_Flag | NoName_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// CvtDbl_Code
		Code::CvtDbl,
		"", "CvtDbl", "",
		Function_Flag | NoName_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// StrInt_Code
		Code{},
		"STR$(", "%", "",
		Function_Flag, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Array_Code
		Code::Array,
		"", "Array", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		arrayRecreate
	},
	{	// ArrayInt_Code
		Code::Array,
		"", "ArrayInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		arrayRecreate
	},
	{	// ArrayStr_Code
		Code::Array,
		"", "ArrayStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		arrayRecreate
	},
	{	// DefFuncN_Code
		Code::DefFuncNoArgs,
		"", "DefFuncN", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		defineFunctionRecreate
	},
	{	// DefFuncNInt_Code
		Code::DefFuncNoArgs,
		"", "DefFuncNInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		defineFunctionRecreate
	},
	{	// DefFuncNStr_Code
		Code::DefFuncNoArgs,
		"", "DefFuncNStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		defineFunctionRecreate
	},
	{	// DefFuncP_Code
		Code::DefFunc,
		"", "DefFuncP", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		defineFunctionRecreate
	},
	{	// DefFuncPInt_Code
		Code::DefFunc,
		"", "DefFuncPInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		defineFunctionRecreate
	},
	{	// DefFuncPStr_Code
		Code::DefFunc,
		"", "DefFuncPStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		defineFunctionRecreate
	},
	{	// Function_Code
		Code::UserFunc,
		"", "Function", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		functionRecreate
	},
	{	// FunctionInt_Code
		Code::UserFunc,
		"", "FunctionInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		functionRecreate
	},
	{	// FunctionStr_Code
		Code::UserFunc,
		"", "FunctionStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		functionRecreate
	}
};


// TODO temporary table instance
static Table table;


// TODO temporary constructor to initialize other alternates
Table::Table()
{
	// manually set up other alternates (temporary)
	for (auto info : alternateInfo)
	{
		auto primary = &tableEntries[info.primaryCode];
		for (CodeIndex codeIndex : info.codes)
		{
			auto alternate = &tableEntries[codeIndex];
			primary->appendAlternate(info.index, alternate);
		}
	}
}


//========================
//  TABLE ENTRY FUNCTIONS
//========================

Table::Table(Code code, const char *name, const char *name2, const char *option,
		unsigned flags, int precedence, ExprInfo *exprInfo,
		OperandType operandType,
		RecreateFunction _recreate) :
	m_code {code},
	m_name {name},
	m_name2 {name2},
	m_option {option},
	m_flags {flags},
	m_precedence {precedence},
	m_exprInfo {exprInfo},
	m_operandType {operandType},
	m_recreate {_recreate}
{
	Erector{this}();
}


// constructor for existing table entry array elements
Table::Table(Code code, const char *name, const char *name2, const char *option,
		unsigned flags, int precedence, ExprInfo *exprInfo,
		void *, void *, void *, void *,
		RecreateFunction _recreate) :
	Table {code, name, name2, option, flags, precedence, exprInfo,
		No_OperandType, _recreate}
{
}

Table::Table(Code code, const char *name, const char *name2, const char *option,
		unsigned flags, int precedence, ExprInfo *exprInfo,
		RecreateFunction _recreate) :
	Table {code, name, name2, option, flags, precedence, exprInfo,
		NumberOf_OperandType, _recreate}
{
}

Table::Table(BaseInfo baseInfo, TypeInfo typeInfo, const char *option,
		int precedence, OperandType operandType, unsigned moreFlags,
		const AlternateItem &alternateItem = {}) :
	Table {baseInfo.code, baseInfo.name, typeInfo.name2, option,
		baseInfo.flags | moreFlags, precedence, typeInfo.exprInfo, operandType}
{
	appendAlternate(alternateItem);
}

Table::Table(BaseInfo baseInfo, TypeInfo typeInfo, int precedence,
		OperandType operandType, unsigned moreFlags,
		const AlternateItem &alternateItem = {}) :
	Table {baseInfo, typeInfo, "", precedence, operandType, moreFlags,
		alternateItem}
{
}


std::string Table::commandName() const
{
	std::string string {m_name};
	if (isTwoWordCommand())
	{
		string += ' ' + m_name2;
	}
	return string;
}


int Table::hasBinaryOperator()
{
	return s_alternate[this][BinaryOperator].size() > 0;
}


void Table::addToExpectedDataType(DataType dataType) noexcept
{
	auto iterator = findExpectedDataType();
	if (iterator == expectedDataTypeNotFound())
	{
		setExpectedDataType(dataType);
	}
	else if (iterator->second == DataType::Double
			 || iterator->second == DataType::Integer)
	{
		iterator->second = DataType::Number;
	}
}


DataType Table::expectedDataType()
{
	return s_expectedDataType.at(this);
}


Table *Table::alternateForReturn(DataType returnDataType)
{
	if (returnDataType != m_exprInfo->m_returnDataType
		&& findAlternate() != alternateNotFound())
	{
		for (Table *alternateEntry : alternateVector(0))
		{
			if (returnDataType == alternateEntry->returnDataType())
			{
				return alternateEntry;
			}
		}
	}
	return this;  // use entry if no alternate
}


Table *Table::firstAlternate(int operand)
{
	return s_alternate[this][operand].front();
}


Table *Table::alternateForOperand(int operand, DataType operandDataType)
{
	if (operandDataType == m_exprInfo->m_operandDataType[operand])
	{
		return this;
	}

	if (findAlternate() != alternateNotFound())
	{
		for (Table *alternate : alternateVector(operand))
		{
			if (operandDataType == alternate->operandDataType(operand))
			{
				return alternate;
			}
		}
	}
	return {};
}


//==========================
//  STATIC TABLE FUNCTIONS
//==========================

Table *Table::entry(Code code)
{
	return s_codeToEntry[code];
}


Table *Table::entry(Code code, DataType dataType)
{
	return s_codeToEntry[code]->alternateForReturn(dataType);
}


Table *Table::find(const std::string &string)
{
	auto iterator = s_nameToEntry.find(string);
	if (iterator != s_nameToEntry.end())
	{
		return iterator->second;
	}
	return {};
}


// end: table.cpp
