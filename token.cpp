// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: token.cpp - token class source file
//	Copyright (C) 2012-2013  Thunder422
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
#include "table.h"


// static token variables
bool Token::s_paren[sizeof_TokenType];
bool Token::s_op[sizeof_TokenType];
int Token::s_prec[sizeof_TokenType];
bool Token::s_table[sizeof_TokenType];

// token status message array
//   (TokenStatus enumeration generated from names
//   in comments at the end each line by enums.awk,
//   lines starting with comments are ignored)
const QString Token::s_messageArray[sizeof_TokenStatus] = {
	tr("Null_TokenStatus (BUG)"),							// Null
	tr("Good_TokenStatus (BUG)"),							// Good
	tr("Done_TokenStatus (BUG)"),							// Done
	tr("Parser_TokenStatus (BUG)"),							// Parser
	tr("expected command"),									// ExpCmd
	tr("expected expression"),								// ExpExpr
	tr("expected expression or end-of-statement"),			// ExpExprOrEnd
	tr("expected operator or end-of-statement"),			// ExpOpOrEnd
	tr("expected binary operator or end-of-statement"),		// ExpBinOpOrEnd
	tr("expected equal or comma for assignment"),			// ExpEqualOrComma
	tr("expected comma"),									// ExpComma
	tr("expected item for assignment"),						// ExpAssignItem
	tr("expected operator or comma"),						// ExpOpOrComma
	tr("expected operator, comma or closing parentheses"),	// ExpOpCommaOrParen
	tr("expected operator or end-of-expression"),			// NoOpenParen
	tr("expected operator or closing parentheses"),			// ExpOpOrParen
	tr("expected double expression"),						// ExpDouble
	tr("expected integer expression"),						// ExpInteger
	tr("expected string expression (old)"),					// ExpString
	tr("expected numeric expression"),						// ExpNumExpr
	tr("expected string expression"),						// ExpStrExpr
	tr("expected semicolon, comma or end-of-statement"),	// ExpSemiCommaOrEnd
	tr("expected semicolon or comma"),						// ExpSemiOrComma
	tr("expected operator, semicolon or comma"),			// ExpOpSemiOrComma
	tr("expected double variable"),							// ExpDblVar
	tr("expected integer variable"),						// ExpIntVar
	tr("expected string variable"),							// ExpStrVar
	tr("expected variable"),								// ExpVar
	tr("expected string item for assignment"),				// ExpStrItem
	tr("expected end-of-statement"),						// ExpEndStmt
	// the following statuses used during development
	tr("BUG: not yet implemented"),					// NotYetImplemented
	tr("BUG: invalid mode"),						// InvalidMode
	tr("BUG: hold stack empty"),					// HoldStackEmpty
	tr("BUG: hold stack not empty"),				// HoldStackNotEmpty
	tr("BUG: done stack not empty"),				// DoneStackNotEmpty
	tr("BUG: done stack empty - parentheses"),		// DoneStackEmptyParen
	tr("BUG: done stack empty - operands"),			// DoneStackEmptyOperands
	tr("BUG: done stack empty - operands 2"),		// DoneStackEmptyOperands2
	tr("BUG: done stack empty - find code"),		// DoneStackEmptyFindCode
	tr("BUG: unexpected closing parentheses"),		// UnexpectedCloseParen
	tr("BUG: unexpected token on hold stack"),		// UnexpectedToken
	tr("BUG: expected operand on done stack"),		// DoneStackEmpty
	tr("BUG: command stack not empty"),				// CmdStackNotEmpty
	tr("BUG: command stack empty"),					// CmdStackEmpty
	tr("BUG: command stack empty for expression"),	// CmdStackEmptyExpr
	tr("BUG: command stack empty for command"),		// CmdStackEmptyCmd
	tr("BUG: no assign list code found"),			// NoAssignListCode
	tr("BUG: invalid data type"),					// InvalidDataType
	tr("BUG: count stack empty"),					// CountStackEmpty
	tr("BUG: unexpected parentheses in expression"),	// UnexpParenExpr
	tr("BUG: unexpected token"),					// UnexpToken
	tr("BUG: debug #1"),							// Debug1
	tr("BUG: debug #2"),							// Debug2
	tr("BUG: debug #3"),							// Debug3
	tr("BUG: debug #4"),							// Debug4
	tr("BUG: debug #5"),							// Debug5
	tr("BUG: debug #6"),							// Debug6
	tr("BUG: debug #7"),							// Debug7
	tr("BUG: debug #8"),							// Debug8
	tr("BUG: debug #9"),							// Debug9
	tr("BUG: debug")								// Debug
};


// function to initialize the static token data
void Token::initialize(void)
{
	// set true for types that contain an opening parentheses
	s_paren[IntFuncP_TokenType] = true;
	s_paren[DefFuncP_TokenType] = true;
	s_paren[Paren_TokenType] = true;

	// set true for types that are considered an operator
	s_op[Command_TokenType] = true;
	s_op[Operator_TokenType] = true;

	// set precedence for non-table token types
	s_prec[Command_TokenType] = -1;  // use table precedence if -1
	s_prec[Operator_TokenType] = -1;
	s_prec[IntFuncP_TokenType] = -1;
	s_prec[DefFuncP_TokenType] = 2;  // same as Paren_TokenType
	s_prec[Paren_TokenType] = 2;

	// set token type has a table entry flags
	s_table[Command_TokenType] = true;
	s_table[Operator_TokenType] = true;
	s_table[IntFuncN_TokenType] = true;
	s_table[IntFuncP_TokenType] = true;
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
}


// function to recreate text (abbreviated contents) of token
QString Token::text(void)
{
	Table &table = Table::instance();
	QString string;

	switch (m_type)
	{
	case DefFuncN_TokenType:
	case NoParen_TokenType:
		string = m_string;
		break;

	case DefFuncP_TokenType:
	case Paren_TokenType:
		string = m_string + '(';
		break;

	case Constant_TokenType:
		switch (m_dataType)
		{
		case Integer_DataType:
		case Double_DataType:
			string = m_string;
			break;

		case String_DataType:
			string = '"' + m_string + '"';
			break;

		}
		break;

	case Operator_TokenType:
		if (isCode(RemOp_Code))
		{
			string = table.name(m_code) + '|' + m_string + '|';
		}
		else
		{
			string = table.debugName(m_code);
		}
		break;

	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		string = table.debugName(m_code);
		break;

	case Command_TokenType:
		if (isCode(Rem_Code))
		{
			string = table.name(m_code) + '|' + m_string + '|';
		}
		else
		{
			string = table.name(m_code);
			if (table.name2(m_code) != NULL)
			{
				string += '-' + table.name2(m_code);
			}
		}
		break;

	default:
		// nothing more to output
		break;
	}
	if (m_reference)
	{
		string += "<ref>";
	}
	if (isSubCode(~Used_SubCode))
	{
		string += '\'';
		if (isSubCode(Paren_SubCode))
		{
			string += ')';
		}
		if (isSubCode(Let_SubCode))
		{
			string += "LET";
		}
		if (isSubCode(SemiColon_SubCode))
		{
			string += ';';
		}
		if (isSubCode(Keep_SubCode))
		{
			string += "Keep";
		}
		if (isSubCode(End_SubCode))
		{
			string += "End";
		}
		if (isSubCode(Question_SubCode))
		{
			string += "Question";
		}
		string += '\'';
	}
	return string;
}


// function to overload the comparison operator
bool Token::operator==(const Token &other) const
{
	if (m_type != other.m_type)
	{
		return false;
	}
	switch (m_type)
	{
	case Constant_TokenType:
		if (m_dataType != other.m_dataType)
		{
			return false;
		}
		// now fall thru to compare string of constant
	case DefFuncN_TokenType:
	case NoParen_TokenType:
	case DefFuncP_TokenType:
	case Paren_TokenType:
		if (m_string != other.m_string)
		{
			return false;
		}
		break;

	case Operator_TokenType:
		if (isCode(RemOp_Code))
		{
			if (m_string != other.m_string)
			{
				return false;
			}
		}
		else
		{
			if (m_code != other.m_code)
			{
				return false;
			}
		}
		break;

	case Command_TokenType:
		if (isCode(Rem_Code))
		{
			if (m_string != other.m_string)
			{
				return false;
			}
		}
		else
		{
			if (m_code != other.m_code)
			{
				return false;
			}
		}
		break;

	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		if (m_code != other.m_code)
		{
			return false;
		}
		break;

	default:
		// nothing to compare
		break;
	}
	if (m_reference != other.m_reference || m_subCode != other.m_subCode)
	{
		return false;
	}
	return true;
}

// end: token.cpp
