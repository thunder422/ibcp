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
int Token::s_prec[sizeof_TokenType];

// token status message array
//   (TokenStatus enumeration generated from names in comments
//   on the line before the line with the message string by enums.awk,
//   extra lines starting with comments are ignored)
const QString Token::s_messageArray[sizeof_TokenStatus] = {
	// Null
	tr("Null_TokenStatus (BUG)"),
	// Good
	tr("Good_TokenStatus (BUG)"),
	// Done
	tr("Done_TokenStatus (BUG)"),
	// Parser
	tr("Parser_TokenStatus (BUG)"),
	// ExpCmd
	tr("expected command"),
	// ExpExpr
	tr("expected expression"),
	// ExpExprOrEnd
	tr("expected expression or end-of-statement"),
	// ExpOpOrEnd
	tr("expected operator or end-of-statement"),
	// ExpBinOpOrEnd
	tr("expected binary operator or end-of-statement"),
	// ExpEqualOrComma
	tr("expected equal or comma for assignment"),
	// ExpComma
	tr("expected comma"),
	// ExpAssignItem
	tr("expected item for assignment"),
	// ExpOpOrComma
	tr("expected operator or comma"),
	// ExpOpCommaOrParen
	tr("expected operator, comma or closing parentheses"),
	// ExpOpOrParen
	tr("expected operator or closing parentheses"),
	// ExpBinOpOrComma
	tr("expected binary operator or comma"),
	// ExpBinOpCommaOrParen
	tr("expected binary operator, comma or closing parentheses"),
	// ExpBinOpOrParen
	tr("expected binary operator or closing parentheses"),
	// ExpDouble
	tr("expected double expression"),
	// ExpInteger
	tr("expected integer expression"),
	// ExpString
	tr("expected string expression (old)"),
	// ExpNumExpr
	tr("expected numeric expression"),
	// ExpStrExpr
	tr("expected string expression"),
	// ExpSemiCommaOrEnd
	tr("expected semicolon, comma or end-of-statement"),
	// ExpCommaSemiOrEnd
	tr("expected comma, semicolon or end-of-statement"),
	// ExpSemiOrComma
	tr("expected semicolon or comma"),
	// ExpOpSemiOrComma
	tr("expected operator, semicolon or comma"),
	// ExpBinOpSemiOrComma
	tr("expected binary operator, semicolon or comma"),
	// ExpDblVar
	tr("expected double variable"),
	// ExpIntVar
	tr("expected integer variable"),
	// ExpStrVar
	tr("expected string variable"),
	// ExpVar
	tr("expected variable"),
	// ExpStrItem
	tr("expected string item for assignment"),
	// ExpEndStmt
	tr("expected end-of-statement"),
	// ExpExprPfnOrEnd
	tr("expected expression, print function or end-of-statement"),
	// ExpExprCommaPfnOrEnd
	tr("expected expression, comma, print function or end-of-statement"),
	// ExpOpSemiCommaOrEnd
	tr("expected operator, semicolon, comma or end-of-statement"),
	// ExpIntConst
	tr("expected valid integer constant"),
	// the following statuses used during development
	// NotYetImplemented
	tr("BUG: not yet implemented"),
	// InvalidMode
	tr("BUG: invalid mode"),
	// HoldStackEmpty
	tr("BUG: hold stack empty"),
	// HoldStackNotEmpty
	tr("BUG: hold stack not empty"),
	// DoneStackNotEmpty
	tr("BUG: done stack not empty"),
	// DoneStackEmptyParen
	tr("BUG: done stack empty - parentheses"),
	// DoneStackEmptyOperands
	tr("BUG: done stack empty - operands"),
	// DoneStackEmptyOperands2
	tr("BUG: done stack empty - operands 2"),
	// DoneStackEmptyFindCode
	tr("BUG: done stack empty - find code"),
	// UnexpectedCloseParen
	tr("BUG: unexpected closing parentheses"),
	// UnexpectedToken
	tr("BUG: unexpected token on hold stack"),
	// DoneStackEmpty
	tr("BUG: expected operand on done stack"),
	// CmdStackNotEmpty
	tr("BUG: command stack not empty"),
	// CmdStackEmpty
	tr("BUG: command stack empty"),
	// CmdStackEmptyExpr
	tr("BUG: command stack empty for command"),
	// CmdStackEmptyCmd
	tr("BUG: command stack empty for expression"),
	// NoAssignListCode
	tr("BUG: no assign list code found"),
	// InvalidDataType
	tr("BUG: invalid data type"),
	// CountStackEmpty
	tr("BUG: count stack empty"),
	// UnexpParenExpr
	tr("BUG: unexpected parentheses in expression"),
	// UnexpToken
	tr("BUG: unexpected token"),
	// Debug1
	tr("BUG: debug #1"),
	// Debug2
	tr("BUG: debug #2"),
	// Debug3
	tr("BUG: debug #3"),
	// Debug4
	tr("BUG: debug #4"),
	// Debug5
	tr("BUG: debug #5"),
	// Debug6
	tr("BUG: debug #6"),
	// Debug7
	tr("BUG: debug #7"),
	// Debug8
	tr("BUG: debug #8"),
	// Debug9
	tr("BUG: debug #9"),
	// Debug
	tr("BUG: debug")
};

Token::FreeStack Token::s_freeStack;	// stack of free tokens
Token::UsedVector Token::s_used;		// vector of tokens currently in use
Token::DeletedList Token::s_deleted;	// list of tokens deleted extra times


// destructor function for the token free stack
//
//   - called automatically at the end of the application
//   - deletes memory used by the token on the free stack

Token::FreeStack::~FreeStack(void)
{
	// delete any tokens left in the free stack
	while (!isEmpty())
	{
		::operator delete(s_freeStack.pop());
	}
}


// destructor function for the used token vector
//
//   - called automatically at the end of the application
//   - to report token leaks before application terminates

Token::UsedVector::~UsedVector(void)
{
	reportErrors();
}


// function to report token leak errors and delete them
//
//   - reports any tokens marked as used and deletes them
//   - to report token leaks at any time (for testing)

void Token::UsedVector::reportErrors(void)
{
	bool first = false;
	for (int i = 0; i < s_used.size(); i++)
	{
		if (s_used[i] != NULL)
		{
			if (!first)
			{
				qCritical("Token Leaks:");
				first = true;
			}
			qCritical("  %d: %s", i, qPrintable(s_used[i]->text()));
			::operator delete(s_used[i]);
			s_used[i] = NULL;
		}
	}
}


// destructor function for the list of extra token deletes
//
//   - called automatically at the end of the application
//   - to report extra token deletes before application terminates

Token::DeletedList::~DeletedList(void)
{
	reportErrors();
}


// function to report extra token deletes errors
//
//   - outputs strings contained in the deleted list
//   - to report extra token deletes at any time (for testing)

void Token::DeletedList::reportErrors(void)
{
	if (!s_deleted.isEmpty())
	{
		qCritical("Token Extra Deletes:");
		for (int i = 0; i < s_deleted.size(); i++)
		{
			qCritical("  %s", qPrintable(s_deleted[i]));
		}
		s_deleted.clear();
	}
}


// static function to initialize the static token data
void Token::initialize(void)
{
	// set true for types that contain an opening parentheses
	s_paren[IntFuncP_TokenType] = true;
	s_paren[DefFuncP_TokenType] = true;
	s_paren[Paren_TokenType] = true;

	// set precedence for non-table token types
	s_prec[Command_TokenType] = -1;  // use table precedence if -1
	s_prec[Operator_TokenType] = -1;
	s_prec[IntFuncN_TokenType] = -1;
	s_prec[IntFuncP_TokenType] = -1;
	// these tokens need to be lowest precedence but above Null_Code
	s_prec[Constant_TokenType] = 2;
	s_prec[DefFuncN_TokenType] = 2;
	s_prec[DefFuncP_TokenType] = 2;
	s_prec[NoParen_TokenType] = 2;
	s_prec[Paren_TokenType] = 2;
}


// function to set the default data type of the token
void Token::setDataType(void)
{
	// only set to double if not an internal function
	if (m_dataType == DataType::None && m_type != IntFuncP_TokenType)
	{
		// TODO for now just set default to double
		m_dataType = DataType::Double;
	}
}


// function to recreate text (abbreviated contents) of token
QString Token::text(bool withIndex)
{
	Table &table = Table::instance();
	QString string;
	QString second;

	if (withIndex)
	{
		string = QString("%1:").arg(m_index);
	}
	switch (m_type)
	{
	case DefFuncN_TokenType:
		string += m_string;
		break;

	case NoParen_TokenType:
		if (withIndex)
		{
			string += table.debugName(m_code);
			second = textOperand(withIndex);
		}
		else
		{
			if (m_code == Invalid_Code)
			{
				string += '?';
			}
			string += m_string;
			if (table.hasFlag(this, Reference_Flag))
			{
				string += "<ref>";
			}
		}
		break;

	case DefFuncP_TokenType:
	case Paren_TokenType:
		string += m_string + '(';
		break;

	case Constant_TokenType:
		if (withIndex)
		{
			string += table.debugName(m_code);
			second = textOperand(withIndex);
		}
		else
		{
			if (m_code == Invalid_Code)
			{
				string += '?';
			}
			switch (m_dataType)
			{
			case DataType::Integer:
			case DataType::Double:
				string += m_string;
				if (m_dataType == DataType::Integer)
				{
					string += "%";
				}
				break;

			case DataType::String:
				string += '"' + m_string + '"';
				break;
            default:
                break;
			}
		}
		break;

	case Operator_TokenType:
		if (isCode(RemOp_Code))
		{
			string += table.name(m_code);
			second = textOperand(withIndex);
		}
		else
		{
			string += table.debugName(m_code);
		}
		break;

	case IntFuncN_TokenType:
	case IntFuncP_TokenType:
		string += table.debugName(m_code);
		if (withIndex && table.hasOperand(m_code))
		{
			second += textOperand(withIndex);
		}
		break;

	case Command_TokenType:
		if (isCode(Rem_Code))
		{
			string += table.name(m_code);
			second = textOperand(withIndex);
		}
		else
		{
			string += table.name(m_code);
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
	if (hasSubCode(~(Used_SubCode | UnUsed_SubCode)))
	{
		string += '\'';
		if (hasSubCode(Paren_SubCode))
		{
			string += ')';
		}
		if (hasSubCode(Option_SubCode))
		{
			QString option = table.optionName(m_code);
			string += option.isEmpty() ? "BUG" : option;
		}
		if (hasSubCode(Colon_SubCode))
		{
			string += ":";
		}
		if (hasSubCode(Double_SubCode))
		{
			string += "Double";
		}
		string += '\'';
	}
	return string + second;
}


// function to get text of operand (with index if selected)
QString Token::textOperand(bool withIndex)
{
	QString string;
	if (withIndex)
	{
		string = QString(" %1:").arg(m_index + 1);
	}
	return string + '|' + m_string + '|';
}


// function to overload the comparison operator
bool Token::operator==(const Token &other) const
{
	if (m_code != other.m_code)
	{
		return false;
	}
	if ((m_subCode & ProgramMask_SubCode)
		!= (other.m_subCode & ProgramMask_SubCode))
	{
		return false;
	}
	if (m_code == Rem_Code || m_code == RemOp_Code || m_code == ConstStr_Code)
	{
		return m_string.compare(other.m_string, Qt::CaseSensitive) == 0;
	}
	else
	{
		return m_string.compare(other.m_string, Qt::CaseInsensitive) == 0;
	}
}


// function to overload the default new operator
//
//   - if available tokens on free stack then pops one and returns it
//   - otherwise new memory is allocated
//   - token pointer is added to the used vector

void *Token::operator new(size_t size)
{
	Token *token;
	if (s_freeStack.isEmpty())
	{
		// allocate the memory for the token
		token = (Token *)::operator new(size);

		// set index into used vector and add to vector
		token->m_id = s_used.size();
		s_used.append(token);
	}
	else  // get a token from the free stack
	{
		token = s_freeStack.pop();

		// mark token as used
		s_used[token->m_id] = token;
	}

	// return pointer to new token
	return token;
}


// function to overload the default delete operator
//
//   - ignores null pointer values
//   - pushes token to free stack, does not delete the token
//   - if token was already deleted then adds text of token to deleted list
//   - otherwise token pointer is removed from the used vector

void Token::operator delete(void *ptr)
{
	if (ptr != NULL)
	{
		Token *token = (Token *)ptr;

		if (s_used[token->m_id] == NULL)  // already deleted?
		{
			s_deleted.append(QString("%1: %2").arg(token->m_id)
				.arg(token->text()));
		}
		else  // mark token as unused and cache on free stack
		{
			s_used[token->m_id] = NULL;
			s_freeStack.push(token);
		}
	}
}


// end: token.cpp
