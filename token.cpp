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

#include "table.h"
#include "token.h"
#include "utility.h"


// constructor for codes
Token::Token(Code code) : m_column{-1}, m_length{-1}, m_string{}, m_reference{},
	m_subCode{}
{
	Table::instance().setToken(this, code);
}

Token::Token(TableEntry *entry, int column, int length,
	const std::string string) : m_column{column}, m_length{length},
	m_string{string}, m_reference{}, m_subCode{}
{
	Table::instance().setToken(this, entry->code());
}


// constructor for codes with operands
Token::Token(TableEntry *entry, DataType dataType, int column, int length,
	const std::string string, bool reference, SubCode subCode) :
	m_column{column}, m_length{length}, m_string{string},
	m_reference{reference}, m_subCode{subCode}
{
	Table::instance().setTokenCode(this, entry->code(), dataType);
}


// constructor for integer constants
Token::Token(DataType dataType, int column, int length,
	const std::string string, int value) : m_column{column}, m_length{length},
	m_string{string}, m_reference{}, m_subCode{}, m_value(value), /*convert*/
	m_valueInt{value}
{
	Table::instance().setTokenCode(this, Const_Code,
		dataType == DataType::Double ? dataType : DataType::Integer);
}


// constructor to set double constants
Token::Token(DataType dataType, int column, int length,
	const std::string string, double value) : m_column{column},
	m_length{length}, m_string{string}, m_reference{}, m_subCode{},
	m_value{value}
{
	if (value > std::numeric_limits<int>::min() - 0.5
		&& value < std::numeric_limits<int>::max() + 0.5)
	{
		m_valueInt = value;  // convert to integer in case needed
		if (dataType != DataType::Double && dataType != DataType::Integer)
		{
			m_subCode = IntConst_SubCode;
			dataType = DataType::Double;
		}
	}
	else  // number can't be converted to integer
	{
		dataType = DataType::Double;
	}
	Table::instance().setTokenCode(this, Const_Code, dataType);
}


// constructor for string constants
Token::Token(int column, int length, const std::string string) :
	m_column{column}, m_length{length}, m_dataType{DataType::String},
	m_string{string}, m_code{Invalid_Code}, m_reference{}, m_subCode{}
{
	Table::instance().setTokenCode(this, Const_Code);
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


// function to change constant token to desired data type
//   - resets integer constant sub-code for numeric desired data types
//   - does nothing if constant is not changed
bool Token::convertConstant(DataType dataType)
{
	if (!isType(Type::Constant))
	{
		return false;
	}
	if (dataType == DataType::Double)
	{
		if (m_dataType == DataType::Double)
		{
			removeSubCode(IntConst_SubCode);
			return true;
		}
		if (m_dataType != DataType::Integer)
		{
			return false;
		}
	}
	else if (dataType == DataType::Integer && m_dataType == DataType::Double
		&& hasSubCode(IntConst_SubCode))
	{
		removeSubCode(IntConst_SubCode);
	}
	else
	{
		return false;
	}
	m_dataType = dataType;
	Table::instance().setTokenCode(this, Const_Code);
	return true;
}


// function to get convert code needed to convert token to data type
//   - changes number constant tokens to desired data type
//   - returns null code if no conversion needed
//   - returns invalid code if cannot be converted
//   - if cannot be converted then sets data type for error reporting
Code Token::convertCode(DataType dataType)
{
	if (convertConstant(dataType))
	{
		return Null_Code;
	}
	switch (dataType)
	{
	case DataType::Double:
		if (m_dataType == DataType::Integer)
		{
			return CvtDbl_Code;
		}
		else if (m_dataType == DataType::Double)
		{
			return Null_Code;
		}
		break;

	case DataType::Integer:
		if (m_dataType == DataType::Double)
		{
			return isType(Type::Constant) ? Invalid_Code : CvtInt_Code;
		}
		else if (m_dataType == DataType::Integer)
		{
			return Null_Code;
		}
		break;

	case DataType::String:
		if (m_dataType == DataType::String)
		{
			return Null_Code;
		}
		break;

	case DataType::Number:
		if (m_dataType != DataType::String)
		{
			return Null_Code;
		}
		break;

	case DataType::None:
	case DataType::Any:
		return Null_Code;
	}
	m_dataType = dataType;
	return Invalid_Code;
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
	if (m_code == Rem_Code || m_code == RemOp_Code
		|| (isType(Type::Constant) && isDataType(DataType::String)))
	{
		return m_string == other.m_string;
	}
	else
	{
		return noCaseStringEqual(m_string, other.m_string);
	}
}


// end: token.cpp
