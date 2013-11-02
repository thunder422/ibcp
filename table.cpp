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

#include <QChar>
#include <QString>

#include "table.h"
#include "basic/basic.h"


const int MaxOperands = 3;
	// this value contains the maximum number of operands
	// (arguments) for any operator or internal function (there are currently
	// no internal function with more than 3 arguments)

const int MaxAssocCodes = 3;
	// this value contains the maximum number of associated codes,
	// codes in additional to the main code for different possible data types
	// for the code (no code currently has more the 3 total codes)


// array of conversion codes [have data type] [need data type]
static Code cvtCodeHaveNeed[sizeof_DataType][sizeof_DataType] = {
	{	// have Double,    need:
		Null_Code,		// Double
		CvtInt_Code,	// Integer
		Invalid_Code,	// String
		Null_Code,		// None
		Null_Code,		// Number
		Null_Code		// Any
	},
	{	// have Integer,   need:
		CvtDbl_Code,	// Double
		Null_Code,		// Integer
		Invalid_Code,	// String
		Null_Code,		// None
		Null_Code,		// Number
		Null_Code		// Any
	},
	{	// have String,    need:
		Invalid_Code,	// Double
		Invalid_Code,	// Integer
		Null_Code,		// String
		Null_Code,		// None
		Invalid_Code,	// Number
		Null_Code		// Any
	},
	{	// have None,      need:  (print functions have this data type)
		Invalid_Code,	// Double
		Invalid_Code,	// Integer
		Invalid_Code,	// String
		Null_Code,		// None (print function allowed if needed None)
		Invalid_Code,	// Number
		Invalid_Code	// Any
	},
	{	// have Number,    need:  (will not have any of this data type)
		Invalid_Code,	// Double
		Invalid_Code,	// Integer
		Invalid_Code,	// String
		Invalid_Code,	// None
		Invalid_Code,	// Number
		Invalid_Code	// Any
	},
	{	// have Any,       need:  (will not have any of this data type)
		Invalid_Code,	// Double
		Invalid_Code,	// Integer
		Invalid_Code,	// String
		Invalid_Code,	// None
		Invalid_Code,	// Number
		Invalid_Code	// Any
	}
};


// expression information for operators and internal functions
struct ExprInfo
{
	Code m_unaryCode;				// operator unary code (Null_Code if none)
	short m_operandCount;			// number of operands (operators/functions)
	short m_associatedCodeCount;	// number of associated codes
	short m_secondAssociatedIndex;	// index of second operand associated codes
	DataType *m_operandDataType;	// data type of each operand
	Code *m_associatedCode;			// associated codes
	DataType m_expectedDataType;	// expected data type of next operand

	ExprInfo(Code unaryCode = Null_Code, short operandCount = 0,
		DataType *operandDataType = NULL, short associatedCodeCount = 0,
		short secondAssociatedIndex = 0, Code *associatedCode = NULL,
		DataType expectedDataType = None_DataType) :
		m_unaryCode(unaryCode), m_operandCount(operandCount),
		m_operandDataType(operandDataType),
		m_associatedCodeCount(associatedCodeCount),
		m_secondAssociatedIndex(secondAssociatedIndex),
		m_associatedCode(associatedCode),
		m_expectedDataType(expectedDataType)
	{
	}
};


struct TableEntry
{
	TokenType type;					// type of token for entry
	Multiple multiple;				// multiple word command/character operator
	const QString name;				// name for table entry
	const QString name2;			// name of second word of command
	const QString option;			// name of option sub-code
	int flags;						// flags for entry
	int precedence;					// precedence of code
	DataType dataType;				// next expression data type for command
	ExprInfo *exprInfo;				// expression info pointer (NULL for none)
	TranslateFunction translate;	// pointer to translate function
	EncodeFunction encode;			// pointer to encode function
	OperandTextFunction operandText;// pointer to operand text function
	RemoveFunction remove;			// pointer to remove function
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
	Double_DataType
};
static DataType DblDbl_OperandArray[] = {
	Double_DataType, Double_DataType
};
static DataType DblInt_OperandArray[] = {
	Double_DataType, Integer_DataType
};

static DataType Int_OperandArray[] = {
	Integer_DataType
};
static DataType IntDbl_OperandArray[] = {
	Integer_DataType, Double_DataType
};
static DataType IntInt_OperandArray[] = {
	Integer_DataType, Integer_DataType
};

static DataType Str_OperandArray[] = {
	String_DataType
};
static DataType StrInt_OperandArray[] = {
	String_DataType, Integer_DataType
};
static DataType StrIntInt_OperandArray[] = {
	String_DataType, Integer_DataType, Integer_DataType
};
static DataType StrStr_OperandArray[] = {
	String_DataType, String_DataType
};
static DataType StrStrInt_OperandArray[] = {
	String_DataType, String_DataType, Integer_DataType
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
static Code AssignLeft_AssocCode[]		= {AssignKeepLeft_Code};
static Code AssignInt_AssocCode[]		= {AssignListInt_Code};
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
static Code Neg_AssocCode[]				= {NegInt_Code};
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
static ExprInfo Dbl_ExprInfo(Null_Code, Operands(Dbl));
static ExprInfo DblDbl_ExprInfo(Null_Code, Operands(DblDbl));
static ExprInfo DblInt_ExprInfo(Null_Code, Operands(DblInt));

static ExprInfo Int_ExprInfo(Null_Code, Operands(Int));
static ExprInfo IntInt_ExprInfo(Null_Code, Operands(IntInt));
static ExprInfo IntDbl_ExprInfo(Null_Code, Operands(IntDbl));

static ExprInfo Str_ExprInfo(Null_Code, Operands(Str));
static ExprInfo StrInt_ExprInfo(Null_Code, Operands(StrInt));
static ExprInfo StrStr_ExprInfo(Null_Code, Operands(StrStr));
static ExprInfo StrStrInt_ExprInfo(Null_Code, Operands(StrStrInt));

static ExprInfo StrIntInt_ExprInfo(Null_Code, Operands(StrIntInt));

// code enumeration names in comments after opening brace
// (code enumeration generated from these by enums.awk)
static TableEntry tableEntries[] =
{
	// Null_Code entry at beginning so Null_Code == 0
	{	// Null_Code
		Operator_TokenType, OneWord_Multiple,
		NULL, "NULL"
	},
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
		Command_TokenType, OneWord_Multiple,
		"LET", NULL, NULL,
		Null_Flag, 4, None_DataType, NULL,
		letTranslate
	},
	{	// Print_Code
		Command_TokenType, OneWord_Multiple,
		"PRINT", NULL, NULL,
		Null_Flag, 4, None_DataType, NULL,
		printTranslate
	},
	{	// Input_Code
		Command_TokenType, TwoWord_Multiple,
		"INPUT", NULL, "Keep",
		Null_Flag, 4, None_DataType, NULL,
		inputTranslate

	},
	{	// InputPrompt_Code
		Command_TokenType, TwoWord_Multiple,
		"INPUT", "PROMPT", "Keep",
		Null_Flag, 4, String_DataType, NULL,
		inputTranslate

	},
	{	// Dim_Code
		Command_TokenType, OneWord_Multiple,
		"DIM", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Def_Code
		Command_TokenType, OneWord_Multiple,
		"DEF", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Rem_Code
		Command_TokenType, OneWord_Multiple,
		"REM", NULL, NULL,
		Null_Flag, 4, None_DataType, NULL,
		NULL, remEncode, remOperandText, remRemove
	},
	{	// If_Code
		Command_TokenType, OneWord_Multiple,
		"IF", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Then_Code
		Command_TokenType, OneWord_Multiple,
		"THEN", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Else_Code
		Command_TokenType, OneWord_Multiple,
		"ELSE", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// End_Code
		Command_TokenType, TwoWord_Multiple,
		"END", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// EndIf_Code
		Command_TokenType, TwoWord_Multiple,
		"END", "IF", NULL,
		Null_Flag, 4, None_DataType
	},
	{	// For_Code
		Command_TokenType, OneWord_Multiple,
		"FOR", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// To_Code
		Command_TokenType, OneWord_Multiple,
		"TO", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Step_Code
		Command_TokenType, OneWord_Multiple,
		"STEP", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Next_Code
		Command_TokenType, OneWord_Multiple,
		"NEXT", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Do_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// DoWhile_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", "WHILE", NULL,
		Null_Flag, 4, None_DataType
	},
	{	// DoUntil_Code
		Command_TokenType, TwoWord_Multiple,
		"DO", "UNTIL", NULL,
		Null_Flag, 4, None_DataType
	},
	{	// While_Code
		Command_TokenType, TwoWord_Multiple,
		"WHILE", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Until_Code
		Command_TokenType, TwoWord_Multiple,
		"UNTIL", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Loop_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// LoopWhile_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", "WHILE", NULL,
		Null_Flag, 4, None_DataType
	},
	{	// LoopUntil_Code
		Command_TokenType, TwoWord_Multiple,
		"LOOP", "UNTIL", NULL,
		Null_Flag, 4, None_DataType
	},
	//-----------------------------------------
	//   Internal Functions (No Parentheses)
	//-----------------------------------------
	{	// Rnd_Code
		IntFuncN_TokenType, OneWord_Multiple,
		"RND", NULL, NULL,
		Null_Flag, 2, Double_DataType, new ExprInfo()

	},
	//--------------------
	//   Word Operators
	//--------------------
	{	// Mod_Code
		Operator_TokenType, OneWord_Multiple,
		"MOD", NULL, NULL,
		Null_Flag, 42, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Mod, 1))
	},
	{	// And_Code
		Operator_TokenType, OneWord_Multiple,
		"AND", NULL, NULL,
		Null_Flag, 18, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Or_Code
		Operator_TokenType, OneWord_Multiple,
		"OR", NULL, NULL,
		Null_Flag, 14, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Not_Code
		Operator_TokenType, OneWord_Multiple,
		"NOT", NULL, NULL,
		Null_Flag, 20, Integer_DataType,
		new ExprInfo(Not_Code, Operands(Int))
	},
	{	// Eqv_Code
		Operator_TokenType, OneWord_Multiple,
		"EQV", NULL, NULL,
		Null_Flag, 12, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Imp_Code
		Operator_TokenType, OneWord_Multiple,
		"IMP", NULL, NULL,
		Null_Flag, 10, Integer_DataType, &IntInt_ExprInfo
	},
	{	// Xor_Code
		Operator_TokenType, OneWord_Multiple,
		"XOR", NULL, NULL,
		Null_Flag, 16, Integer_DataType, &IntInt_ExprInfo
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
		"ABS(", NULL, NULL,
		Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Abs))
	},
	{	// Fix_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"FIX(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Frac_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"FRAC(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Int_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INT(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo

	},
	{	// RndArg_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RND(", NULL, NULL,
		Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(RndArgs))
	},
	{	// Sgn_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", NULL, NULL,
		Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Sgn))
	},
	{	// Cint_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CINT(", NULL, NULL,
		Null_Flag, 2, Integer_DataType, &Dbl_ExprInfo
	},
	{	// Cdbl_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CDBL(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Int_ExprInfo
	},
	{	// Sqr_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SQR(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Atn_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ATN(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Cos_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"COS(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Sin_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SIN(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Tan_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"TAN(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Exp_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"EXP(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Log_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LOG(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Dbl_ExprInfo
	},
	{	// Tab_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"TAB(", NULL, NULL,
		Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// Spc_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SPC(", NULL, NULL,
		Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// Asc_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", NULL, NULL,
		Multiple_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(Str))
	},
	{	// Asc2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ASC(", "ASC2(", NULL,
		Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrInt))
	},
	{	// Chr_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"CHR$(", NULL, NULL,
		Null_Flag, 2, String_DataType, &Int_ExprInfo
	},
	{	// Instr2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR2(", NULL,
		Multiple_Flag, 2, Integer_DataType,
		&StrStr_ExprInfo
	},
	{	// Instr3_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"INSTR(", "INSTR3(", NULL,
		Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(StrStrInt))
	},
	{	// Left_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEFT$(", NULL, NULL,
		SubStr_Flag, 2, String_DataType,
		new ExprInfo(Null_Code, Operands(StrInt), AssocCode2(Left, -1))
	},
	{	// Len_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"LEN(", NULL, NULL,
		Null_Flag, 2, Integer_DataType,
		new ExprInfo(Null_Code, Operands(Str))
	},
	{	// Mid2_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID2$(", NULL,
		Multiple_Flag | SubStr_Flag, 2, String_DataType,
		new ExprInfo(Null_Code, Operands(StrInt), AssocCode2(Mid2, -1))
	},
	{	// Mid3_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"MID$(", "MID3$(", NULL,
		SubStr_Flag, 2, String_DataType,
		new ExprInfo(Null_Code, Operands(StrIntInt), AssocCode2(Mid3, -1))
	},
	{	// Repeat_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"REPEAT$(", NULL, NULL,
		Null_Flag, 2, String_DataType,
		new ExprInfo(Null_Code, Operands(StrInt))
	},
	{	// Right_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RIGHT$(", NULL, NULL,
		SubStr_Flag, 2, String_DataType,
		new ExprInfo(Null_Code, Operands(StrInt), AssocCode2(Right, -1))
	},
	{	// Space_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SPACE$(", NULL, NULL,
		Null_Flag, 2, String_DataType, &Int_ExprInfo
	},
	{	// Str_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", NULL, NULL,
		Null_Flag, 2, String_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Str))
	},
	{	// Val_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"VAL(", NULL, NULL,
		Null_Flag, 2, Double_DataType, &Str_ExprInfo
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
		"+", NULL, NULL,
		Null_Flag, 40, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Add, 2))
	},
	{	// Sub_Code
		Operator_TokenType, OneChar_Multiple,
		"-", NULL, NULL,
		Null_Flag, 40, Double_DataType,
		new ExprInfo(Neg_Code, Operands(DblDbl), AssocCode2(Sub, 1))
	},
	{	// Mul_Code
		Operator_TokenType, OneChar_Multiple,
		"*", NULL, NULL,
		Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Mul, 1))
	},
	{	// Div_Code
		Operator_TokenType, OneChar_Multiple,
		"/", NULL, NULL,
		Null_Flag, 46, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Div, 1))
	},
	{	// IntDiv_Code
		Operator_TokenType, OneChar_Multiple,
		"\\", NULL, NULL,
		Null_Flag, 44, Integer_DataType, &DblDbl_ExprInfo
	},
	{	// Power_Code
		Operator_TokenType, OneChar_Multiple,
		"^", NULL, NULL,
		UseConstAsIs_Flag, 50, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Power, 1))
	},
	{	// Eq_Code
		Operator_TokenType, OneChar_Multiple,
		"=", NULL, NULL,
		Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Eq, 2))
	},
	{	// Gt_Code
		Operator_TokenType, TwoChar_Multiple,
		">", NULL, NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Gt, 2))
	},
	{	// GtEq_Code
		Operator_TokenType, TwoChar_Multiple,
		">=", NULL, NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(GtEq, 2))
	},
	{	// Lt_Code
		Operator_TokenType, TwoChar_Multiple,
		"<", NULL, NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Lt, 2))
	},
	{	// LtEq_Code
		Operator_TokenType, TwoChar_Multiple,
		"<=", NULL, NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(LtEq, 2))
	},
	{	// NotEq_Code
		Operator_TokenType, TwoChar_Multiple,
		"<>", NULL, NULL,
		Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(NotEq, 2))
	},
	{	// OpenParen_Code
		Operator_TokenType, OneChar_Multiple,
		"(", NULL, NULL,
		Null_Flag, 2, None_DataType
	},
	{	// CloseParen_Code
		Operator_TokenType, OneChar_Multiple,
		")", NULL, NULL,
		Null_Flag, 4, None_DataType
	},
	{	// Comma_Code
		Operator_TokenType, OneChar_Multiple,
		",", NULL, NULL,
		Null_Flag, 6, None_DataType
	},
	{	// SemiColon_Code
		Operator_TokenType, OneChar_Multiple,
		";", NULL, NULL,
		Null_Flag, 6, None_DataType
	},
	{	// Colon_Code
		Operator_TokenType, OneChar_Multiple,
		":", NULL, NULL,
		EndStmt_Flag, 4, None_DataType
	},
	{	// RemOp_Code
		Operator_TokenType, OneChar_Multiple,
		"'", NULL, NULL,
		EndStmt_Flag, 2, None_DataType, NULL,
		NULL, remEncode, remOperandText, remRemove
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
	{	// Neg_Code
		Operator_TokenType, OneWord_Multiple,
		// FIXME temporarily replaced "-" with "Neq" for testing
		"-", "Neg", NULL,
		UseConstAsIs_Flag, 48, Double_DataType,
		new ExprInfo(Neg_Code, Operands(Dbl), AssocCode(Neg))
	},
	{	// Assign_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign", "LET",
		Reference_Flag, 4, Double_DataType,
		new ExprInfo(Null_Code, Operands(DblDbl), AssocCode2(Assign, 2))
	},
	{	// AssignInt_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign%", "LET",
		Reference_Flag, 4, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(AssignInt))
	},
	{	// AssignStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "Assign$", "LET",
		Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode2(AssignStr, 1))
	},
	{	// AssignLeft_Code
		Operator_TokenType, OneWord_Multiple,
		"LEFT$(", "AssignLeft", "LET",
		Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignLeft))
	},
	{	// AssignMid2_Code
		Operator_TokenType, OneWord_Multiple,
		"MID$(", "AssignMid2", "LET",
		Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignMid2))
	},
	{	// AssignMid3_Code
		Operator_TokenType, OneWord_Multiple,
		"MID$(", "AssignMid3", "LET",
		Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignMid3))
	},
	{	// AssignRight_Code
		Operator_TokenType, OneWord_Multiple,
		"MID$(", "AssignRight", "LET",
		Reference_Flag, 4, String_DataType,
		new ExprInfo(Null_Code, Operands(StrStr), AssocCode(AssignRight))
	},
	{	// AssignList_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList", "LET",
		Reference_Flag, 4, Double_DataType, &DblDbl_ExprInfo
	},
	{	// AssignListInt_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList%", "LET",
		Reference_Flag, 4, Integer_DataType,
		&IntInt_ExprInfo
	},
	{	// AssignListStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignList$", "LET",
		Reference_Flag, 4, String_DataType, &StrStr_ExprInfo
	},
	{	// AssignKeepStr_Code
		Operator_TokenType, OneWord_Multiple,
		"=", "AssignKeep$", "LET",
		Reference_Flag, 4, String_DataType, &StrStr_ExprInfo
	},
	{	// AssignKeepLeft_Code
		Operator_TokenType, OneWord_Multiple,
		"LEFT$(", "AssignKeepLeft", "LET",
		Reference_Flag, 4, String_DataType,
		&StrStr_ExprInfo
	},
	{	// AssignKeepMid2_Code
		Operator_TokenType, OneWord_Multiple,
		"MID$(", "AssignKeepMid2", "LET",
		Reference_Flag, 4, String_DataType,
		&StrStr_ExprInfo
	},
	{	// AssignKeepMid3_Code
		Operator_TokenType, OneWord_Multiple,
		"MID$(", "AssignKeepMid3", "LET",
		Reference_Flag, 4, String_DataType,
		&StrStr_ExprInfo
	},
	{	// AssignKeepRight_Code
		Operator_TokenType, OneWord_Multiple,
		"RIGHT$(", "AssignKeepRight", "LET",
		Reference_Flag, 4, String_DataType,
		&StrStr_ExprInfo
	},
	{	// EOL_Code
		Operator_TokenType, OneWord_Multiple,
		NULL, "EOL", NULL,
		EndStmt_Flag, 4, None_DataType
	},
	{	// AddI1_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%1", NULL,
		Null_Flag, 40, Double_DataType, &IntDbl_ExprInfo
	},
	{	// AddI2_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%2", NULL,
		Null_Flag, 40, Double_DataType, &DblInt_ExprInfo
	},
	{	// AddInt_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+%", NULL,
		Null_Flag, 40, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(AddInt))
	},
	{	// CatStr_Code
		Operator_TokenType, OneChar_Multiple,
		"+", "+$", NULL,
		Null_Flag, 40, String_DataType, &StrStr_ExprInfo
	},
	{	// SubI1_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%1", NULL,
		Null_Flag, 40, Double_DataType, &IntDbl_ExprInfo
	},
	{	// SubI2_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%2", NULL,
		Null_Flag, 40, Double_DataType, &DblInt_ExprInfo
	},
	{	// SubInt_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "-%", NULL,
		Null_Flag, 40, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(SubInt))
	},
	{	// NegInt_Code
		Operator_TokenType, OneChar_Multiple,
		"-", "Neg%", NULL,
		Null_Flag, 40, Integer_DataType,
		new ExprInfo(NegInt_Code, Operands(Int))
	},
	{	// MulI1_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%1", NULL,
		Null_Flag, 46, Double_DataType, &IntDbl_ExprInfo
	},
	{	// MulI2_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%2", NULL,
		Null_Flag, 46, Double_DataType, &DblInt_ExprInfo
	},
	{	// MulInt_Code
		Operator_TokenType, OneChar_Multiple,
		"*", "*%", NULL,
		Null_Flag, 46, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(MulInt))
	},
	{	// DivI1_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%1", NULL,
		Null_Flag, 46, Double_DataType, &IntDbl_ExprInfo
	},
	{	// DivI2_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%2", NULL,
		Null_Flag, 46, Double_DataType, &DblInt_ExprInfo
	},
	{	// DivInt_Code
		Operator_TokenType, OneChar_Multiple,
		"/", "/%", NULL,
		Null_Flag, 46, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(DivInt))
	},
	{	// ModI1_Code
		Operator_TokenType, OneChar_Multiple,
		"MOD", "MOD%1", NULL,
		Null_Flag, 42, Double_DataType, &IntDbl_ExprInfo
	},
	{	// ModI2_Code
		Operator_TokenType, OneChar_Multiple,
		"MOD", "MOD%2", NULL,
		Null_Flag, 42, Double_DataType, &DblInt_ExprInfo
	},
	{	// ModInt_Code
		Operator_TokenType, OneWord_Multiple,
		"MOD", "MOD%", NULL,
		Null_Flag, 42, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(ModInt))
	},
	{	// PowerI1_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^%1", NULL,
		Null_Flag, 50, Double_DataType, &IntDbl_ExprInfo
	},
	{	// PowerMul_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^*", NULL,
		Null_Flag, 50, Double_DataType, &DblInt_ExprInfo
	},
	{	// PowerInt_Code
		Operator_TokenType, OneChar_Multiple,
		"^", "^%", NULL,
		Null_Flag, 50, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(PowerInt))
	},
	{	// EqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%1", NULL,
		Null_Flag, 30, Integer_DataType, &IntDbl_ExprInfo
	},
	{	// EqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%2", NULL,
		Null_Flag, 30, Integer_DataType, &DblInt_ExprInfo
	},
	{	// EqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=%", NULL,
		Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(EqInt))
	},
	{	// EqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"=", "=$", NULL,
		Null_Flag, 30, Integer_DataType, &StrStr_ExprInfo
	},
	{	// GtI1_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%1", NULL,
		Null_Flag, 32, Integer_DataType, &IntDbl_ExprInfo
	},
	{	// GtI2_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%2", NULL,
		Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// GtInt_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">%", NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(GtInt))
	},
	{	// GtStr_Code
		Operator_TokenType, OneChar_Multiple,
		">", ">$", NULL,
		Null_Flag, 32, Integer_DataType, &StrStr_ExprInfo
	},
	{	// GtEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%1", NULL,
		Null_Flag, 32, Integer_DataType, &IntDbl_ExprInfo
	},
	{	// GtEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%2", NULL,
		Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// GtEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=%", NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(GtEqInt))
	},
	{	// GtEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		">=", ">=$", NULL,
		Null_Flag, 32, Integer_DataType, &StrStr_ExprInfo
	},
	{	// LtI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%1", NULL,
		Null_Flag, 32, Integer_DataType, &IntDbl_ExprInfo
	},
	{	// LtI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%2", NULL,
		Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// LtInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<%", NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(LtInt))
	},
	{	// LtStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<", "<$", NULL,
		Null_Flag, 32, Integer_DataType, &StrStr_ExprInfo
	},
	{	// LtEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%1", NULL,
		Null_Flag, 32, Integer_DataType, &IntDbl_ExprInfo
	},
	{	// LtEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%2", NULL,
		Null_Flag, 32, Integer_DataType, &DblInt_ExprInfo
	},
	{	// LtEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=%", NULL,
		Null_Flag, 32, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(LtEqInt))
	},
	{	// LtEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<=", "<=$", NULL,
		Null_Flag, 32, Integer_DataType, &StrStr_ExprInfo
	},
	{	// NotEqI1_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%1", NULL,
		Null_Flag, 30, Integer_DataType, &IntDbl_ExprInfo
	},
	{	// NotEqI2_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%2", NULL,
		Null_Flag, 30, Integer_DataType, &DblInt_ExprInfo
	},
	{	// NotEqInt_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>%", NULL,
		Null_Flag, 30, Integer_DataType,
		new ExprInfo(Null_Code, Operands(IntInt), AssocCode(NotEqInt))
	},
	{	// NotEqStr_Code
		Operator_TokenType, OneChar_Multiple,
		"<>", "<>$", NULL,
		Null_Flag, 30, Integer_DataType, &StrStr_ExprInfo
	},
	{	// AbsInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"ABS(", "ABS%(", NULL,
		Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// RndArgInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"RND(", "RND%(", NULL,
		Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// SgnInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"SGN(", "SGN%(", NULL,
		Null_Flag, 2, Integer_DataType, &Int_ExprInfo
	},
	{	// CvtInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtInt", NULL,
		Hidden_Flag, 2, None_DataType
	},
	{	// CvtDbl_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "CvtDbl", NULL,
		Hidden_Flag, 2, None_DataType
	},
	{	// StrInt_Code
		IntFuncP_TokenType, OneWord_Multiple,
		"STR$(", "STR%$(", NULL,
		Null_Flag, 2, String_DataType, &Int_ExprInfo
	},
	{	// PrintDbl_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "PrintDbl", NULL,
		Print_Flag | UseConstAsIs_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Print))
	},
	{	// PrintInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "PrintInt", NULL,
		Print_Flag, 2, None_DataType, &Int_ExprInfo
	},
	{	// PrintStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "PrintStr", NULL,
		Print_Flag, 2, None_DataType, &Str_ExprInfo
	},
	{	// InputBegin_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputBegin", NULL,
		Null_Flag, 2, None_DataType
	},
	{	// InputBeginStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputBeginStr", "Question",
		Null_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Str))
	},
	{	// InputAssign_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputAssign", NULL,
		Reference_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode2(InputAssign, 2))
	},
	{	// InputAssignInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputAssignInt", NULL,
		Reference_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Int), AssocCode(InputAssignInt))
	},
	{	// InputAssignStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputAssignStr", NULL,
		Reference_Flag, 2, None_DataType,
		new ExprInfo(Null_Code, Operands(Str), AssocCode(InputAssignStr))
	},
	{	// InputParse_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputParse", NULL,
		Null_Flag, 2, None_DataType
	},
	{	// InputParseInt_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputParseInt", NULL,
		Null_Flag, 2, None_DataType
	},
	{	// InputParseStr_Code
		IntFuncN_TokenType, OneWord_Multiple,
		NULL, "InputParseStr", NULL,
		Null_Flag, 2, None_DataType
	},
	{	// Const_Code
		Constant_TokenType, OneWord_Multiple,
		NULL, "Const", NULL,
		Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Const)),
		NULL, constNumEncode, constNumOperandText, constNumRemove
	},
	{	// ConstInt_Code
		Constant_TokenType, OneWord_Multiple,
		NULL, "ConstInt", NULL,
		Null_Flag, 2, Integer_DataType, &Int_ExprInfo,
		NULL, constNumEncode, constNumOperandText, constNumRemove
	},
	{	// ConstStr_Code
		Constant_TokenType, OneWord_Multiple,
		NULL, "ConstStr", NULL,
		Null_Flag, 2, String_DataType, &Str_ExprInfo,
		NULL, constStrEncode, constStrOperandText, constStrRemove
	},
	{	// Var_Code
		NoParen_TokenType, OneWord_Multiple,
		NULL, "Var", NULL,
		Null_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(Var)),
		NULL, varDblEncode, varDblOperandText, varDblRemove
	},
	{	// VarInt_Code
		NoParen_TokenType, OneWord_Multiple,
		NULL, "VarInt", NULL,
		Null_Flag, 2, Integer_DataType, &Int_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove
	},
	{	// VarStr_Code
		NoParen_TokenType, OneWord_Multiple,
		NULL, "VarStr", NULL,
		Null_Flag, 2, String_DataType, &Str_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove
	},
	{	// VarRef_Code
		NoParen_TokenType, OneWord_Multiple,
		NULL, "VarRef", NULL,
		Reference_Flag, 2, Double_DataType,
		new ExprInfo(Null_Code, Operands(Dbl), AssocCode(VarRef)),
		NULL, varDblEncode, varDblOperandText, varDblRemove
	},
	{	// VarRefInt_Code
		NoParen_TokenType, OneWord_Multiple,
		NULL, "VarRefInt", NULL,
		Reference_Flag, 2, Integer_DataType, &Int_ExprInfo,
		NULL, varIntEncode, varIntOperandText, varIntRemove
	},
	{	// VarRefStr_Code
		NoParen_TokenType, OneWord_Multiple,
		NULL, "VarRefStr", NULL,
		Reference_Flag, 2, String_DataType, &Str_ExprInfo,
		NULL, varStrEncode, varStrOperandText, varStrRemove
	}
};


// static function to return a refernce to the single table instance
//
//   - creates the single table instance with the table entries
//   - constructor fails (abort application) if there are table errors

Table &Table::instance(void)
{
	if (s_instance == NULL)
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
	int maxOperands = 0;
	int maxAssocCodes = 0;
	for (i = 0; i < entryCount; i++)
	{
		// check if found new maximums
		ExprInfo *exprInfo = m_entry[i].exprInfo;
		if (exprInfo != NULL)
		{
			if (maxOperands < exprInfo->m_operandCount)
			{
				maxOperands = exprInfo->m_operandCount;
			}
			if (maxAssocCodes < exprInfo->m_associatedCodeCount)
			{
				maxAssocCodes = exprInfo->m_associatedCodeCount;
			}

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

			// validate multiple entries
			if (m_entry[i].flags & Multiple_Flag != 0)
			{
				ExprInfo *exprInfo2 = m_entry[i + 1].exprInfo;
				if (m_entry[i].name != m_entry[i + 1].name)
				{
					errorList.append(QString("Multiple entry '%1' name "
						"mis-match '%2'").arg(m_entry[i].name)
						.arg(m_entry[i + 1].name));
				}
				else if (exprInfo2 == NULL)
				{
					errorList.append(QString("Multiple entry '%1' next entry "
						"no expression info").arg(m_entry[i + 1].name));
				}
				else if (exprInfo2->m_operandCount
					!= exprInfo->m_operandCount + 1)
				{
					errorList.append(QString("Multiple entry '%1' incorrect "
						"number of operands (%2, %3)").arg(m_entry[i].name)
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
			int bitMaskDataType[numberof_DataType] = {
				Dbl_BitMask,	// Double
				Int_BitMask,	// Integer
				Str_BitMask		// String
			};

			// set expected data type (start with data type of last operand)
			if (exprInfo->m_operandCount > 0)
			{
				// use last operand for operators, first operand for functions
				exprInfo->m_expectedDataType
					= exprInfo->m_operandDataType[m_entry[i].type
					== Operator_TokenType ? exprInfo->m_operandCount - 1 : 0];
				// check each secondary associated code
				if (exprInfo->m_associatedCodeCount > 0)
				{
					int bitMask = bitMaskDataType[exprInfo->m_expectedDataType];
					int index = exprInfo->m_secondAssociatedIndex;
					if (index >= 0)
					{
						for (; index < exprInfo->m_associatedCodeCount; index++)
						{
							Code assocCode = exprInfo->m_associatedCode[index];
							ExprInfo *exprInfo2 = m_entry[assocCode].exprInfo;
							if (exprInfo2 != NULL)
							{
								bitMask |= bitMaskDataType[exprInfo2
									->m_operandDataType[exprInfo2
									->m_operandCount - 1]];
							}
						}
						if (bitMask == Num_BitMask)
						{
							exprInfo->m_expectedDataType = Number_DataType;
						}
						else if (bitMask == Any_BitMask)
						{
							exprInfo->m_expectedDataType = Any_DataType;
						}
					}
				}
			}
		}
	}

	// check maximums found against constants
	if (maxOperands != MaxOperands)
	{
		errorList.append(QString("Max_Operands=%1 incorrect, actual is %2")
			.arg(MaxOperands).arg(maxOperands));
	}
	if (maxAssocCodes != MaxAssocCodes)
	{
		errorList.append(QString("Max_Assoc_Codes=%1 incorrect, actual is %2")
			.arg(MaxAssocCodes).arg(maxAssocCodes));
	}

	// setup indexes for bracketing codes
	// (will be set to -1 if missing - missing errors were recorded above)
	m_range[PlainWord_SearchType].beg = BegPlainWord_Code;
	m_range[PlainWord_SearchType].end = EndPlainWord_Code;
	m_range[ParenWord_SearchType].beg = BegParenWord_Code;
	m_range[ParenWord_SearchType].end = EndParenWord_Code;
	m_range[DataTypeWord_SearchType].beg = BegDataTypeWord_Code;
	m_range[DataTypeWord_SearchType].end = EndDataTypeWord_Code;
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
			for (int type2 = 0; type2 < sizeof_SearchType; type2++)
			{
				if (type != type2
					&& (m_range[type].beg > m_range[type2].beg
					&& m_range[type].beg < m_range[type2].end
					|| m_range[type].end > m_range[type2].beg
					&& m_range[type].end < m_range[type2].end))
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
		int n = 0;
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
TokenType Table::type(Code code) const
{
	return m_entry[code].type;
}

// returns data type for code
DataType Table::dataType(Code code) const
{
	return m_entry[code].dataType;
}

// returns multiple word or character type for code
Multiple Table::multiple(Code code) const
{
	return m_entry[code].multiple;
}

// returns primary name for code
const QString Table::name(Code code) const
{
	return m_entry[code].name;
}

// returns second name of a two word command for code
const QString Table::name2(Code code) const
{
	return m_entry[code].name2;
}

const QString Table::optionName(Code code) const
{
	return m_entry[code].option;
}

// returns the debug name for code, which is the primary name
// except for internal functions with multiple argument footprints
const QString Table::debugName(Code code) const
{
	QString name = code == Invalid_Code ? "<NotSet>" : m_entry[code].name2;
	if (name.isEmpty())
	{
		name = m_entry[code].name;
	}
	return name;
}

// returns the flags for code
int Table::hasFlag(Code code, int flag) const
{
	return m_entry[code].flags & flag;
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
	return m_entry[code].operandText != NULL;
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

//=================================
//  TOKEN RELATED TABLE FUNCTIONS
//=================================

// returns the unary operator code (or Null_Code if none) for token code
Code Table::unaryCode(Token *token) const
{
	if (token->isType(Operator_TokenType))
	{
		ExprInfo *ei = m_entry[token->code()].exprInfo;
		if (ei != NULL)
		{
			return ei->m_unaryCode;
		}
	}
	return Null_Code;
}

// returns whether the token contains a unary operator code
// (convenience function to avoid confusion)
bool Table::isUnaryOperator(Token *token) const
{
	return token->isType(Operator_TokenType) ? operandCount(token) == 1 : false;
}

// returns whether the token is a unary or binary operator
// (token type must be operator and have operands)
bool Table::isUnaryOrBinaryOperator(Token *token) const
{
	return token->isType(Operator_TokenType) ? operandCount(token) > 0 : false;
}

// returns the precedence of the code contained in a token
//
//   - the precedence is obtained from the token
//   - if this is -1 then the precedences if obtained for the token's code
int Table::precedence(Token *token) const
{
	int prec = token->precedence();
	return prec != -1 ? prec : precedence(token->code());
}

// returns the flags of the code contained in a token
//
//   - returns Null_Flag is the token does not contain a code
int Table::hasFlag(Token *token, int flag) const
{
	// (invalid code tokens have no flags)
	return token->hasValidCode() ? hasFlag(token->code(), flag) : 0;
}

// returns number of operands expected for code in token token
int Table::operandCount(Token *token) const
{
	ExprInfo *exprInfo = m_entry[token->code()].exprInfo;
	return exprInfo == NULL ? 0 : exprInfo->m_operandCount;
}

// returns the expected data type for last operand for operator token
DataType Table::expectedDataType(Token *token) const
{
	return expectedDataType(token->code());
}

// function to set a token for a code (code, token type and data type)
void Table::setToken(Token *token, Code code)
{
	token->setCode(code);
	token->setType(type(code));
	token->setDataType(dataType(code));
}

// function to create a new token and initialize it for a code
Token *Table::newToken(Code code)
{
	Token *token = new Token;	// allocates and initializes base members
	setToken(token, code);		// initializes code related members
	return token;
}

// function to get convert code needed to convert token to data type
Code Table::cvtCode(Token *token, DataType dataType) const
{
	return cvtCodeHaveNeed[token->dataType()][dataType];
}


// function to find and set code in token for a data type
// and possibly return a conversion code if data type is convertible
//
//   - if data type matches expected data type for operand, no action
//   - else finds associated code that matches data type
//   - if no associated code, returns possible conversion code for data type
//   - if no associated code, returns expected data type

Code Table::findCode(Token *token, Token *operandToken, int operandIndex)
{
	DataType expectedDataType = operandDataType(token->code(), operandIndex);

	if (operandToken->dataType() == expectedDataType     // exact match?
		|| operandIndex == operandCount(token) - 1       // last operand?
		&& operandToken->isType(Constant_TokenType)
		&& (expectedDataType == Double_DataType
		|| operandToken->isDataType(Integer_DataType))
		&& !hasFlag(token, UseConstAsIs_Flag)
		&& !operandToken->isDataType(String_DataType))
	{
		// force token to expected data type and remove double sub-code
		// (applies only to constant token type, but safe for all token types)
		operandToken->setDataType(expectedDataType);
		operandToken->removeSubCode(Double_SubCode);
		if (operandToken->isType(Constant_TokenType))
		{
			setTokenCode(operandToken, Const_Code);
		}
		return Null_Code;
	}

	// see if any associated code's data types match
	if (setTokenCode(token, token->code(), operandToken->dataType(),
		operandIndex))
	{
		if (operandToken->isType(Constant_TokenType))
		{
			setTokenCode(operandToken, Const_Code);
		}
		return Null_Code;
	}

	// get a conversion code if no associated code was found
	Code cvtCode = cvtCodeHaveNeed[operandToken->dataType()][expectedDataType];

	// did not find an associated code, return conversion code
	if (cvtCode == Invalid_Code)
	{
		// set operand token to return expected data type
		operandToken->setDataType(expectedDataType);
	}
	else if (operandToken->isType(Constant_TokenType))
	{
		if (cvtCode == CvtInt_Code && operandToken->isDataType(Double_DataType))
		{
			// double constant can't be converted to an integer
			operandToken->setDataType(No_DataType);
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

bool Table::setTokenCode(Token *token, Code code, DataType dataType,
	int operandIndex)
{
	// first check if data type already matches data type of code
	if (dataType != operandDataType(code, operandIndex))
	{
		// if not, see if data type of any associated code matches
		int i = secondAssociatedIndex(code);
		if (i < 0)  // if second index -1, then no associated codes to search
		{
			return false;
		}
		// determine range of associated codes to search
		int end = associatedCodeCount(code);
		if (operandIndex == 0 && i != 0)  // first operand?
		{
			// if first operand and there are second operand associated codes
			// then set end to start of second operand associated codes
			// and start to first associated code
			end = i;
			i = 0;
		}
		Code *associatedCodes = associatedCodeArray(code);
		forever
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

Code Table::search(SearchType type, const QStringRef &string) const
{
	Code i = m_range[type].beg;
	Code end = m_range[type].end;
	while (++i < end)
	{
		if (string.compare(m_entry[i].name, Qt::CaseInsensitive) == 0)
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

Code Table::search(const QStringRef &word1, const QStringRef &word2) const
{
	for (Code i = m_range[PlainWord_SearchType].beg;
		i < m_range[PlainWord_SearchType].end; i++)
	{
		if (m_entry[i].name2 != NULL
			&& word1.compare(m_entry[i].name, Qt::CaseInsensitive) == 0
			&& word2.compare(m_entry[i].name2, Qt::CaseInsensitive) == 0)
		{
			return i;
		}
	}
	return Invalid_Code;
}


// search function to look for a function matching the same function
// name as the index specified and matching the same number of
// arguments specified
//
//   - returns the index of the entry that is found
//   - returns -1 if the function with same noperands was not found
//   - case sensitive compare used (all entry names must match)
//   - name of function found at index specified
//   - search begins at entry after index
//   - search ends at end of section

Code Table::search(Code index, int argumentCount) const
{
	for (Code i = index + 1; m_entry[i].name != NULL; i++)
	{
		if (m_entry[index].name == m_entry[i].name
			&& argumentCount == operandCount(i))
		{
			return i;
		}
	}
	return Invalid_Code;
}


// search function to look for a code matching the same operand
// data types as specified in the argument, the main code is
// checked first and then each of the associated codes
//
//   - returns the index of the code that is found
//   - returns -1 if there is not matching code
//   - the code specified must have associated codes
//   - the number of data types must match that of the code

Code Table::search(Code code, DataType *datatype) const
{
	if (match(code, datatype))
	{
		return code;  // main code matches
	}

	for (int n = associatedCodeCount(code); --n >= 0;)
	{
		Code assoc_code = this->associatedCode(code, n);
		if (match(assoc_code, datatype))
		{
			return assoc_code;  // associated code matches
		}
	}
	return Invalid_Code;  // no matches
}

// function to check to see if data types specified match the data
// types of the code at the index specified
//
//   - returns true if there is match, otherwise returns false

bool Table::match(Code code, DataType *datatype) const
{
	for (int n = operandCount(code); --n >= 0;)
	{
		if (datatype[n] != operandDataType(code, n))
		{
			return false;
		}
	}
	return true;
}


// end: table.cpp
