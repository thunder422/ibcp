// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: token.cpp - contains code for the token class
//	Copyright (C) 2012  Thunder422
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
//	2012-11-03	initial version (parts removed from ibcp.cpp)

#include "ibcp.h"
#include "token.h"


// static token variables
bool Token::paren[sizeof_TokenType];
bool Token::op[sizeof_TokenType];
int Token::prec[sizeof_TokenType];
bool Token::table[sizeof_TokenType];

// token status message array
//   (TokenStatus enumeration generated from names
//   in comments at the end each line by enums.awk,
//   lines starting with comments are ignored)
const QString Token::messageArray[sizeof_TokenStatus] = {
	"Null_TokenStatus (BUG)",							// Null
	"Good_TokenStatus (BUG)",							// Good
	"Done_TokenStatus (BUG)",							// Done
	"expected command",									// ExpCmd
	"expected expression",								// ExpExpr
	"expected expression or end-of-statement",			// ExpExprOrEnd
	"expected operator or end-of-statement",			// ExpOpOrEnd
	"expected binary operator or end-of-statement",		// ExpBinOpOrEnd
	"expected equal or comma for assignment",			// ExpEqualOrComma
	"expected comma",									// ExpComma
	"expected item for assignment",						// ExpAssignItem
	"expected operator or comma",						// ExpOpOrComma
	"expected operator, comma or closing parentheses",	// ExpOpCommaOrParen
	"expected operator or end-of-expression",			// NoOpenParen
	"expected operator or closing parentheses",			// ExpOpOrParen
	"expected double expression",						// ExpDouble
	"expected integer expression",						// ExpInteger
	"expected string expression (old)",					// ExpString
	"expected numeric expression",						// ExpNumExpr
	"expected string expression",						// ExpStrExpr
	"expected semicolon, comma or end-of-statement",	// ExpSemiCommaOrEnd
	"expected semicolon or comma",						// ExpSemiOrComma
	"expected operator, semicolon or comma",			// ExpOpSemiOrComma
	"expected double variable",							// ExpDblVar
	"expected integer variable",						// ExpIntVar
	"expected string variable",							// ExpStrVar
	"expected variable",								// ExpVar
	"expected string item for assignment",				// ExpStrItem
	"expected end-of-statement",						// ExpEndStmt
	// the following statuses used during development
	"BUG: not yet implemented",						// NotYetImplemented
	"BUG: invalid mode",							// InvalidMode
	"BUG: hold stack empty",						// HoldStackEmpty
	"BUG: hold stack not empty",					// HoldStackNotEmpty
	"BUG: done stack not empty",					// DoneStackNotEmpty
	"BUG: done stack empty - parentheses",			// DoneStackEmptyParen
	"BUG: done stack empty - operands",				// DoneStackEmptyOperands
	"BUG: done stack empty - operands 2",			// DoneStackEmptyOperands2
	"BUG: done stack empty - find code",			// DoneStackEmptyFindCode
	"BUG: unexpected closing parentheses",			// UnexpectedCloseParen
	"BUG: unexpected token on hold stack",			// UnexpectedToken
	"BUG: expected operand on done stack",			// DoneStackEmpty
	"BUG: command stack not empty",					// CmdStackNotEmpty
	"BUG: command stack empty",						// CmdStackEmpty
	"BUG: command stack empty for expression",		// CmdStackEmptyExpr
	"BUG: command stack empty for command",			// CmdStackEmptyCmd
	"BUG: no assign list code found",				// NoAssignListCode
	"BUG: invalid data type",						// InvalidDataType
	"BUG: count stack empty",						// CountStackEmpty
	"BUG: unexpected parentheses in expression",	// UnexpParenExpr
	"BUG: unexpected token",						// UnexpToken
	"BUG: debug #1",								// Debug1
	"BUG: debug #2",								// Debug2
	"BUG: debug #3",								// Debug3
	"BUG: debug #4",								// Debug4
	"BUG: debug #5",								// Debug5
	"BUG: debug #6",								// Debug6
	"BUG: debug #7",								// Debug7
	"BUG: debug #8",								// Debug8
	"BUG: debug #9",								// Debug9
	"BUG: debug"									// Debug
};


// function to initialize the static token data
void Token::initialize(void)
{
	// set true for types that contain an opening parentheses
	paren[IntFuncP_TokenType] = true;
	paren[DefFuncP_TokenType] = true;
	paren[Paren_TokenType] = true;

	// set true for types that are considered an operator
	op[Command_TokenType] = true;
	op[Operator_TokenType] = true;

	// set precedence for non-table token types
	prec[Command_TokenType] = -1;  // use table precedence if -1
	prec[Operator_TokenType] = -1;
	prec[IntFuncP_TokenType] = -1;
	prec[DefFuncP_TokenType] = 2;  // same as open parentheses (Paren_TokenType)
	prec[Paren_TokenType] = 2;

	// set token type has a table entry flags
	table[Command_TokenType] = true;
	table[Operator_TokenType] = true;
	table[IntFuncN_TokenType] = true;
	table[IntFuncP_TokenType] = true;
	// FIXME should Remark_TokenType also have table entry flag set?
}


// function to set the default data type of the token
void Token::setDataType(void)
{
	// only set to double if not an internal function
	if (m_dataType == None_DataType && m_type != IntFuncP_TokenType)
	{
		// TODO for now just set default to double
		m_dataType = Double_DataType;
	}
	// change string DefFuncN/P to TmpStr
	else if ((m_type == DefFuncN_TokenType || m_type == DefFuncP_TokenType)
		&& m_dataType == String_DataType)
	{
		m_dataType = TmpStr_DataType;
	}
}


// end: token.cpp
