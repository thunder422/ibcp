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

#include <limits>

#include "token.h"
#include "utility.h"


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


// constructor to set double constants
Token::Token(int column, int length, const std::string string, double value,
	bool decimal) : m_column{column}, m_length{length},
	m_type{Token::Type::Constant}, m_string{string}, m_code{Invalid_Code},
	m_reference{}, m_value{value}
{
	if (value > std::numeric_limits<int>::min() - 0.5
		&& value < std::numeric_limits<int>::max() + 0.5)
	{
		m_dataType = DataType::Integer;
		// 'double' if decimal pointer present
		m_subCode = decimal ? Double_SubCode : SubCode{};
		m_valueInt = value;  // convert to integer in case needed
	}
	else  // number can't be converted to integer
	{
		m_dataType = DataType::Double;
		m_subCode = {};  // ignore sub-code argument
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


// function to append data type if applicable
std::string Token::stringWithDataType() const
{
	std::string string = m_string;
	if (!isType(Type::Constant))
	{
		switch (m_dataType)
		{
		case DataType::Double:
			if (hasSubCode(Double_SubCode))
			{
				string.push_back('#');  // only if originally present
			}
			break;

		case DataType::Integer:
			string.push_back('%');
			break;

		case DataType::String:
			string.push_back('$');
			break;

		default:
			break;
		}
	}
	return string;
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
		return m_string == other.m_string;
	}
	else
	{
		return noCaseStringEqual(m_string, other.m_string);
	}
}


// end: token.cpp
