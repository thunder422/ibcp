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

// include auto-generated enumerations
#include "autoenums.h"


// code operator in-line functions for additional, increment and decrement
inline Code operator +(Code code, int number)
{
	return (Code)((int)code + number);
}

inline Code operator ++(Code &code)
{
	return code = code + 1;
}

inline Code operator ++(Code &code, int postfix)
{
	return code = code + 1;
}


// data type of operands
enum DataType
{
	No_DataType = -1,
	// actual execution data types must be listed first
	// since they will also be used for indexes
	Double_DataType,
	Integer_DataType,
	String_DataType,
	// end of the actual execution data types
	// the following data types are used internally for other uses
	None_DataType,		// indicates none of the above data types
	// number of actual execution data types
	numberof_DataType = None_DataType,
	Number_DataType,	// either Double or Integer
	Any_DataType,		// any type (Double, Integer or String)
	sizeof_DataType
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
	Used_SubCode		= 0x00010000,	// parentheses used in output
	Last_SubCode		= 0x00020000,	// parentheses used as last token
	UnUsed_SubCode		= 0x00040000	// token not in output (for errors)
};


#endif  // IBCP_H
