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
		if (table.hasFlag(m_code, Reference_Flag))
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
			if (!table.name2(m_code).isNull())
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
	if (hasSubCode())
	{
		ss << '\'';
		if (hasSubCode(Paren_SubCode))
		{
			ss << ')';
		}
		if (hasSubCode(Option_SubCode))
		{
			std::string option {table.optionName(m_code).toStdString()};
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


// end: token.cpp
