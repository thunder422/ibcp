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

#include <unordered_map>

#include <QChar>
#include <QString>
#include <QStringList>

#include "table.h"
#include "utility.h"
#include "basic/basic.h"


// expression information for operators and internal functions
struct ExprInfo
{
	short m_operandCount;			// number of operands (operators/functions)
	short m_associatedCodeCount;	// number of associated codes
	short m_secondAssociatedIndex;	// index of second operand associated codes
	DataType *m_operandDataType;	// data type of each operand
	Code *m_associatedCode;			// associated codes
	DataType m_expectedDataType;	// expected data type of next operand

	ExprInfo(short operandCount = 0,
		DataType *operandDataType = NULL, short associatedCodeCount = 0,
		short secondAssociatedIndex = 0, Code *associatedCode = NULL,
		DataType expectedDataType = DataType::None) :
		m_operandCount(operandCount),
		m_associatedCodeCount(associatedCodeCount),
		m_secondAssociatedIndex(secondAssociatedIndex),
		m_operandDataType(operandDataType),
		m_associatedCode(associatedCode),
		m_expectedDataType(expectedDataType)
	{
	}
};


struct TableEntry
{
	Token::Type type;				// type of token for entry
	const std::string name;			// name for table entry
	const std::string name2;		// name of second word of command
	const std::string option;		// name of option sub-code
	unsigned flags;					// flags for entry
	int precedence;					// precedence of code
	DataType dataType;				// next expression data type for command
	ExprInfo *exprInfo;				// expression info pointer (NULL for none)
	TranslateFunction translate;	// pointer to translate function
	EncodeFunction encode;			// pointer to encode function
	OperandTextFunction operandText;// pointer to operand text function
	RemoveFunction remove;			// pointer to remove function
	RecreateFunction recreate;		// pointer to recreate function
};


Table *Table::s_instance;			// pointer to single table instance


// this macro produces two entries for the ExprInfo constructor,
// one for the number of operands, one for the pointer to the
// operand data type array
#define Operands(type)  (sizeof(type ## _OperandArray) \
	/ sizeof(type ## _OperandArray[0])), type ## _OperandArray

// macro produces associated code fields for table entries
#define AssocCode(code)  (sizeof(code ## _AssocCode) \
	/ sizeof(code ## _AssocCode[0])), 0, code ## _AssocCode

// macro produces associated code fields with secondary index for table entries
#define AssocCode2(code, index2)  (sizeof(code ## _AssocCode) \
	/ sizeof(code ## _AssocCode[0])), index2, code ## _AssocCode


// operand data type arrays
static DataType Dbl_OperandArray[] = {
	DataType::Double
};
static DataType DblDbl_OperandArray[] = {
	DataType::Double, DataType::Double
};
static DataType DblInt_OperandArray[] = {
	DataType::Double, DataType::Integer
};

static DataType Int_OperandArray[] = {
	DataType::Integer
};
static DataType IntDbl_OperandArray[] = {
	DataType::Integer, DataType::Double
};
static DataType IntInt_OperandArray[] = {
	DataType::Integer, DataType::Integer
};

static DataType Str_OperandArray[] = {
	DataType::String
};
static DataType StrInt_OperandArray[] = {
	DataType::String, DataType::Integer
};
static DataType StrIntInt_OperandArray[] = {
	DataType::String, DataType::Integer, DataType::Integer
};
static DataType StrStr_OperandArray[] = {
	DataType::String, DataType::String
};
static DataType StrStrInt_OperandArray[] = {
	DataType::String, DataType::String, DataType::Integer
};


// associated code data type arrays
static Code Abs_AssocCode[]				= {AbsInt_Code};
static Code Add_AssocCode[]				= {
	AddInt_Code, CatStr_Code, AddI2_Code
};
static Code AddInt_AssocCode[]			= {AddI1_Code};
static Code Assign_AssocCode[]			= {
	AssignInt_Code, AssignStr_Code, AssignList_Code
};
static Code AssignInt_AssocCode[]		= {AssignListInt_Code};
static Code AssignLeft_AssocCode[]		= {AssignKeepLeft_Code};
static Code AssignMid2_AssocCode[]		= {AssignKeepMid2_Code};
static Code AssignMid3_AssocCode[]		= {AssignKeepMid3_Code};
static Code AssignRight_AssocCode[]		= {AssignKeepRight_Code};
static Code AssignStr_AssocCode[]		= {
	AssignKeepStr_Code, AssignListStr_Code
};
static Code Const_AssocCode[]			= {ConstInt_Code, ConstStr_Code};
static Code Div_AssocCode[]				= {DivInt_Code, DivI2_Code};
static Code DivInt_AssocCode[]			= {DivI1_Code};
static Code Eq_AssocCode[]				= {
	EqInt_Code, EqStr_Code, EqI2_Code
};
static Code EqInt_AssocCode[]			= {EqI1_Code};
static Code Gt_AssocCode[]				= {
	GtInt_Code, GtStr_Code, GtI2_Code
};
static Code GtInt_AssocCode[]			= {GtI1_Code};
static Code GtEq_AssocCode[]			= {
	GtEqInt_Code, GtEqStr_Code, GtEqI2_Code
};
static Code GtEqInt_AssocCode[]			= {GtEqI1_Code};
static Code InputAssign_AssocCode[]		= {
	InputAssignInt_Code, InputAssignStr_Code, InputParse_Code
};
static Code InputAssignInt_AssocCode[]	= {InputParseInt_Code};
static Code InputAssignStr_AssocCode[]	= {InputParseStr_Code};
static Code Left_AssocCode[]			= {AssignLeft_Code};
static Code Lt_AssocCode[]				= {
	LtInt_Code, LtStr_Code, LtI2_Code
};
static Code LtInt_AssocCode[]			= {LtI1_Code};
static Code LtEq_AssocCode[]			= {
	LtEqInt_Code, LtEqStr_Code, LtEqI2_Code
};
static Code LtEqInt_AssocCode[]			= {LtEqI1_Code};
static Code Mid2_AssocCode[]			= {AssignMid2_Code};
static Code Mid3_AssocCode[]			= {AssignMid3_Code};
static Code Mod_AssocCode[]				= {ModInt_Code, ModI2_Code};
static Code ModInt_AssocCode[]			= {ModI1_Code};
static Code Mul_AssocCode[]				= {MulInt_Code, MulI2_Code};
static Code MulInt_AssocCode[]			= {MulI1_Code};
static Code Neg_AssocCode[]				= {NegInt_Code, Sub_Code};
static Code NotEq_AssocCode[]			= {
	NotEqInt_Code, NotEqStr_Code, NotEqI2_Code
};
static Code NotEqInt_AssocCode[] 		= {NotEqI1_Code};
static Code Power_AssocCode[]			= {PowerInt_Code, PowerMul_Code};
static Code PowerInt_AssocCode[]		= {PowerI1_Code};
static Code Print_AssocCode[]			= {
	PrintInt_Code, PrintStr_Code
};
static Code Right_AssocCode[]			= {AssignRight_Code};
static Code RndArgs_AssocCode[]			= {RndArgInt_Code};
static Code Sgn_AssocCode[]				= {SgnInt_Code};
static Code Str_AssocCode[]				= {StrInt_Code};
static Code Sub_AssocCode[]				= {SubInt_Code, SubI2_Code};
static Code SubInt_AssocCode[]			= {SubI1_Code};
static Code Var_AssocCode[]				= {VarInt_Code, VarStr_Code};
static Code VarRef_AssocCode[]			= {VarRefInt_Code, VarRefStr_Code};


// standard expression information structures
static ExprInfo Dbl_ExprInfo(Operands(Dbl));
static ExprInfo DblDbl_ExprInfo(Operands(DblDbl));
static ExprInfo DblInt_ExprInfo(Operands(DblInt));

static ExprInfo Int_ExprInfo(Operands(Int));
static ExprInfo IntInt_ExprInfo(Operands(IntInt));
static ExprInfo IntDbl_ExprInfo(Operands(IntDbl));

static ExprInfo Str_ExprInfo(Operands(Str));
static ExprInfo StrInt_ExprInfo(Operands(StrInt));
static ExprInfo StrStr_ExprInfo(Operands(StrStr));
static ExprInfo StrStrInt_ExprInfo(Operands(StrStrInt));

static ExprInfo StrIntInt_ExprInfo(Operands(StrIntInt));

// code enumeration names in comments after opening brace
// (code enumeration generated from these by enums.awk)
static TableEntry tableEntries[] =
{
	// Null_Code entry at beginning so Null_Code == 0
	{	// Null_Code
		Token::Type::Operator,
		"", "NULL", "",
		TableFlag{}, 0, DataType{}, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	//***********************
	//   BEGIN PLAIN WORDS
	//***********************
	{	// BegPlainWord_Code
		Token::Type{},
		"", "NULL", "",
		TableFlag{}, 0, DataType{}, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	//--------------
	//   Commands
	//--------------
	{	// Let_Code
		Token::Type::Command,
		"LET", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		letTranslate, NULL, NULL, NULL, NULL
	},
	{	// Print_Code
		Token::Type::Command,
		"PRINT", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		printTranslate, NULL, NULL, NULL, printRecreate
	},
	{	// Input_Code
		Token::Type::Command,
		"INPUT", "", "Keep",
		Two_Flag, 4, DataType::None, NULL,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// InputPrompt_Code
		Token::Type::Command,
		"INPUT", "PROMPT", "Keep",
		Two_Flag, 4, DataType::String, NULL,
		inputTranslate, NULL, NULL, NULL, inputRecreate

	},
	{	// Dim_Code
		Token::Type::Command,
		"DIM", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Def_Code
		Token::Type::Command,
		"DEF", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Rem_Code
		Token::Type::Command,
		"REM", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	{	// If_Code
		Token::Type::Command,
		"IF", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Then_Code
		Token::Type::Command,
		"THEN", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Else_Code
		Token::Type::Command,
		"ELSE", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// End_Code
		Token::Type::Command,
		"END", "", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// EndIf_Code
		Token::Type::Command,
		"END", "IF", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// For_Code
		Token::Type::Command,
		"FOR", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// To_Code
		Token::Type::Command,
		"TO", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Step_Code
		Token::Type::Command,
		"STEP", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Next_Code
		Token::Type::Command,
		"NEXT", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Do_Code
		Token::Type::Command,
		"DO", "", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoWhile_Code
		Token::Type::Command,
		"DO", "WHILE", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// DoUntil_Code
		Token::Type::Command,
		"DO", "UNTIL", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// While_Code
		Token::Type::Command,
		"WHILE", "", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Until_Code
		Token::Type::Command,
		"UNTIL", "", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// Loop_Code
		Token::Type::Command,
		"LOOP", "", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopWhile_Code
		Token::Type::Command,
		"LOOP", "WHILE", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// LoopUntil_Code
		Token::Type::Command,
		"LOOP", "UNTIL", "",
		Two_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{	// Rnd_Code
		Token::Type::IntFuncN,
		"RND", "", "",
		TableFlag{}, 2, DataType::Double, new ExprInfo(),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//--------------------
	//   Word Operators
	//--------------------
	{	// Mod_Code
		Token::Type::Operator,
		"MOD", "", "",
		TableFlag{}, 42, DataType::Double,
		new ExprInfo(Operands(DblDbl), AssocCode2(Mod, 1)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// And_Code
		Token::Type::Operator,
		"AND", "", "",
		TableFlag{}, 18, DataType::Integer, &IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Or_Code
		Token::Type::Operator,
		"OR", "", "",
		TableFlag{}, 14, DataType::Integer, &IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Not_Code
		Token::Type::Operator,
		"NOT", "", "",
		TableFlag{}, 20, DataType::Integer,
		new ExprInfo(Operands(Int)),
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Eqv_Code
		Token::Type::Operator,
		"EQV", "", "",
		TableFlag{}, 12, DataType::Integer, &IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Imp_Code
		Token::Type::Operator,
		"IMP", "", "",
		TableFlag{}, 10, DataType::Integer, &IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Xor_Code
		Token::Type::Operator,
		"XOR", "", "",
		TableFlag{}, 16, DataType::Integer, &IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	//*********************
	//   END PLAIN WORDS
	//*********************
	{	// EndPlainWord_Code
		Token::Type{},
		"", "NULL", "",
		TableFlag{}, 0, DataType{}, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	//*****************************
	//   BEGIN PARENTHESES WORDS
	//*****************************
	{	// BegParenWord_Code
		Token::Type{},
		"", "NULL", "",
		TableFlag{}, 0, DataType{}, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	//--------------------------------------
	//   INTERNAL FUNCTIONS (Parentheses)
	//--------------------------------------
	{	// Abs_Code
		Token::Type::IntFuncP,
		"ABS(", "", "",
		TableFlag{}, 2, DataType::Double,
		new ExprInfo(Operands(Dbl), AssocCode(Abs)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Fix_Code
		Token::Type::IntFuncP,
		"FIX(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Frac_Code
		Token::Type::IntFuncP,
		"FRAC(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Int_Code
		Token::Type::IntFuncP,
		"INT(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate

	},
	{	// RndArg_Code
		Token::Type::IntFuncP,
		"RND(", "", "",
		TableFlag{}, 2, DataType::Double,
		new ExprInfo(Operands(Dbl), AssocCode(RndArgs)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sgn_Code
		Token::Type::IntFuncP,
		"SGN(", "", "",
		TableFlag{}, 2, DataType::Double,
		new ExprInfo(Operands(Dbl), AssocCode(Sgn)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cint_Code
		Token::Type::IntFuncP,
		"CINT(", "", "",
		TableFlag{}, 2, DataType::Integer, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cdbl_Code
		Token::Type::IntFuncP,
		"CDBL(", "", "",
		TableFlag{}, 2, DataType::Double, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sqr_Code
		Token::Type::IntFuncP,
		"SQR(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Atn_Code
		Token::Type::IntFuncP,
		"ATN(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Cos_Code
		Token::Type::IntFuncP,
		"COS(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Sin_Code
		Token::Type::IntFuncP,
		"SIN(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tan_Code
		Token::Type::IntFuncP,
		"TAN(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Exp_Code
		Token::Type::IntFuncP,
		"EXP(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Log_Code
		Token::Type::IntFuncP,
		"LOG(", "", "",
		TableFlag{}, 2, DataType::Double, &Dbl_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Tab_Code
		Token::Type::IntFuncP,
		"TAB(", "", "",
		Print_Flag, 2, DataType::None, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Spc_Code
		Token::Type::IntFuncP,
		"SPC(", "", "",
		Print_Flag, 2, DataType::None, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, printFunctionRecreate
	},
	{	// Asc_Code
		Token::Type::IntFuncP,
		"ASC(", "", "",
		Multiple_Flag, 2, DataType::Integer,
		new ExprInfo(Operands(Str)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Asc2_Code
		Token::Type::IntFuncP,
		"ASC(", "2", "",
		TableFlag{}, 2, DataType::Integer,
		new ExprInfo(Operands(StrInt)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Chr_Code
		Token::Type::IntFuncP,
		"CHR$(", "", "",
		TableFlag{}, 2, DataType::String, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr2_Code
		Token::Type::IntFuncP,
		"INSTR(", "2", "",
		Multiple_Flag, 2, DataType::Integer, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Instr3_Code
		Token::Type::IntFuncP,
		"INSTR(", "3", "",
		TableFlag{}, 2, DataType::Integer,
		new ExprInfo(Operands(StrStrInt)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Left_Code
		Token::Type::IntFuncP,
		"LEFT$(", "", "",
		SubStr_Flag, 2, DataType::String,
		new ExprInfo(Operands(StrInt), AssocCode2(Left, -1)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Len_Code
		Token::Type::IntFuncP,
		"LEN(", "", "",
		TableFlag{}, 2, DataType::Integer,
		new ExprInfo(Operands(Str)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid2_Code
		Token::Type::IntFuncP,
		"MID$(", "2", "",
		Multiple_Flag | SubStr_Flag, 2, DataType::String,
		new ExprInfo(Operands(StrInt), AssocCode2(Mid2, -1)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Mid3_Code
		Token::Type::IntFuncP,
		"MID$(", "3", "",
		SubStr_Flag, 2, DataType::String,
		new ExprInfo(Operands(StrIntInt), AssocCode2(Mid3, -1)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Repeat_Code
		Token::Type::IntFuncP,
		"REPEAT$(", "", "",
		TableFlag{}, 2, DataType::String,
		new ExprInfo(Operands(StrInt)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Right_Code
		Token::Type::IntFuncP,
		"RIGHT$(", "", "",
		SubStr_Flag, 2, DataType::String,
		new ExprInfo(Operands(StrInt), AssocCode2(Right, -1)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Space_Code
		Token::Type::IntFuncP,
		"SPACE$(", "", "",
		TableFlag{}, 2, DataType::String, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Str_Code
		Token::Type::IntFuncP,
		"STR$(", "", "",
		TableFlag{}, 2, DataType::String,
		new ExprInfo(Operands(Dbl), AssocCode(Str)),
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// Val_Code
		Token::Type::IntFuncP,
		"VAL(", "", "",
		TableFlag{}, 2, DataType::Double, &Str_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	//***************************
	//   END PARENTHESES WORDS
	//***************************
	{	// EndParenWord_Code
		Token::Type{},
		"", "NULL", "",
		TableFlag{}, 0, DataType{}, NULL,
		NULL, NULL, NULL, NULL, NULL

	},

	//*******************
	//   BEGIN SYMBOLS
	//*******************
	{	// BegSymbol_Code
		Token::Type{},
		"", "NULL", "",
		TableFlag{}, 0, DataType{}, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	//----------------------
	//   Symbol Operators
	//----------------------
	{	// Add_Code
		Token::Type::Operator,
		"+", "", "",
		TableFlag{}, 40, DataType::Double,
		new ExprInfo(Operands(DblDbl), AssocCode2(Add, 2)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Neg_Code
		Token::Type::Operator,
		"-", "U", "",
		UseConstAsIs_Flag, 48, DataType::Double,
		new ExprInfo(Operands(Dbl), AssocCode2(Neg, 1)),
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// Mul_Code
		Token::Type::Operator,
		"*", "", "",
		TableFlag{}, 46, DataType::Double,
		new ExprInfo(Operands(DblDbl), AssocCode2(Mul, 1)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Div_Code
		Token::Type::Operator,
		"/", "", "",
		TableFlag{}, 46, DataType::Double,
		new ExprInfo(Operands(DblDbl), AssocCode2(Div, 1)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// IntDiv_Code
		Token::Type::Operator,
		"\\", "", "",
		TableFlag{}, 44, DataType::Integer, &DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Power_Code
		Token::Type::Operator,
		"^", "", "",
		UseConstAsIs_Flag, 50, DataType::Double,
		new ExprInfo(Operands(DblDbl), AssocCode2(Power, 1)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Eq_Code
		Token::Type::Operator,
		"=", "", "",
		TableFlag{}, 30, DataType::Integer,
		new ExprInfo(Operands(DblDbl), AssocCode2(Eq, 2)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Gt_Code
		Token::Type::Operator,
		">", "", "",
		Two_Flag, 32, DataType::Integer,
		new ExprInfo(Operands(DblDbl), AssocCode2(Gt, 2)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEq_Code
		Token::Type::Operator,
		">=", "", "",
		TableFlag{}, 32, DataType::Integer,
		new ExprInfo(Operands(DblDbl), AssocCode2(GtEq, 2)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Lt_Code
		Token::Type::Operator,
		"<", "", "",
		Two_Flag, 32, DataType::Integer,
		new ExprInfo(Operands(DblDbl), AssocCode2(Lt, 2)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEq_Code
		Token::Type::Operator,
		"<=", "", "",
		TableFlag{}, 32, DataType::Integer,
		new ExprInfo(Operands(DblDbl), AssocCode2(LtEq, 2)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEq_Code
		Token::Type::Operator,
		"<>", "", "",
		Two_Flag, 30, DataType::Integer,
		new ExprInfo(Operands(DblDbl), AssocCode2(NotEq, 2)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// OpenParen_Code
		Token::Type::Operator,
		"(", "", "",
		TableFlag{}, 2, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// CloseParen_Code
		Token::Type::Operator,
		")", "", "",
		TableFlag{}, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, parenRecreate
	},
	{	// Comma_Code
		Token::Type::Operator,
		",", "", "",
		TableFlag{}, 6, DataType::None, NULL,
		NULL, NULL, NULL, NULL, printCommaRecreate
	},
	{	// SemiColon_Code
		Token::Type::Operator,
		";", "", "",
		TableFlag{}, 6, DataType::None, NULL,
		NULL, NULL, NULL, NULL, printSemicolonRecreate
	},
	{	// Colon_Code
		Token::Type::Operator,
		":", "", "",
		EndStmt_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// RemOp_Code
		Token::Type::Operator,
		"'", "", "",
		EndStmt_Flag, 2, DataType::None, NULL,
		NULL, remEncode, remOperandText, remRemove, remRecreate
	},
	//*****************
	//   END SYMBOLS
	//*****************
	{	// EndSymbol_Code
		Token::Type{},
		"", "NULL", "",
		TableFlag{}, 0, DataType{}, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	//***************************
	//   MISCELLANEOUS ENTRIES
	//***************************
	{	// Assign_Code
		Token::Type::Operator,
		"=", "Assign", "LET",
		Reference_Flag, 4, DataType::Double,
		new ExprInfo(Operands(DblDbl), AssocCode2(Assign, 2)),
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignInt_Code
		Token::Type::Operator,
		"=", "Assign%", "LET",
		Reference_Flag, 4, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(AssignInt)),
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignStr_Code
		Token::Type::Operator,
		"=", "Assign$", "LET",
		Reference_Flag, 4, DataType::String,
		new ExprInfo(Operands(StrStr), AssocCode2(AssignStr, 1)),
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignLeft_Code
		Token::Type::Operator,
		"LEFT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag, 4, DataType::String,
		new ExprInfo(Operands(StrStr), AssocCode(AssignLeft)),
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid2_Code
		Token::Type::Operator,
		"MID$(", "Assign2", "LET",
		Reference_Flag | SubStr_Flag, 4, DataType::String,
		new ExprInfo(Operands(StrStr), AssocCode(AssignMid2)),
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignMid3_Code
		Token::Type::Operator,
		"MID$(", "Assign3", "LET",
		Reference_Flag | SubStr_Flag | Multiple_Flag, 4, DataType::String,
		new ExprInfo(Operands(StrStr), AssocCode(AssignMid3)),
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignRight_Code
		Token::Type::Operator,
		"RIGHT$(", "Assign", "LET",
		Reference_Flag | SubStr_Flag, 4, DataType::String,
		new ExprInfo(Operands(StrStr), AssocCode(AssignRight)),
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignList_Code
		Token::Type::Operator,
		"=", "AssignList", "LET",
		Reference_Flag, 4, DataType::Double, &DblDbl_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListInt_Code
		Token::Type::Operator,
		"=", "AssignList%", "LET",
		Reference_Flag, 4, DataType::Integer, &IntInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignListStr_Code
		Token::Type::Operator,
		"=", "AssignList$", "LET",
		Reference_Flag, 4, DataType::String, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, assignRecreate
	},
	{	// AssignKeepStr_Code
		Token::Type::Operator,
		"", "AssignKeep$", "LET",
		Reference_Flag | Keep_Flag, 4, DataType::String, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepLeft_Code
		Token::Type::Operator,
		"LEFT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, DataType::String,
		&StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid2_Code
		Token::Type::Operator,
		"MID$(", "AssignKeep2", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, DataType::String,
		&StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepMid3_Code
		Token::Type::Operator,
		"MID$(", "AssignKeep3", "LET",
		Reference_Flag | SubStr_Flag | Multiple_Flag | Keep_Flag, 4,
		DataType::String, &StrStrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// AssignKeepRight_Code
		Token::Type::Operator,
		"RIGHT$(", "AssignKeep", "LET",
		Reference_Flag | SubStr_Flag | Keep_Flag, 4, DataType::String,
		&StrInt_ExprInfo,
		NULL, NULL, NULL, NULL, assignStrRecreate
	},
	{	// EOL_Code
		Token::Type::Operator,
		"", "EOL", "",
		EndStmt_Flag, 4, DataType::None, NULL,
		NULL, NULL, NULL, NULL, NULL

	},
	{	// AddI1_Code
		Token::Type::Operator,
		"+", "%1", "",
		TableFlag{}, 40, DataType::Double, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddI2_Code
		Token::Type::Operator,
		"+", "%2", "",
		TableFlag{}, 40, DataType::Double, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AddInt_Code
		Token::Type::Operator,
		"+", "%", "",
		TableFlag{}, 40, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(AddInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// CatStr_Code
		Token::Type::Operator,
		"+", "$", "",
		TableFlag{}, 40, DataType::String, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI1_Code
		Token::Type::Operator,
		"-", "%1", "",
		TableFlag{}, 40, DataType::Double, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// Sub_Code
		Token::Type::Operator,
		"-", "", "",
		TableFlag{}, 40, DataType::Double,
		new ExprInfo(Operands(DblDbl), AssocCode2(Sub, 1)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubI2_Code
		Token::Type::Operator,
		"-", "%2", "",
		TableFlag{}, 40, DataType::Double, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// SubInt_Code
		Token::Type::Operator,
		"-", "%", "",
		TableFlag{}, 40, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(SubInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NegInt_Code
		Token::Type::Operator,
		"-", "U%", "",
		TableFlag{}, 48, DataType::Integer,
		new ExprInfo(Operands(Int)),
		NULL, NULL, NULL, NULL, unaryOperatorRecreate
	},
	{	// MulI1_Code
		Token::Type::Operator,
		"*", "%1", "",
		TableFlag{}, 46, DataType::Double, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulI2_Code
		Token::Type::Operator,
		"*", "%2", "",
		TableFlag{}, 46, DataType::Double, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// MulInt_Code
		Token::Type::Operator,
		"*", "%", "",
		TableFlag{}, 46, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(MulInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI1_Code
		Token::Type::Operator,
		"/", "%1", "",
		TableFlag{}, 46, DataType::Double, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivI2_Code
		Token::Type::Operator,
		"/", "%2", "",
		TableFlag{}, 46, DataType::Double, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// DivInt_Code
		Token::Type::Operator,
		"/", "%", "",
		TableFlag{}, 46, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(DivInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI1_Code
		Token::Type::Operator,
		"MOD", "%1", "",
		TableFlag{}, 42, DataType::Double, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModI2_Code
		Token::Type::Operator,
		"MOD", "%2", "",
		TableFlag{}, 42, DataType::Double, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// ModInt_Code
		Token::Type::Operator,
		"MOD", "%", "",
		TableFlag{}, 42, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(ModInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerI1_Code
		Token::Type::Operator,
		"^", "%1", "",
		TableFlag{}, 50, DataType::Double, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerMul_Code
		Token::Type::Operator,
		"^", "*", "",
		TableFlag{}, 50, DataType::Double, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// PowerInt_Code
		Token::Type::Operator,
		"^", "%", "",
		TableFlag{}, 50, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(PowerInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI1_Code
		Token::Type::Operator,
		"=", "%1", "",
		TableFlag{}, 30, DataType::Integer, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqI2_Code
		Token::Type::Operator,
		"=", "%2", "",
		TableFlag{}, 30, DataType::Integer, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqInt_Code
		Token::Type::Operator,
		"=", "%", "",
		TableFlag{}, 30, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(EqInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// EqStr_Code
		Token::Type::Operator,
		"=", "$", "",
		TableFlag{}, 30, DataType::Integer, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI1_Code
		Token::Type::Operator,
		">", "%1", "",
		TableFlag{}, 32, DataType::Integer, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtI2_Code
		Token::Type::Operator,
		">", "%2", "",
		TableFlag{}, 32, DataType::Integer, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtInt_Code
		Token::Type::Operator,
		">", "%", "",
		TableFlag{}, 32, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(GtInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtStr_Code
		Token::Type::Operator,
		">", "$", "",
		TableFlag{}, 32, DataType::Integer, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI1_Code
		Token::Type::Operator,
		">=", "%1", "",
		TableFlag{}, 32, DataType::Integer, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqI2_Code
		Token::Type::Operator,
		">=", "%2", "",
		TableFlag{}, 32, DataType::Integer, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqInt_Code
		Token::Type::Operator,
		">=", "%", "",
		TableFlag{}, 32, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(GtEqInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// GtEqStr_Code
		Token::Type::Operator,
		">=", "$", "",
		TableFlag{}, 32, DataType::Integer, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI1_Code
		Token::Type::Operator,
		"<", "%1", "",
		TableFlag{}, 32, DataType::Integer, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtI2_Code
		Token::Type::Operator,
		"<", "%2", "",
		TableFlag{}, 32, DataType::Integer, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtInt_Code
		Token::Type::Operator,
		"<", "%", "",
		TableFlag{}, 32, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(LtInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtStr_Code
		Token::Type::Operator,
		"<", "$", "",
		TableFlag{}, 32, DataType::Integer, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI1_Code
		Token::Type::Operator,
		"<=", "%1", "",
		TableFlag{}, 32, DataType::Integer, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqI2_Code
		Token::Type::Operator,
		"<=", "%2", "",
		TableFlag{}, 32, DataType::Integer, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqInt_Code
		Token::Type::Operator,
		"<=", "%", "",
		TableFlag{}, 32, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(LtEqInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// LtEqStr_Code
		Token::Type::Operator,
		"<=", "$", "",
		TableFlag{}, 32, DataType::Integer, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI1_Code
		Token::Type::Operator,
		"<>", "%1", "",
		TableFlag{}, 30, DataType::Integer, &IntDbl_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqI2_Code
		Token::Type::Operator,
		"<>", "%2", "",
		TableFlag{}, 30, DataType::Integer, &DblInt_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqInt_Code
		Token::Type::Operator,
		"<>", "%", "",
		TableFlag{}, 30, DataType::Integer,
		new ExprInfo(Operands(IntInt), AssocCode(NotEqInt)),
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// NotEqStr_Code
		Token::Type::Operator,
		"<>", "$", "",
		TableFlag{}, 30, DataType::Integer, &StrStr_ExprInfo,
		NULL, NULL, NULL, NULL, binaryOperatorRecreate
	},
	{	// AbsInt_Code
		Token::Type::IntFuncP,
		"ABS(", "%", "",
		TableFlag{}, 2, DataType::Integer, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// RndArgInt_Code
		Token::Type::IntFuncP,
		"RND(", "%", "",
		TableFlag{}, 2, DataType::Integer, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// SgnInt_Code
		Token::Type::IntFuncP,
		"SGN(", "%", "",
		TableFlag{}, 2, DataType::Integer, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// CvtInt_Code
		Token::Type::IntFuncN,
		"", "CvtInt", "",
		Hidden_Flag, 2, DataType::None, NULL,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// CvtDbl_Code
		Token::Type::IntFuncN,
		"", "CvtDbl", "",
		Hidden_Flag, 2, DataType::None, NULL,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// StrInt_Code
		Token::Type::IntFuncP,
		"STR$(", "%", "",
		TableFlag{}, 2, DataType::String, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, internalFunctionRecreate
	},
	{	// PrintDbl_Code
		Token::Type::IntFuncN,
		"", "PrintDbl", "",
		Print_Flag | UseConstAsIs_Flag, 2, DataType::None,
		new ExprInfo(Operands(Dbl), AssocCode(Print)),
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintInt_Code
		Token::Type::IntFuncN,
		"", "PrintInt", "",
		Print_Flag, 2, DataType::None, &Int_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// PrintStr_Code
		Token::Type::IntFuncN,
		"", "PrintStr", "",
		Print_Flag, 2, DataType::None, &Str_ExprInfo,
		NULL, NULL, NULL, NULL, printItemRecreate
	},
	{	// InputBegin_Code
		Token::Type::IntFuncN,
		"", "InputBegin", "",
		TableFlag{}, 2, DataType::None, NULL,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputBeginStr_Code
		Token::Type::IntFuncN,
		"", "InputBeginStr", "Question",
		TableFlag{}, 2, DataType::None, new ExprInfo(Operands(Str)),
		NULL, NULL, NULL, NULL, inputPromptBeginRecreate
	},
	{	// InputAssign_Code
		Token::Type::IntFuncN,
		"", "InputAssign", "",
		Reference_Flag, 2, DataType::None,
		new ExprInfo(Operands(Dbl), AssocCode2(InputAssign, 2)),
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignInt_Code
		Token::Type::IntFuncN,
		"", "InputAssignInt", "",
		Reference_Flag, 2, DataType::None,
		new ExprInfo(Operands(Int), AssocCode(InputAssignInt)),
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputAssignStr_Code
		Token::Type::IntFuncN,
		"", "InputAssignStr", "",
		Reference_Flag, 2, DataType::None,
		new ExprInfo(Operands(Str), AssocCode(InputAssignStr)),
		NULL, NULL, NULL, NULL, inputAssignRecreate
	},
	{	// InputParse_Code
		Token::Type::IntFuncN,
		"", "InputParse", "",
		TableFlag{}, 2, DataType::None, NULL,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseInt_Code
		Token::Type::IntFuncN,
		"", "InputParseInt", "",
		TableFlag{}, 2, DataType::None, NULL,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// InputParseStr_Code
		Token::Type::IntFuncN,
		"", "InputParseStr", "",
		TableFlag{}, 2, DataType::None, NULL,
		NULL, NULL, NULL, NULL, blankRecreate
	},
	{	// Const_Code
		Token::Type::Constant,
		"", "Const", "",
		TableFlag{}, 2, DataType::Double,
		new ExprInfo(Operands(Dbl), AssocCode(Const)),
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstInt_Code
		Token::Type::Constant,
		"", "ConstInt", "",
		TableFlag{}, 2, DataType::Integer, &Int_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove,
		operandRecreate
	},
	{	// ConstStr_Code
		Token::Type::Constant,
		"", "ConstStr", "",
		TableFlag{}, 2, DataType::String, &Str_ExprInfo,
		NULL, constStrEncode, constStrOperandText, constStrRemove,
		constStrRecreate
	},
	{	// Var_Code
		Token::Type::NoParen,
		"", "Var", "",
		TableFlag{}, 2, DataType::Double,
		new ExprInfo(Operands(Dbl), AssocCode(Var)),
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarInt_Code
		Token::Type::NoParen,
		"", "VarInt", "",
		TableFlag{}, 2, DataType::Integer, &Int_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarStr_Code
		Token::Type::NoParen,
		"", "VarStr", "",
		TableFlag{}, 2, DataType::String, &Str_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// VarRef_Code
		Token::Type::NoParen,
		"", "VarRef", "",
		Reference_Flag, 2, DataType::Double,
		new ExprInfo(Operands(Dbl), AssocCode(VarRef)),
		NULL, varDblEncode, varDblOperandText, varDblRemove, operandRecreate
	},
	{	// VarRefInt_Code
		Token::Type::NoParen,
		"", "VarRefInt", "",
		Reference_Flag, 2, DataType::Integer, &Int_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove, operandRecreate
	},
	{	// VarRefStr_Code
		Token::Type::NoParen,
		"", "VarRefStr", "",
		Reference_Flag, 2, DataType::String, &Str_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove, operandRecreate
	},
	{	// Array_Code
		// TODO preliminary until full array support is implemented
		Token::Type::Paren,
		"", "Array", "",
		TableFlag{}, 2, DataType::Double, NULL,
		NULL, NULL, NULL, NULL, arrayRecreate
	},
	{	// DefFuncN_Code
		// TODO preliminary until full define function support is implemented
		Token::Type::DefFuncN,
		"", "DefFuncN", "",
		TableFlag{}, 2, DataType::Double, NULL,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// DefFuncP_Code
		// TODO preliminary until full define function support is implemented
		Token::Type::DefFuncP,
		"", "DefFuncP", "",
		TableFlag{}, 2, DataType::Double, NULL,
		NULL, NULL, NULL, NULL, defineFunctionRecreate
	},
	{	// Function_Code
		// TODO preliminary until full user function support is implemented
		Token::Type::Paren,
		"", "Function", "",
		TableFlag{}, 2, DataType::Double, NULL,
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
	{
		s_instance = new Table(tableEntries, sizeof(tableEntries)
			/ sizeof(TableEntry));  // aborts application if table errors
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
	QStringList errorList;
	int i;
	int type;

	// scan table and record indexes
	// find maximum number of operands and associated codes
	for (i = 0; i < entryCount; i++)
	{
		// check if found new maximums
		ExprInfo *exprInfo {m_entry[i].exprInfo};
		if (exprInfo)
		{
			// check if assoc2_index is valid
			if (exprInfo->m_secondAssociatedIndex > 0
				&& exprInfo->m_secondAssociatedIndex
				> exprInfo->m_associatedCodeCount)
			{
				errorList.append(QString("Entry:%1 Assoc2Index=%2 too large, "
					"maximum is %3").arg(i)
					.arg(exprInfo->m_secondAssociatedIndex)
					.arg(exprInfo->m_associatedCodeCount));
			}

			// validate multiple non-assignment entries
			if (hasFlag((Code)i, Multiple_Flag)
				&& !hasFlag((Code)i, Reference_Flag))
			{
				ExprInfo *exprInfo2 {m_entry[i + 1].exprInfo};
				if (m_entry[i].name != m_entry[i + 1].name)
				{
					errorList.append(QString("Multiple entry '%1' name "
						"mis-match '%2'").arg(m_entry[i].name.c_str())
						.arg(m_entry[i + 1].name.c_str()));
				}
				else if (!exprInfo2)
				{
					errorList.append(QString("Multiple entry '%1' next entry "
						"no expression info").arg(m_entry[i + 1].name.c_str()));
				}
				else if (exprInfo2->m_operandCount
					!= exprInfo->m_operandCount + 1)
				{
					errorList.append(QString("Multiple entry '%1' incorrect "
						"number of operands (%2, %3)")
						.arg(m_entry[i].name.c_str())
						.arg(exprInfo->m_operandCount)
						.arg(exprInfo2->m_operandCount));
				}
			}

			enum {
				Dbl_BitMask = 0x01,
				Int_BitMask = 0x02,
				Str_BitMask = 0x04,
				Num_BitMask = Dbl_BitMask | Int_BitMask,
				Any_BitMask = Num_BitMask | Str_BitMask
			};
			std::unordered_map<DataType, int, EnumClassHash> bitMaskDataType {
				{DataType::Double,  Dbl_BitMask},
				{DataType::Integer, Int_BitMask},
				{DataType::String,  Str_BitMask}
			};

			// set expected data type (start with data type of last operand)
			if (exprInfo->m_operandCount > 0)
			{
				// use last operand for operators, first operand for functions
				exprInfo->m_expectedDataType
					= exprInfo->m_operandDataType[m_entry[i].type
					== Token::Type::Operator
					? exprInfo->m_operandCount - 1 : 0];
				// check each secondary associated code
				if (exprInfo->m_associatedCodeCount > 0)
				{
					int bitMask {bitMaskDataType[exprInfo->m_expectedDataType]};
					// start at 0 for unary ops
					bool unaryOp {m_entry[i].type == Token::Type::Operator
						&& exprInfo->m_operandCount == 1};
					int index {unaryOp ? 0 : exprInfo->m_secondAssociatedIndex};
					if (index >= 0)
					{
						int limit {unaryOp ? exprInfo->m_secondAssociatedIndex
							: exprInfo->m_associatedCodeCount};
						for (; index < limit; index++)
						{
							Code assocCode {exprInfo->m_associatedCode[index]};
							ExprInfo *exprInfo2 {m_entry[assocCode].exprInfo};
							if (exprInfo2)
							{
								bitMask |= bitMaskDataType[exprInfo2
									->m_operandDataType[exprInfo2
									->m_operandCount - 1]];
							}
						}
						if (bitMask == Num_BitMask)
						{
							exprInfo->m_expectedDataType = DataType::Number;
						}
						else if (bitMask == Any_BitMask)
						{
							exprInfo->m_expectedDataType = DataType::Any;
						}
					}
				}
			}
		}
	}

	// setup indexes for bracketing codes
	// (will be set to -1 if missing - missing errors were recorded above)
	m_range[PlainWord_SearchType].beg = BegPlainWord_Code;
	m_range[PlainWord_SearchType].end = EndPlainWord_Code;
	m_range[ParenWord_SearchType].beg = BegParenWord_Code;
	m_range[ParenWord_SearchType].end = EndParenWord_Code;
	m_range[Symbol_SearchType].beg = BegSymbol_Code;
	m_range[Symbol_SearchType].end = EndSymbol_Code;

	// check for missing bracketing codes and if properly positioned in table
	// (missing codes recorded above, however,
	// need to make sure all types were set, i.e. no missing assignments above)
	for (type = 0; type < sizeof_SearchType; type++)
	{
		if (m_range[type].beg > m_range[type].end)
		{
			// record bracket range error
			errorList.append(QString("Search type %1 indexes (%2, %3) not "
				"correct").arg(type).arg(m_range[type].beg)
				.arg(m_range[type].end));
		}
		else
		{
			// check to make sure no bracketing codes overlap
			for (int type2 {}; type2 < sizeof_SearchType; type2++)
			{
				if (type != type2
					&& ((m_range[type].beg > m_range[type2].beg
					&& m_range[type].beg < m_range[type2].end)
					|| (m_range[type].end > m_range[type2].beg
					&& m_range[type].end < m_range[type2].end)))
				{
					// record bracket overlap error
					errorList.append(QString("Search type %1 indexes (%2, %3) "
						"overlap with search type %4").arg(type)
						.arg(m_range[type].beg).arg(m_range[type].end)
						.arg(type2));
				}
			}
		}
	}

	// if errors then output messages and abort
	if (!errorList.isEmpty())
	{
		int n {};
		foreach (QString error, errorList)
		{
			qCritical("%s", qPrintable(tr("Error #%1: %2").arg(++n)
				.arg(error)));
		}
		qFatal("%s", qPrintable(tr("Program aborting!")));
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

// returns data type for code
DataType Table::dataType(Code code) const
{
	return m_entry[code].dataType;
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

// returns the number of associate codes for code
int Table::associatedCodeCount(Code code) const
{
	return m_entry[code].exprInfo->m_associatedCodeCount;
}

// returns the associate code for a specific index for code
Code Table::associatedCode(Code code, int index) const
{
	return m_entry[code].exprInfo->m_associatedCode[index];
}

// returns the associated code array for a code
Code *Table::associatedCodeArray(Code code) const
{
	return m_entry[code].exprInfo->m_associatedCode;
}

// returns the start index of the secondary associated codes for code
int Table::secondAssociatedIndex(Code code) const
{
	return m_entry[code].exprInfo->m_secondAssociatedIndex;
}

// returns the secondary associated code for a specific index for code
Code Table::secondAssociatedCode(Code code, int index) const
{
	return m_entry[code].exprInfo->m_associatedCode[secondAssociatedIndex(code)
		+ index];
}

// returns the expected data type for last operand for operator code
DataType Table::expectedDataType(Code code) const
{
	return m_entry[code].exprInfo->m_expectedDataType;
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
	if (token->isType(Token::Type::Operator))
	{
		ExprInfo *ei {m_entry[token->code()].exprInfo};
		if (ei && ei->m_operandCount == 1)
		{
			return token->code();
		}
	}
	return Null_Code;
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
	ExprInfo *exprInfo {m_entry[token->code()].exprInfo};
	return !exprInfo ? 0 : exprInfo->m_operandCount;
}

// returns the expected data type for last operand for operator token
DataType Table::expectedDataType(const TokenPtr &token) const
{
	return expectedDataType(token->code());
}

// function to set a token for a code (code, token type and data type)
void Table::setToken(TokenPtr &token, Code code)
{
	token->setCode(code);
	token->setType(type(code));
	token->setDataType(dataType(code));
}

// function to create a new token and initialize it for a code
TokenPtr Table::newToken(Code code)
{
	TokenPtr token {new Token};	// allocates and initializes base members
	setToken(token, code);		// initializes code related members
	return token;
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
	if (dataType != operandDataType(code, operandIndex))
	{
		// if not, see if data type of any associated code matches
		int i {secondAssociatedIndex(code)};
		if (i < 0)  // if second index -1, then no associated codes to search
		{
			return false;
		}
		// determine range of associated codes to search
		int end {associatedCodeCount(code)};
		if (operandIndex == 0 && i != 0)  // first operand?
		{
			// if first operand and there are second operand associated codes
			// then set end to start of second operand associated codes
			// and start to first associated code
			end = i;
			i = 0;
		}
		Code *associatedCodes {associatedCodeArray(code)};
		for (;;)
		{
			if (i >= end)
			{
				return false;  // did not find an associated code for data type
			}
			code = associatedCodes[i++];
			if (dataType == operandDataType(code, operandIndex))
			{
				break;
			}
		}
	}
	// change token's code and data type to associated code
	setToken(token, code);
	return true;
}


//============================
//  TABLE SPECIFIC FUNCTIONS
//============================

// search function to look for a string of a particular type in
// the Table, the search is case insensitive
//
//   - returns the index of the entry that is found
//   - returns -1 if the string was not found in the table

Code Table::search(SearchType type, const std::string &string) const
{
	Code i {m_range[type].beg};
	Code end {m_range[type].end};
	while (++i < end)
	{
		std::string name {m_entry[i].name};
		if (noCaseStringEqual(string, name))
		{
			return i;
		}
	}
	return Invalid_Code;
}


// search function to look for a two word command in the Table,
// the search is case insensitive an only entries containing a second
// word are checked
//
//   - returns the index of the entry that is found
//   - returns -1 if the string was not found in the table

Code Table::search(const std::string &word1, const std::string &word2) const
{
	for (Code i {m_range[PlainWord_SearchType].beg};
		i < m_range[PlainWord_SearchType].end; i++)
	{
		std::string name {m_entry[i].name};
		std::string name2 {m_entry[i].name2};
		if (!name2.empty() && noCaseStringEqual(word1, name)
			&& noCaseStringEqual(word2, name2))
		{
			return i;
		}
	}
	return Invalid_Code;
}


// function to check to see if data types specified match the data
// types of the code at the index specified
//
//   - returns true if there is match, otherwise returns false

bool Table::match(Code code, DataType *datatype) const
{
	for (int n {operandCount(code)}; --n >= 0;)
	{
		if (datatype[n] != operandDataType(code, n))
		{
			return false;
		}
	}
	return true;
}


// end: table.cpp
