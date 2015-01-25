// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: token.cpp - token class source file
//	Copyright (C) 2012-2015  Thunder422
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
	m_entry = Table::entry(Code::Constant, dataType);
}


// function to append data type if applicable
std::string Token::stringWithDataType() const
{
	std::string string = m_string;
	if (!isCode(Code::Constant))
	{
		switch (dataType())
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


Table *Token::convert(TokenPtr &operandToken, int operand)
{
	DataType expectedDataType {m_entry->operandDataType(operand)};

	if (operandToken->dataType() == expectedDataType)
	{
		operandToken->removeSubCode(IntConst_SubCode);  // safe for any token
		return {};
	}

	if (isLastOperand(operand) && !hasFlag(UseConstAsIs_Flag))
	{
		operandToken->changeConstantIgnoreError(expectedDataType);
	}

	if (Table *entry = m_entry->alternateForOperand(operand,
		operandToken->dataType()))
	{
		setTableEntry(entry);
		return {};
	}

	return operandToken->convertCodeEntry(expectedDataType);
}


Table *Token::convertCodeEntry(DataType toDataType)
{
	if (!changeConstant(toDataType) && m_entry->returnDataType() != toDataType)
	{
		switch (toDataType)
		{
		case DataType::Double:
			if (m_entry->returnDataType() != DataType::Integer)
			{
				throw Status::ExpNumExpr;
			}
			return Table::entry(Code::CvtDbl);

		case DataType::Integer:
			if (m_entry->returnDataType() != DataType::Double)
			{
				throw Status::ExpNumExpr;
			}
			return Table::entry(Code::CvtInt);

		case DataType::String:
			throw Status::ExpStrExpr;

		case DataType::Number:
			if (m_entry->returnDataType() == DataType::String)
			{
				throw Status::ExpNumExpr;
			}
			break;

		case DataType::None:
		case DataType::Any:
			break;
		}
	}
	return {};
}


void Token::changeConstantIgnoreError(DataType toDataType) noexcept
{
	try
	{
		changeConstant(toDataType);
	}
	catch (Status) {}  // ignore unconvertible double to integer error
}


bool Token::changeConstant(DataType toDataType)
{
	if (!isCode(Code::Constant))
	{
		return false;
	}
	if (toDataType == DataType::Double)
	{
		if (m_entry->returnDataType() == DataType::Double)
		{
			removeSubCode(IntConst_SubCode);
			return true;
		}
		if (m_entry->returnDataType() != DataType::Integer)
		{
			return false;
		}
		// fall to below
	}
	else if (toDataType == DataType::Integer)
	{
		if (m_entry->returnDataType() == DataType::Double)
		{
			if (!hasSubCode(IntConst_SubCode))
			{
				throw Status::ExpIntConst;
			}
			removeSubCode(IntConst_SubCode);
			// fall to below
		}
		else
		{
			return m_entry->returnDataType() == DataType::Integer;
		}
	}
	else
	{
		return false;  // can't convert to number or any
	}
	m_entry = Table::entry(Code::Constant, toDataType);
	return true;
}


// function to overload the comparison operator
bool Token::operator==(const Token &other) const
{
	if (m_entry != other.m_entry)
	{
		return false;
	}
	if ((m_subCode & ProgramMask_SubCode)
		!= (other.m_subCode & ProgramMask_SubCode))
	{
		return false;
	}
	if (isCode(Code::Rem) || isCode(Code::RemOp)
		|| (isCode(Code::Constant) && isDataType(DataType::String)))
	{
		return m_string == other.m_string;
	}
	else
	{
		return noCaseStringEqual(m_string, other.m_string);
	}
}


// end: token.cpp
