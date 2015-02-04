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
#include "programcode.h"
#include "table.h"


class Token
{
public:
	// constructor for table entries
	Token(Table *entry, int column = -1, int length = -1,
		const std::string string = {}) : m_column{column}, m_length{length},
		m_string{string}, m_entry{entry}, m_reference{}, m_subCode{} {}

	// constructor for codes with operands
	Token(Table *entry, DataType dataType, int column, int length,
		const std::string string = {}, bool reference = {},
		SubCode subCode = {}) : m_column{column}, m_length{length},
		m_string{string}, m_entry{entry->alternateForReturn(dataType)},
		m_reference{reference}, m_subCode{subCode} {}

	// constructor for integer constants
	Token(DataType dataType, int column, int length, const std::string string,
		int value) : m_column{column}, m_length{length}, m_string{string},
		m_entry{Table::entry(Code::Constant, dataType == DataType::Double
		? dataType : DataType::Integer)}, m_reference{}, m_subCode{},
		m_value(value), /*convert*/ m_valueInt{value} {}

	// constructor for double constants
	Token(DataType dataType, int column, int length, const std::string string,
		double value);

	// constructor for string constants
	Token(int column, int length, const std::string string) :
		m_column{column}, m_length{length}, m_string{string},
		m_entry{Table::entry(Code::Constant, DataType::String)},
		m_reference{}, m_subCode{} {}

	// constructor for program word
	Token(const ProgramModel *programUnit, ProgramLineReader &programLineReader);

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

	// access functions
	int column(void) const
	{
		return m_column;
	}
	int length(void) const
	{
		return m_length;
	}

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

	Table *tableEntry() const
	{
		return m_entry;
	}
	void setTableEntry(Table *entry)
	{
		m_entry = entry;
	}

	bool reference(void) const
	{
		return m_reference;
	}
	void setReference(bool reference = true)
	{
		m_reference = reference;
	}

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

	int offset(void)
	{
		return m_offset;
	}
	void setOffset(int offset)
	{
		m_offset = offset;
	}

	// table pass-thru access functions
	Code code(void) const
	{
		return m_entry->code();
	}
	bool isCode(Code code) const
	{
		return code == m_entry->code();
	}
	int index(void) const
	{
		return m_entry->index();
	}

	const std::string name() const
	{
		return m_entry->name();
	}
	const std::string name2() const
	{
		return m_entry->name2();
	}
	std::string commandName() const
	{
		return m_entry->commandName();
	}
	const std::string debugName() const
	{
		return m_entry->debugName();
	}
	const std::string optionName() const
	{
		return m_entry->optionName();
	}

	bool hasFlag(unsigned flag) const
	{
		return m_entry->hasFlag(flag);
	}
	bool isOperator() const
	{
		return m_entry->isOperator();
	}
	bool isCommand() const
	{
		return m_entry->isCommand();
	}
	bool isFunction() const
	{
		return m_entry->isFunction();
	}
	int precedence() const
	{
		return m_entry->precedence();
	}

	DataType dataType() const
	{
		return m_entry->returnDataType();
	}
	bool isDataType(DataType dataType) const
	{
		return dataType == m_entry->returnDataType();
	}
	bool isDataTypeCompatible(DataType dataType)
	{
		return dataType == m_entry->returnDataType()
			|| (dataType == DataType::Number
			&& m_entry->returnDataType() != DataType::String)
			|| dataType == DataType::Any || dataType == DataType::None;
	}
	int operandCount() const
	{
		return m_entry->operandCount();
	}
	DataType operandDataType(int operand) const
	{
		return m_entry->operandDataType(operand);
	}
	DataType expectedDataType() const
	{
		return m_entry->expectedDataType();
	}
	int lastOperand() const
	{
		return m_entry->lastOperand();
	}
	bool isLastOperand(int operand) const
	{
		return operand == lastOperand();
	}
	bool isUnaryOperator() const
	{
		return m_entry->isUnaryOperator();
	}
	bool isUnaryOrBinaryOperator() const
	{
		return m_entry->isUnaryOrBinaryOperator();
	}
	bool isCodeWithOperand() const
	{
		return m_entry->isCodeWithOperand();
	}

	int hasBinaryOperator() const
	{
		return m_entry->hasBinaryOperator();
	}
	Table *firstAlternate(int operand = FirstOperand) const
	{
		return m_entry->firstAlternate(operand);
	}
	void setToFirstAlternate(int operand)
	{
		m_entry = m_entry->firstAlternate(operand);
	}

	// other functions
	Table *convert(TokenPtr &operandToken, int operand);
	Table *convertCodeEntry(DataType dataType);
	void changeConstantIgnoreError(DataType toDataType) noexcept;
	bool changeConstant(DataType toDataType);
	void encode(ProgramModel *programUnit, ProgramCode::BackInserter backInserter);

private:
	// instance members
	int m_column;			// start column of token
	int m_length;			// length of token
	std::string m_string;	// pointer to string of token
	Table *m_entry;			// pointer to table entry of token
	bool m_reference;		// token is a reference flag
	uint16_t m_subCode;		// sub-code flags of token
	double m_value;			// double value for constant token
	int m_valueInt;			// integer value for constant token (if possible)
	int m_offset;			// index within encoded program code line
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
