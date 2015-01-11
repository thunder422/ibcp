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
	m_code = Table::entry(Const_Code, dataType)->code();
}


// function to append data type if applicable
std::string Token::stringWithDataType() const
{
	std::string string = m_string;
	if (!isType(Type::Constant))
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


TableEntry *Token::convert(TokenPtr &operandToken, int operandIndex)
{
	DataType expectedDataType {table()->operandDataType(operandIndex)};

	if (operandToken->dataType() == expectedDataType)
	{
		operandToken->removeSubCode(IntConst_SubCode);  // safe for any token
		return {};
	}

	if (isLastOperand(operandIndex) && !hasFlag(UseConstAsIs_Flag))
	{
		operandToken->changeConstantIgnoreError(expectedDataType);
	}

	if (TableEntry *entry = table()->alternate(operandIndex,
		operandToken->dataType()))
	{
		setCode(entry->code());
		return {};
	}

	return operandToken->convertCodeEntry(expectedDataType);
}


TableEntry *Token::convertCodeEntry(DataType toDataType)
{
	if (!changeConstant(toDataType) && table()->returnDataType() != toDataType)
	{
		switch (toDataType)
		{
		case DataType::Double:
			if (table()->returnDataType() != DataType::Integer)
			{
				throw Status::ExpNumExpr;
			}
			return Table::entry(CvtDbl_Code);

		case DataType::Integer:
			if (table()->returnDataType() != DataType::Double)
			{
				throw Status::ExpNumExpr;
			}
			return Table::entry(CvtInt_Code);

		case DataType::String:
			throw Status::ExpStrExpr;

		case DataType::Number:
			if (table()->returnDataType() == DataType::String)
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
	if (!isType(Type::Constant))
	{
		return false;
	}
	if (toDataType == DataType::Double)
	{
		if (table()->returnDataType() == DataType::Double)
		{
			removeSubCode(IntConst_SubCode);
			return true;
		}
		if (table()->returnDataType() != DataType::Integer)
		{
			return false;
		}
		// fall to below
	}
	else if (toDataType == DataType::Integer)
	{
		if (table()->returnDataType() == DataType::Double)
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
			return table()->returnDataType() == DataType::Integer;
		}
	}
	else
	{
		return false;  // can't convert to number or any
	}
	m_code = Table::entry(Const_Code, toDataType)->code();
	return true;
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
