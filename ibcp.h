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

// include auto-generated enumerations
#include "autoenums.h"


// parser and translator status codes
enum class Status
{
	Done,
	ExpCmdOrAssignItem,
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


// code operator in-line functions for additional, increment and decrement
inline Code operator +(Code code, int number)
{
	return (Code)((int)code + number);
}

inline Code operator ++(Code &code)
{
	return code = code + 1;
}

inline Code operator ++(Code &code, int)
{
	return code = code + 1;
}


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
enum SubCode
{
	None_SubCode		= 0x00000000,	// no sub-code present
	ProgramMask_SubCode	= 0x0000FC00,	// mask for actual program sub-codes
	Paren_SubCode		= 0x00000400,	// reproduce unnecessary parenthesis
	Colon_SubCode		= 0x00000800,	// reproduce ":" after token
	Option_SubCode		= 0x00001000,	// reproduce command specific option
	// sub-codes used by translator only
	Double_SubCode		= 0x00000001,	// integer constant has decimal/exponent

	// code program mask
	ProgramMask_Code	= 0x000003FF	// mask for actual program codes
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
