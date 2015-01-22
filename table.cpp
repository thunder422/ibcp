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


Table::NameMap Table::s_nameToEntry;		// name to code table map
Table::CodeMap Table::s_codeToEntry;
std::unordered_map<TableEntry *, Table::EntryVectorArray> Table::s_alternate;
std::unordered_map<TableEntry *, DataType> Table::s_expectedDataType;


// TODO temporary code index enumeration
enum CodeIndex
{
	Let_Code = 1,
	Print_Code,
	Input_Code,
	InputPrompt_Code,
	Dim_Code,
	Def_Code,
	Rem_Code,
	If_Code,
	Then_Code,
	Else_Code,
	End_Code,
	EndIf_Code,
	For_Code,
	To_Code,
	Step_Code,
	Next_Code,
	Do_Code,
	DoWhile_Code,
	DoUntil_Code,
	While_Code,
	Until_Code,
	Loop_Code,
	LoopWhile_Code,
	LoopUntil_Code,
	Rnd_Code,
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
	Comma_Code,
	SemiColon_Code,
	Colon_Code,
	RemOp_Code,
	Assign_Code,
	AssignInt_Code,
	AssignStr_Code,
	AssignLeft_Code,
	AssignMid2_Code,
	AssignMid3_Code,
	AssignRight_Code,
	AssignList_Code,
	AssignListInt_Code,
	AssignListStr_Code,
	AssignKeepStr_Code,
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
	PrintDbl_Code,
	PrintInt_Code,
	PrintStr_Code,
	InputBegin_Code,
	InputBeginStr_Code,
	InputAssign_Code,
	InputAssignInt_Code,
	InputAssignStr_Code,
	InputParse_Code,
	InputParseInt_Code,
	InputParseStr_Code,
	Const_Code,
	ConstInt_Code,
	ConstStr_Code,
	Var_Code,
	VarInt_Code,
	VarStr_Code,
	VarRef_Code,
	VarRefInt_Code,
	VarRefStr_Code,
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
	// CodeIndex{} entry at beginning because CodeIndex{} == 0
	{	// Code{}
		Code::Null,
		"", "NULL", "",
		TableFlag{}, 0, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	//--------------
	//   Commands
	//--------------
	{	// Let_Code
		Code::Let,
		"LET", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		letTranslate, NULL, NULL, NULL, NULL
	},
	{	// Print_Code
		Code{},
		"PRINT", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		printTranslate, NULL, NULL, NULL, printRecreate
	},
	{	// Input_Code
		Code{},
		"INPUT", "", "Keep",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// InputPrompt_Code
		Code{},
		"INPUT", "PROMPT", "Keep",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// Dim_Code
		Code{},
		"DIM", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Def_Code
		Code{},
		"DEF", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Rem_Code
		Code::Rem,
		"REM", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	{	// If_Code
		Code{},
		"IF", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Then_Code
		Code{},
		"THEN", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Else_Code
		Code{},
		"ELSE", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// End_Code
		Code{},
		"END", "", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// EndIf_Code
		Code{},
		"END", "IF", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// For_Code
		Code{},
		"FOR", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// To_Code
		Code{},
		"TO", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Step_Code
		Code{},
		"STEP", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Next_Code
		Code{},
		"NEXT", "", "",
		Command_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Do_Code
		Code{},
		"DO", "", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoWhile_Code
		Code{},
		"DO", "WHILE", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoUntil_Code
		Code{},
		"DO", "UNTIL", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// While_Code
		Code{},
		"WHILE", "", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Until_Code
		Code{},
		"UNTIL", "", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Loop_Code
		Code{},
		"LOOP", "", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopWhile_Code
		Code{},
		"LOOP", "WHILE", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopUntil_Code
		Code{},
		"LOOP", "UNTIL", "",
		Command_Flag | Two_Flag, 4, &Null_ExprInfo,
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
		Operator_Flag | Two_Flag, 32, &Int_DblDbl_ExprInfo,
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
		Operator_Flag | Two_Flag, 32, &Int_DblDbl_ExprInfo,
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
		Operator_Flag | Two_Flag, 30, &Int_DblDbl_ExprInfo,
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
	{	// Comma_Code
		Code::Comma,
		",", "", "",
		Operator_Flag | Command_Flag, 6, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, printCommaRecreate
	},
	{	// SemiColon_Code
		Code::Semicolon,
		";", "", "",
		Operator_Flag | Command_Flag, 6, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, printSemicolonRecreate
	},
	{	// Colon_Code
		Code::Colon,
		":", "", "",
		Operator_Flag | EndStmt_Flag, 4, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// RemOp_Code
		Code::RemOp,
		"'", "", "",
		Operator_Flag | EndStmt_Flag, 2, &Null_ExprInfo,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	{	// Assign_Code
		Code{},
		"", "Assign", "LET",
		Reference_Flag | Command_Flag, 4, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignInt_Code
		Code{},
		"", "Assign%", "LET",
		Reference_Flag | Command_Flag, 4, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignStr_Code
		Code{},
		"", "Assign$", "LET",
		Reference_Flag | Command_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignLeft_Code
		Code{},
		"LEFT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag | Command_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid2_Code
		Code{},
		"MID$(", "Assign2", "LET",
		Reference_Flag | SubStr_Flag | Command_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid3_Code
		Code{},
		"MID$(", "Assign3", "LET",
		Reference_Flag | SubStr_Flag | Command_Flag, 4, &Str_StrIntInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignRight_Code
		Code{},
		"RIGHT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignList_Code
		Code{},
		"", "AssignList", "LET",
		Reference_Flag | Command_Flag, 4, &Dbl_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListInt_Code
		Code{},
		"", "AssignList%", "LET",
		Reference_Flag | Command_Flag, 4, &Int_Int_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListStr_Code
		Code{},
		"", "AssignList$", "LET",
		Reference_Flag | Command_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignKeepStr_Code
		Code{},
		"", "AssignKeep$", "LET",
		Reference_Flag | Keep_Flag, 4, &Str_Str_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepLeft_Code
		Code{},
		"LEFT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid2_Code
		Code{},
		"MID$(", "AssignKeep2", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid3_Code
		Code{},
		"MID$(", "AssignKeep3", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepRight_Code
		Code{},
		"RIGHT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, &Str_StrInt_ExprInfo,
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
		Function_Flag | Hidden_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// CvtDbl_Code
		Code::CvtDbl,
		"", "CvtDbl", "",
		Function_Flag | Hidden_Flag, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// StrInt_Code
		Code{},
		"STR$(", "%", "",
		Function_Flag, 2, &Str_Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// PrintDbl_Code
		Code{},
		"", "PrintDbl", "",
		Print_Flag | UseConstAsIs_Flag, 2, &None_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintInt_Code
		Code{},
		"", "PrintInt", "",
		Print_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintStr_Code
		Code{},
		"", "PrintStr", "",
		Print_Flag, 2, &None_Str_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// InputBegin_Code
		Code{},
		"", "InputBegin", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputBeginStr_Code
		Code{},
		"", "InputBeginStr", "Question",
		TableFlag{}, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, inputPromptBeginRecreate
	},
	{	// InputAssign_Code
		Code{},
		"", "InputAssign", "",
		Reference_Flag, 2, &None_Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignInt_Code
		Code{},
		"", "InputAssignInt", "",
		Reference_Flag, 2, &None_Int_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignStr_Code
		Code{},
		"", "InputAssignStr", "",
		Reference_Flag, 2, &None_Str_ExprInfo,
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputParse_Code
		Code{},
		"", "InputParse", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseInt_Code
		Code{},
		"", "InputParseInt", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseStr_Code
		Code{},
		"", "InputParseStr", "",
		TableFlag{}, 2, &Null_ExprInfo,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// Const_Code
		Code::Constant,
		"", "Const", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstInt_Code
		Code::Constant,
		"", "ConstInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstStr_Code
		Code::Constant,
		"", "ConstStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, constStrEncode, constStrOperandText, constStrRemove,
		constStrRecreate
	},
	{	// Var_Code
		Code::Variable,
		"", "Var", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarInt_Code
		Code::Variable,
		"", "VarInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarStr_Code
		Code::Variable,
		"", "VarStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// VarRef_Code
		Code::Variable,
		"", "VarRef", "",
		Reference_Flag, 2, &Dbl_None_ExprInfo,
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarRefInt_Code
		Code::Variable,
		"", "VarRefInt", "",
		Reference_Flag, 2, &Int_None_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarRefStr_Code
		Code::Variable,
		"", "VarRefStr", "",
		Reference_Flag, 2, &Str_None_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// Array_Code
		Code::Array,
		"", "Array", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// ArrayInt_Code
		Code::Array,
		"", "ArrayInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// ArrayStr_Code
		Code::Array,
		"", "ArrayStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// DefFuncN_Code
		Code::DefFuncNoArgs,
		"", "DefFuncN", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncNInt_Code
		Code::DefFuncNoArgs,
		"", "DefFuncNInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncNStr_Code
		Code::DefFuncNoArgs,
		"", "DefFuncNStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncP_Code
		Code::DefFunc,
		"", "DefFuncP", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncPInt_Code
		Code::DefFunc,
		"", "DefFuncPInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncPStr_Code
		Code::DefFunc,
		"", "DefFuncPStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// Function_Code
		Code::UserFunc,
		"", "Function", "",
		TableFlag{}, 2, &Dbl_None_ExprInfo,
		NULL, NULL, NULL, NULL, functionRecreate
	},
	{	// FunctionInt_Code
		Code::UserFunc,
		"", "FunctionInt", "",
		TableFlag{}, 2, &Int_None_ExprInfo,
		NULL, NULL, NULL, NULL, functionRecreate
	},
	{	// FunctionStr_Code
		Code::UserFunc,
		"", "FunctionStr", "",
		TableFlag{}, 2, &Str_None_ExprInfo,
		NULL, NULL, NULL, NULL, functionRecreate
	}
};


// TODO temporary table instance
static Table table;


// constructor function that initializes the table instance variables
//
//   - fatally aborts application if table entry errors were detected
//   - table entry errors are output before aborting

Table::Table()
{
	// iterate entries and build alternate code map
	for (unsigned i = 0; i < sizeof(tableEntries) / sizeof(TableEntry); i++)
	{
		tableEntries[i].addToTable();
	}

	// manually set up other alternates (temporary)
	for (auto info : alternateInfo)
	{
		auto primary = &tableEntries[info.primaryCode];
		for (CodeIndex codeIndex : info.codes)
		{
			auto alternate = &tableEntries[codeIndex];
			s_alternate[primary][info.index].push_back(alternate);
		}
	}
}


//========================
//  TABLE ENTRY FUNCTIONS
//========================

int TableEntry::index() const
{
	return this - tableEntries;
}


struct Done {};

inline void TableEntry::addToTable() noexcept
try
{
	addToCodeMap();
	if (!addTwoWordCommand() && !m_name.empty())
	{
		auto iterator = Table::s_nameToEntry.find(m_name);
		if (iterator == Table::s_nameToEntry.end())
		{
			addPrimaryCodeToNameMap();
		}
		else if (m_exprInfo->m_operandCount > 0 && !hasFlag(Reference_Flag))
		{
			TableEntry *primary = setNewPrimaryOrGetPrimary(iterator->second);

			if (isOperator() && m_exprInfo->m_operandCount
				> primary->m_exprInfo->m_operandCount)
			{
				// not the correct primary entry
				primary = getCorrectPrimary(primary);
			}
			primary = addAlternateOrGetNewPrimary(primary);
			checkIfMulipleFunctionEntry(primary);
		}
	}
}
catch (Done) {}
catch (std::string &error)
{
	std::cerr << "Table Error: " << error << std::endl;
	abort();
}


inline void TableEntry::addToCodeMap() noexcept
{
	if (m_code != Code{})
	{
		auto iterator = Table::s_codeToEntry.find(m_code);
		if (iterator == Table::s_codeToEntry.end())
		{
			Table::s_codeToEntry[m_code] = this;
		}
	}
}


inline bool TableEntry::addTwoWordCommand()
{
	if (hasFlag(Two_Flag) && !m_name2.empty())
	{
		std::string name {m_name + ' ' + m_name2};
		auto iterator = Table::s_nameToEntry.find(name);
		if (iterator != Table::s_nameToEntry.end())
		{
			throw "Multiple two-word command '" + name + '\'';
		}
		Table::s_nameToEntry.emplace(std::move(name), this);
		return true;
	}
	return false;
}


inline void TableEntry::addPrimaryCodeToNameMap()
{
	if (isOperator() && m_exprInfo->m_operandCount == 2
		&& m_exprInfo->m_operandDataType[0]
		!= m_exprInfo->m_operandDataType[1])
	{
		throw "Binary operator '" + m_name + m_name2 + "' not homogeneous";
	}
	Table::s_nameToEntry.emplace(m_name, this);
	if (m_exprInfo->m_operandCount > 0)
	{
		int index = isOperator() ? m_exprInfo->m_operandCount - 1 : 0;
		addExpectedDataType(m_exprInfo->m_operandDataType[index]);
	}
}

inline TableEntry *TableEntry::setNewPrimaryOrGetPrimary(TableEntry *primary)
{
	if (m_exprInfo->m_operandCount < primary->m_exprInfo->m_operandCount)
	{
		TableEntry *alternate = primary;
		primary = this;
		Table::s_alternate[this][alternate->lastOperand()].push_back(alternate);
		if (isFunction())
		{
			m_flags |= Multiple_Flag;
			Table::s_expectedDataType.erase(alternate);
		}
		addExpectedDataType(m_exprInfo->m_operandDataType[0]);
		throw Done{};
	}
	return primary;
}


inline TableEntry *TableEntry::getCorrectPrimary(TableEntry *primary)
{
	Table::EntryVector &vector = Table::s_alternate[primary][lastOperand()];
	if (vector.empty())
	{
		if (isOperator() && m_exprInfo->m_operandCount == 2
			&& m_exprInfo->m_operandDataType[0]
			!= m_exprInfo->m_operandDataType[1])
		{
			throw "First binary operator '" + m_name + m_name2
				+ "' not homogeneous";
		}
		vector.push_back(this);
		addExpectedDataType(m_exprInfo->m_operandDataType[0]);
		throw Done{};
	}
	return vector.front();
}


inline TableEntry *TableEntry::addAlternateOrGetNewPrimary(TableEntry *primary)
{
	for (int i = 0; i < primary->m_exprInfo->m_operandCount; ++i)
	{
		if (m_exprInfo->m_operandDataType[i]
			!= primary->m_exprInfo->m_operandDataType[i])
		{
			auto newEntry = this;
			do
			{
				TableEntry *newPrimary {};
				for (auto &alternate : Table::s_alternate[primary][i])
				{
					if (m_exprInfo->m_operandDataType[i]
						== alternate->m_exprInfo->m_operandDataType[i])
					{
						if (isOperator() && m_exprInfo->m_operandCount == 2
							&& m_exprInfo->m_operandDataType[0]
							== m_exprInfo->m_operandDataType[1])
						{
							Table::s_expectedDataType.erase(alternate);
							newEntry->addExpectedDataType(newEntry
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
					Table::s_alternate[primary][i].push_back(newEntry);
					(i == 0 && newEntry->isOperator()
						&& newEntry->m_exprInfo->m_operandCount == 2
						? newEntry : primary)->addExpectedDataType(
						newEntry->m_exprInfo->m_operandDataType[i]);
					throw Done{};
				}
				primary = newPrimary;  // new primary, next operand
			}
			while (i < m_exprInfo->m_operandCount);
			break;  // no more operands, all operands match
		}
	}
	return primary;
}


void TableEntry::addExpectedDataType(DataType dataType) noexcept
{
	DataType current = Table::s_expectedDataType[this];
	if (current == DataType{})
	{
		Table::s_expectedDataType[this] = dataType;
	}
	else if (current == DataType::Double || current == DataType::Integer)
	{
		Table::s_expectedDataType[this] = DataType::Number;
	}
}


inline void TableEntry::checkIfMulipleFunctionEntry(TableEntry *primary)
{
	if (isFunction()
		&& m_exprInfo->m_operandCount > primary->m_exprInfo->m_operandCount)
	{
		// multiple codes; set multiple flag on primary code
		primary->m_flags |= Multiple_Flag;
		Table::s_alternate[primary][m_exprInfo->m_operandCount - 1]
			.push_back(this);
	}
	else
	{
		throw "Multiple entries with same operand data types ("
			+ std::to_string(this - tableEntries) + ','
			+ std::to_string(primary - tableEntries) + ')';
	}
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


//==========================
//  STATIC TABLE FUNCTIONS
//==========================

TableEntry *Table::entry(Code code)
{
	return s_codeToEntry[code];
}


TableEntry *Table::entry(Code code, DataType dataType)
{
	return s_codeToEntry[code]->alternate(dataType);
}


TableEntry *Table::entry(int index)
{
	return &tableEntries[index];
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
