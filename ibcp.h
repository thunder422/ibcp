// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: ibcp.h - main header file
//	Copyright (C) 2010-2012  Thunder422
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

#ifndef IBCP_H
#define IBCP_H

#include <cstddef>


// parser and translator status codes
enum class Status
{
	ExpCmdOrAssignItem = 1,
	ExpExpr,
	ExpExprOrEnd,
	ExpOpOrEnd,
	ExpBinOpOrEnd,
	ExpEqualOrComma,
	ExpComma,
	ExpAssignItem,
	ExpOpOrComma,
	ExpOpCommaOrParen,
	ExpOpOrParen,
	ExpBinOpOrComma,
	ExpBinOpCommaOrParen,
	ExpBinOpOrParen,
	ExpNumExpr,
	ExpStrExpr,
	ExpSemiCommaOrEnd,
	ExpCommaSemiOrEnd,
	ExpSemiOrComma,
	ExpOpSemiOrComma,
	ExpDblVar,
	ExpIntVar,
	ExpStrVar,
	ExpVar,
	ExpStrItem,
	ExpEndStmt,
	ExpExprPfnOrEnd,
	ExpExprCommaPfnOrEnd,
	ExpOpSemiCommaOrEnd,
	ExpIntConst,
	// the following are parser errors
	UnknownToken,
	ExpNonZeroDigit,
	ExpDigitsOrSngDP,
	ExpManDigits,
	ExpExpSignOrDigits,
	ExpExpDigits,
	ExpDigits,
	FPOutOfRange,
	// the following statuses used during development
	BUG_NotYetImplemented,
	BUG_HoldStackNotEmpty,
	BUG_DoneStackNotEmpty,
	BUG_DoneStackEmptyFindCode,
	BUG_UnexpectedCloseParen,
	BUG_DoneStackEmpty,
	BUG_InvalidDataType,
	BUG_UnexpToken,
	BUG_Debug1,
	BUG_Debug2,
	BUG_Debug3,
	BUG_Debug4,
	BUG_Debug5,
	BUG_Debug6,
	BUG_Debug7,
	BUG_Debug8,
	BUG_Debug9,
	BUG_Debug
};


// code enumeration
enum Code
{
	Invalid_Code = -1,		// REMOVE possibly use can be removed
	Null_Code,				// REMOVE will be replaced with Code{}
	Let_Code,
	Print_Code,				// REMOVE could be alternate[0] of SemiColon
	Input_Code,				// REMOVE will be removed
	InputPrompt_Code,		// REMOVE will be removed
	Dim_Code,				// REMOVE not currently used
	Def_Code,				// REMOVE not currently used
	Rem_Code,
	If_Code,				// REMOVE not currently used
	Then_Code,				// REMOVE not currently used
	Else_Code,				// REMOVE not currently used
	End_Code,				// REMOVE not currently used
	EndIf_Code,				// REMOVE not currently used
	For_Code,				// REMOVE not currently used
	To_Code,				// REMOVE not currently used
	Step_Code,				// REMOVE not currently used
	Next_Code,				// REMOVE not currently used
	Do_Code,				// REMOVE not currently used
	DoWhile_Code,			// REMOVE not currently used
	DoUntil_Code,			// REMOVE not currently used
	While_Code,				// REMOVE not currently used
	Until_Code,				// REMOVE not currently used
	Loop_Code,				// REMOVE not currently used
	LoopWhile_Code,			// REMOVE not currently used
	LoopUntil_Code,			// REMOVE not currently used
	Rnd_Code,				// REMOVE primary function (will not be needed)
	Mod_Code,				// REMOVE primary operator (will not be needed)
	And_Code,				// REMOVE primary operator (will not be needed)
	Or_Code,				// REMOVE primary operator (will not be needed)
	Not_Code,				// REMOVE primary operator (will not be needed)
	Eqv_Code,				// REMOVE primary operator (will not be needed)
	Imp_Code,				// REMOVE primary operator (will not be needed)
	Xor_Code,				// REMOVE primary operator (will not be needed)
	Abs_Code,				// REMOVE primary function (will not be needed)
	Fix_Code,				// REMOVE primary function (will not be needed)
	Frac_Code,				// REMOVE primary function (will not be needed)
	Int_Code,				// REMOVE primary function (will not be needed)
	RndArg_Code,			// REMOVE primary function (will not be needed)
	Sgn_Code,				// REMOVE primary function (will not be needed)
	Cint_Code,				// REMOVE primary function (will not be needed)
	Cdbl_Code,				// REMOVE primary function (will not be needed)
	Sqr_Code,				// REMOVE primary function (will not be needed)
	Atn_Code,				// REMOVE primary function (will not be needed)
	Cos_Code,				// REMOVE primary function (will not be needed)
	Sin_Code,				// REMOVE primary function (will not be needed)
	Tan_Code,				// REMOVE primary function (will not be needed)
	Exp_Code,				// REMOVE primary function (will not be needed)
	Log_Code,				// REMOVE primary function (will not be needed)
	Tab_Code,				// REMOVE primary function (will not be needed)
	Spc_Code,				// REMOVE primary function (will not be needed)
	Asc_Code,				// REMOVE primary function (will not be needed)
	Asc2_Code,				// REMOVE alternate[1] of Asc
	Chr_Code,				// REMOVE primary function (will not be needed)
	Instr2_Code,			// REMOVE primary function (will not be needed)
	Instr3_Code,			// REMOVE alternate[2] of Instr2
	Left_Code,				// REMOVE primary function (will not be needed)
	Len_Code,				// REMOVE primary function (will not be needed)
	Mid2_Code,				// REMOVE primary function (will not be needed)
	Mid3_Code,				// REMOVE alternate[2] of Mid2
	Repeat_Code,			// REMOVE primary function (will not be needed)
	Right_Code,				// REMOVE primary function (will not be needed)
	Space_Code,				// REMOVE primary function (will not be needed)
	Str_Code,				// REMOVE primary function (will not be needed)
	Val_Code,				// REMOVE primary function (will not be needed)
	Add_Code,				// REMOVE primary operator (will not be needed)
	Neg_Code,				// REMOVE primary operator (will not be needed)
	Mul_Code,				// REMOVE primary operator (will not be needed)
	Div_Code,				// REMOVE primary operator (will not be needed)
	IntDiv_Code,			// REMOVE primary operator (will not be needed)
	Power_Code,				// REMOVE primary operator (will not be needed)
	Eq_Code,
	Gt_Code,				// REMOVE primary operator (will not be needed)
	GtEq_Code,				// REMOVE primary operator (will not be needed)
	Lt_Code,				// REMOVE primary operator (will not be needed)
	LtEq_Code,				// REMOVE primary operator (will not be needed)
	NotEq_Code,				// REMOVE primary operator (will not be needed)
	OpenParen_Code,
	CloseParen_Code,
	Comma_Code,
	SemiColon_Code,
	Colon_Code,
	RemOp_Code,
	Assign_Code,			// REMOVE could be alternate to Let
	AssignInt_Code,			// REMOVE alternate[0] of Assign
	AssignStr_Code,			// REMOVE alternate[0] of Assign
	AssignLeft_Code,		// REMOVE alternate[1] of Left
	AssignMid2_Code,		// REMOVE alternate[1] of Mid2
	AssignMid3_Code,		// REMOVE alternate[1] of Mid3
	AssignRight_Code,		// REMOVE alternate[1] of Right
	AssignList_Code,		// REMOVE alternate[1] of Assign
	AssignListInt_Code,		// REMOVE alternate[1] of AssignInt
	AssignListStr_Code,		// REMOVE alternate[1] of AssignStr
	AssignKeepStr_Code,		// REMOVE alternate[0] of AssignStr
	AssignKeepLeft_Code,	// REMOVE alternate[0] of AssignLeft
	AssignKeepMid2_Code,	// REMOVE alternate[0] of AssignMid2
	AssignKeepMid3_Code,	// REMOVE alternate[0] of AssignMid3
	AssignKeepRight_Code,	// REMOVE alternate[0] of AssignRight
	EOL_Code,
	AddI1_Code,				// REMOVE alternate[0] of Add
	AddI2_Code,				// REMOVE alternate[1] of Add
	AddInt_Code,			// REMOVE alternate[1] of AddI1
	CatStr_Code,			// REMOVE alternate[0] of Add
	Sub_Code,				// REMOVE alternate[1] of Neg
	SubI1_Code,				// REMOVE alternate[0] of Sub
	SubI2_Code,				// REMOVE alternate[1] of Sub
	SubInt_Code,			// REMOVE alternate[1] of SubI1
	NegInt_Code,			// REMOVE alternate[0] of Neg
	MulI1_Code,				// REMOVE alternate[0] of Mul
	MulI2_Code,				// REMOVE alternate[1] of Mul
	MulInt_Code,			// REMOVE alternate[1] of MulI1
	DivI1_Code,				// REMOVE alternate[0] of Div
	DivI2_Code,				// REMOVE alternate[1] of Div
	DivInt_Code,			// REMOVE alternate[1] of DivI1
	ModI1_Code,				// REMOVE alternate[0] of Mod
	ModI2_Code,				// REMOVE alternate[1] of Mod
	ModInt_Code,			// REMOVE alternate[1] of ModI1
	PowerI1_Code,			// REMOVE alternate[0] of Power
	PowerMul_Code,			// REMOVE alternate[1] of Power
	PowerInt_Code,			// REMOVE alternate[1] of PowerI1
	EqI1_Code,				// REMOVE alternate[0] of Eq
	EqI2_Code,				// REMOVE alternate[1] of Eq
	EqInt_Code,				// REMOVE alternate[1] of EqI1
	EqStr_Code,				// REMOVE alternate[0] of Eq
	GtI1_Code,				// REMOVE alternate[0] of Gt
	GtI2_Code,				// REMOVE alternate[1] of Gt
	GtInt_Code,				// REMOVE alternate[1] of GtI1
	GtStr_Code,				// REMOVE alternate[0] of Gt
	GtEqI1_Code,			// REMOVE alternate[0] of GtEq
	GtEqI2_Code,			// REMOVE alternate[1] of GtEq
	GtEqInt_Code,			// REMOVE alternate[1] of GtEqI1
	GtEqStr_Code,			// REMOVE alternate[0] of GtEq
	LtI1_Code,				// REMOVE alternate[0] of Lt
	LtI2_Code,				// REMOVE alternate[1] of Lt
	LtInt_Code,				// REMOVE alternate[1] of LtI1
	LtStr_Code,				// REMOVE alternate[0] of Lt
	LtEqI1_Code,			// REMOVE alternate[0] of LtEq
	LtEqI2_Code,			// REMOVE alternate[1] of LtEq
	LtEqInt_Code,			// REMOVE alternate[1] of LtEqI1
	LtEqStr_Code,			// REMOVE alternate[0] of LtEq
	NotEqI1_Code,			// REMOVE alternate[0] of NotEq
	NotEqI2_Code,			// REMOVE alternate[1] of NotEq
	NotEqInt_Code,			// REMOVE alternate[1] of NotEqI1
	NotEqStr_Code,			// REMOVE alternate[0] of NotEq
	AbsInt_Code,			// REMOVE alternate[0] of Abs
	RndArgInt_Code,			// REMOVE alternate[0] of RngArg
	SgnInt_Code,			// REMOVE alternate[0] of Sgn
	CvtInt_Code,
	CvtDbl_Code,
	StrInt_Code,			// REMOVE alternate[0] of Str
	PrintDbl_Code,			// REMOVE could be alternate[0] of Print
	PrintInt_Code,			// REMOVE alternate[0] of PrintDbl
	PrintStr_Code,			// REMOVE alternate[0] of PrintDbl
	InputBegin_Code,		// REMOVE could be alternate[0] of Input
	InputBeginStr_Code,		// REMOVE could be alternate[0] of InputPrompt
	InputAssign_Code,		// REMOVE could be alternate[1] of Input
	InputAssignInt_Code,	// REMOVE alternate[0] of InputAssign
	InputAssignStr_Code,	// REMOVE alternate[0] of InputAssign
	InputParse_Code,		// REMOVE alternate[1] of InputAssign
	InputParseInt_Code,		// REMOVE alternate[1] of InputAssignInt
	InputParseStr_Code,		// REMOVE alternate[1] of InputAssignStr
	Const_Code,
	ConstInt_Code,			// REMOVE alternate[0] of Const
	ConstStr_Code,
	Var_Code,
	VarInt_Code,			// REMOVE alternate[0] of Var
	VarStr_Code,			// REMOVE alternate[0] of Var
	VarRef_Code,
	VarRefInt_Code,			// REMOVE alternate[0] of VarRef
	VarRefStr_Code,			// REMOVE alternate[0] of VarRef
	Array_Code,
	DefFuncN_Code,
	DefFuncP_Code,
	Function_Code
};


// miscellenous constant definitions
constexpr int HighestPrecedence {127};	// highest precedence value
	// this value was selected as the highest value because it is the highest
	// one-byte signed value (in case the precedence member is changed to a
	// char); all precedences in the table must be below this value

constexpr int BaseLineNumber {0};		// number of first line in program
	// this value is the number of the first line in the program edit box and
	// program view dock widget (a value of zero is used for development, but
	// will be changed to one)


// data type of operands
enum class DataType
{
	// actual execution data types must be listed first
	// since they will also be used for indexes
	Double = 1,
	Integer,
	String,
	// end of the actual execution data types
	// the following data types are used internally for other uses
	None,		// indicates none of the above data types
	Number,		// either Double or Integer
	Any 		// any type (Double, Integer or String)
};


// sub-code flags for use in Token and internal program
enum SubCode : uint16_t
{
	None_SubCode		= 0x0000,	// no sub-code present
	ProgramMask_SubCode	= 0xFC00,	// mask for actual program sub-codes
	Paren_SubCode		= 0x0400,	// reproduce unnecessary parenthesis
	Colon_SubCode		= 0x0800,	// reproduce ":" after token
	Option_SubCode		= 0x1000,	// reproduce command specific option
	// sub-codes used by translator only
	Double_SubCode		= 0x0001,	// integer constant has decimal/exponent

	// code program mask
	ProgramMask_Code	= 0x03FF	// mask for actual program codes
};


// provide generic hash for enum class with std::unordered_map
struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};


#endif  // IBCP_H
