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

#include <sstream>

#include "ibcp.h"
#include "token.h"
#include "table.h"


// static token variables
std::unordered_map<Token::Type, bool, EnumClassHash> Token::s_hasParen {
	{Type::IntFuncP, true},
	{Type::DefFuncP, true},
	{Type::Paren, true}
};
// set precedence for non-table token types
std::unordered_map<Token::Type, int, EnumClassHash> Token::s_precendence {
	{Type::Command, -1},  // use table precedence if -1
	{Type::Operator, -1},
	{Type::IntFuncN, -1},
	{Type::IntFuncP, -1},
	// these tokens need to be lowest precedence but above Null_Code
	{Type::Constant, 2},
	{Type::DefFuncN, 2},
	{Type::DefFuncP, 2},
	{Type::NoParen, 2},
	{Type::Paren, 2}
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
			qCritical("  %d: %s", i, s_used[i]->text().c_str());
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


// function to set the default data type of the token
void Token::setDataType(void)
{
	// only set to double if not an internal function
	if (m_dataType == DataType::None && m_type != Type::IntFuncP)
	{
		// TODO for now just set default to double
		m_dataType = DataType::Double;
	}
}


// function to get convert code needed to convert token to data type
Code Token::convertCode(DataType toDataType) const
{
	switch (dataType())
	{
	case DataType::Double:
		switch (toDataType)
		{
		case DataType::Integer:
			return CvtInt_Code;		// convert Double to Integer
		case DataType::String:
			return Invalid_Code;	// can't convert Double to String
		default:
			return Null_Code;		// no conversion needed
		}
	case DataType::Integer:
		switch (toDataType)
		{
		case DataType::Double:
			return CvtDbl_Code;		// convert Integer to Double
		case DataType::String:
			return Invalid_Code;	// can't convert Integer to String
		default:
			return Null_Code;		// no conversion needed
		}
	case DataType::String:
		switch (toDataType)
		{
		case DataType::String:
		case DataType::None:
		case DataType::Any:
			return Null_Code;		// print function allowed if needed None
		default:
			return Invalid_Code;	// conversion from string no allowed
		}
	case DataType::None:
		// print function allowed if needed none,
		// else conversion from none not allowed
		return toDataType == DataType::None ? Null_Code : Invalid_Code;

	default:
		// Number, Any (will not have any of this data type)
		return Invalid_Code;
	}
}


// function to recreate text (abbreviated contents) of token
std::string Token::text()
{
	Table &table = Table::instance();
	std::stringstream ss;
	bool second {};

	switch (m_type)
	{
	case Type::DefFuncN:
		ss << m_string.toStdString();
		break;

	case Type::NoParen:
		if (m_code == Invalid_Code)
		{
			ss << '?';
		}
		ss << m_string.toStdString();
		if (table.hasFlag(this, Reference_Flag))
		{
			ss << "<ref>";
		}
		break;

	case Type::DefFuncP:
	case Type::Paren:
		ss << m_string.toStdString() << '(';
		break;

	case Type::Constant:
		if (m_code == Invalid_Code)
		{
			ss << '?';
		}
		switch (m_dataType)
		{
		case DataType::Integer:
		case DataType::Double:
			ss << m_string.toStdString();
			if (m_dataType == DataType::Integer)
			{
				ss << "%";
			}
			break;

		case DataType::String:
			ss << '"' << m_string.toStdString() << '"';
			break;
		default:
			break;
		}
		break;

	case Type::Operator:
		if (isCode(RemOp_Code))
		{
			ss << table.name(m_code).toStdString();
			second = true;
		}
		else
		{
			ss << table.debugName(m_code).toStdString();
		}
		break;

	case Type::IntFuncN:
	case Type::IntFuncP:
		ss << table.debugName(m_code).toStdString();
		break;

	case Type::Command:
		if (isCode(Rem_Code))
		{
			ss << table.name(m_code).toStdString();
			second = true;
		}
		else
		{
			ss << table.name(m_code).toStdString();
			if (table.name2(m_code) != NULL)
			{
				ss << '-' << table.name2(m_code).toStdString();
			}
		}
		break;

	default:
		// nothing more to output
		break;
	}
	if (m_reference)
	{
		ss << "<ref>";
	}
	if (hasSubCode(~(Used_SubCode | UnUsed_SubCode)))
	{
		ss << '\'';
		if (hasSubCode(Paren_SubCode))
		{
			ss << ')';
		}
		if (hasSubCode(Option_SubCode))
		{
			std::string option = table.optionName(m_code).toStdString();
			if (option.empty())
			{
				ss << "BUG";
			}
			else
			{
				ss << option;
			}
		}
		if (hasSubCode(Colon_SubCode))
		{
			ss << ':';
		}
		if (hasSubCode(Double_SubCode))
		{
			ss << "Double";
		}
		ss << '\'';
	}
	if (second)
	{
		ss << '|' << m_string.toStdString() << '|';
	}
	return ss.str();
}


// function to convert token status enumerator to translated string
const QString Token::message(Status status)
{
	switch (status)
	{
	case Status::Good:
		return tr("Good_TokenStatus (BUG)");
	case Status::Done:
		return tr("Done_TokenStatus (BUG)");
	case Status::Parser:
		return tr("Parser_TokenStatus (BUG)");
	case Status::ExpCmd:
		return tr("expected command");
	case Status::ExpExpr:
		return tr("expected expression");
	case Status::ExpExprOrEnd:
		return tr("expected expression or end-of-statement");
	case Status::ExpOpOrEnd:
		return tr("expected operator or end-of-statement");
	case Status::ExpBinOpOrEnd:
		return tr("expected binary operator or end-of-statement");
	case Status::ExpEqualOrComma:
		return tr("expected equal or comma for assignment");
	case Status::ExpComma:
		return tr("expected comma");
	case Status::ExpAssignItem:
		return tr("expected item for assignment");
	case Status::ExpOpOrComma:
		return tr("expected operator or comma");
	case Status::ExpOpCommaOrParen:
		return tr("expected operator, comma or closing parentheses");
	case Status::ExpOpOrParen:
		return tr("expected operator or closing parentheses");
	case Status::ExpBinOpOrComma:
		return tr("expected binary operator or comma");
	case Status::ExpBinOpCommaOrParen:
		return tr("expected binary operator, comma or closing parentheses");
	case Status::ExpBinOpOrParen:
		return tr("expected binary operator or closing parentheses");
	case Status::ExpNumExpr:
		return tr("expected numeric expression");
	case Status::ExpStrExpr:
		return tr("expected string expression");
	case Status::ExpSemiCommaOrEnd:
		return tr("expected semicolon, comma or end-of-statement");
	case Status::ExpCommaSemiOrEnd:
		return tr("expected comma, semicolon or end-of-statement");
	case Status::ExpSemiOrComma:
		return tr("expected semicolon or comma");
	case Status::ExpOpSemiOrComma:
		return tr("expected operator, semicolon or comma");
	case Status::ExpDblVar:
		return tr("expected double variable");
	case Status::ExpIntVar:
		return tr("expected integer variable");
	case Status::ExpStrVar:
		return tr("expected string variable");
	case Status::ExpVar:
		return tr("expected variable");
	case Status::ExpStrItem:
		return tr("expected string item for assignment");
	case Status::ExpEndStmt:
		return tr("expected end-of-statement");
	case Status::ExpExprPfnOrEnd:
		return tr("expected expression, print function or end-of-statement");
	case Status::ExpExprCommaPfnOrEnd:
		return tr("expected expression, comma, print function or "
			"end-of-statement");
	case Status::ExpOpSemiCommaOrEnd:
		return tr("expected operator, semicolon, comma or end-of-statement");
	case Status::ExpIntConst:
		return tr("expected valid integer constant");
	// the following statuses used during development
	case Status::BUG_NotYetImplemented:
		return tr("BUG: not yet implemented");
	case Status::BUG_HoldStackNotEmpty:
		return tr("BUG: hold stack not empty");
	case Status::BUG_DoneStackNotEmpty:
		return tr("BUG: done stack not empty");
	case Status::BUG_DoneStackEmptyFindCode:
		return tr("BUG: done stack empty - find code");
	case Status::BUG_UnexpectedCloseParen:
		return tr("BUG: unexpected closing parentheses");
	case Status::BUG_DoneStackEmpty:
		return tr("BUG: expected operand on done stack");
	case Status::BUG_InvalidDataType:
		return tr("BUG: invalid data type");
	case Status::BUG_UnexpToken:
		return tr("BUG: unexpected token");
	case Status::BUG_Debug1:
		return tr("BUG: debug #1");
	case Status::BUG_Debug2:
		return tr("BUG: debug #2");
	case Status::BUG_Debug3:
		return tr("BUG: debug #3");
	case Status::BUG_Debug4:
		return tr("BUG: debug #4");
	case Status::BUG_Debug5:
		return tr("BUG: debug #5");
	case Status::BUG_Debug6:
		return tr("BUG: debug #6");
	case Status::BUG_Debug7:
		return tr("BUG: debug #7");
	case Status::BUG_Debug8:
		return tr("BUG: debug #8");
	case Status::BUG_Debug9:
		return tr("BUG: debug #9");
	case Status::BUG_Debug:
		return tr("BUG: debug");
	}
	return "";
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
	TokenPtr token;
	if (s_freeStack.isEmpty())
	{
		// allocate the memory for the token
		token = (TokenPtr)::operator new(size);

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
		TokenPtr token = (TokenPtr)ptr;

		if (s_used[token->m_id] == NULL)  // already deleted?
		{
			s_deleted.append(QString("%1: %2").arg(token->m_id)
				.arg(QString::fromStdString(token->text())));
		}
		else  // mark token as unused and cache on free stack
		{
			s_used[token->m_id] = NULL;
			s_freeStack.push(token);
		}
	}
}


// end: token.cpp
