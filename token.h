// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: token.h - token class header file
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
//	2012-11-02	initial version (parts removed from ibcp.h)

#ifndef TOKEN_H
#define TOKEN_H

#include <memory>
#include <string>

#include "ibcp.h"
#include "table.h"


class Token
{
public:
	// constructor for codes
	Token(Code code) : m_column{-1}, m_length{-1}, m_string{}, m_code{code},
		m_reference{}, m_subCode{} {}

	Token(TableEntry *entry, int column, int length,
		const std::string string = {}) : m_column{column}, m_length{length},
		m_string{string}, m_code{entry->code()}, m_reference{}, m_subCode{} {}

	// constructor for codes with operands
	Token(TableEntry *entry, DataType dataType, int column, int length,
		const std::string string = {}, bool reference = {},
		SubCode subCode = {}) : m_column{column}, m_length{length},
		m_string{string}, m_code{entry->alternate(dataType)->code()},
		m_reference{reference}, m_subCode{subCode} {}

	// constructor for integer constants
	Token(DataType dataType, int column, int length, const std::string string,
		int value) : m_column{column}, m_length{length}, m_string{string},
		m_code{Table::entry(Const_Code, dataType == DataType::Double
		? dataType : DataType::Integer)->code()}, m_reference{}, m_subCode{},
		m_value(value), /*convert*/ m_valueInt{value} {}

	// constructor for double constants
	Token(DataType dataType, int column, int length, const std::string string,
		double value);

	// constructor for string constants
	Token(int column, int length, const std::string string) :
		m_column{column}, m_length{length}, m_string{string},
		m_code{Table::entry(Const_Code, DataType::String)->code()},
		m_reference{}, m_subCode{} {}

	Token(const Token &token)  // copy constructor
	{
		*this = token;
	}
	~Token(void) {}

	bool operator==(const Token &other) const;
	bool operator!=(const Token &other) const
	{
		return !(*this == other);
	}

	// column and length access functions
	int column(void) const
	{
		return m_column;
	}
	int length(void) const
	{
		return m_length;
	}
	void setLength(int length)
	{
		m_length = length;
	}

	// table access functions
	TableEntry *table() const
	{
		return Table::entry(m_code);
	}
	Type type(void) const
	{
		return table()->type();
	}
	bool isType(Type type) const
	{
		return type == table()->type();
	}
	bool hasFlag(unsigned flag) const
	{
		return table()->hasFlag(flag);
	}
	int precedence() const
	{
		return table()->precedence();
	}

	// data type access functions
	DataType dataType() const
	{
		return table()->returnDataType();
	}
	bool isDataType(DataType dataType) const
	{
		return dataType == table()->returnDataType();
	}
	bool isDataTypeCompatible(DataType dataType)
	{
		// check if token data type is compatible with desired data type
		return dataType == table()->returnDataType()
			|| (dataType == DataType::Number
			&& table()->returnDataType() != DataType::String)
			|| dataType == DataType::Any || dataType == DataType::None;
	}

	// string access function
	std::string string(void) const
	{
		return m_string;
	}
	std::string stringWithDataType() const;
	void setString(const std::string &string)
	{
		m_string = string;
	}
	int stringLength(void) const
	{
		return m_string.length();
	}

	// code access functions
	Code code(void) const
	{
		return m_code;
	}
	void setCode(Code code)
	{
		m_code = code;
	}
	bool isCode(Code code) const
	{
		return code == m_code;
	}
	bool isLastOperand(int operandIndex)
	{
		return operandIndex == table()->operandCount() - 1;
	}
	void setFirstAlternate(int operandIndex)
	{
		m_code = table()->alternate(operandIndex)->code();
	}

	// reference access functions
	bool reference(void) const
	{
		return m_reference;
	}
	void setReference(bool reference = true)
	{
		m_reference = reference;
	}

	// sub-code access functions
	bool hasSubCode() const
	{
		return m_subCode;
	}
	bool hasSubCode(SubCode subCode) const
	{
		return m_subCode & subCode;
	}
	void addSubCode(SubCode subCode)
	{
		m_subCode |= subCode;
	}
	void removeSubCode(SubCode subCode)
	{
		m_subCode &= ~subCode;
	}
	uint16_t subCodes(void) const
	{
		return m_subCode;
	}

	// value access functions
	double value(void) const
	{
		return m_value;
	}
	int valueInt(void) const
	{
		return m_valueInt;
	}
	void setValue(double value)
	{
		m_value = value;
	}
	void setValue(int value)
	{
		m_valueInt = value;
	}

	// index access functions
	int index(void)
	{
		return m_index;
	}
	void setIndex(int index)
	{
		m_index = index;
	}

	// token information functions
	bool isNull(void) const
	{
		return !m_code;
	}

	// other functions
	TableEntry *convert(TokenPtr &operandToken, int operandIndex);
	TableEntry *convertCodeEntry(DataType dataType);
	void changeConstantIgnoreError(DataType toDataType) noexcept;
	bool changeConstant(DataType toDataType);

private:
	// instance members
	int m_column;			// start column of token
	int m_length;			// length of token
	std::string m_string;	// pointer to string of token
	Code m_code;			// internal code of token (index of TableEntry)
	bool m_reference;		// token is a reference flag
	uint16_t m_subCode;		// sub-code flags of token
	double m_value;			// double value for constant token
	int m_valueInt;			// integer value for constant token (if possible)
	int m_index;			// index within encoded program code line
};

using TokenPtr = std::shared_ptr<Token>;


// structure for holding information about an error exception
struct TokenError
{
	TokenError(Status status, int column, int length) : m_status {status},
		m_column {column}, m_length {length} {}
	TokenError(Status status, const TokenPtr &token) :
		TokenError {status, token->column(), token->length()} {}

	// convenience function for getting error status
	Status operator()()
	{
		return m_status;
	}

	// convenience function for checking for a particular error status
	bool operator()(Status status)
	{
		return status == m_status;
	}

	// convenience function for setting error status
	void operator=(Status status)
	{
		m_status = status;
	}

	Status m_status;						// status of error
	int m_column;							// column of error
	int m_length;							// length of error
};


#endif  // TOKEN_H
