// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: token.h - token class header file
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
//	2012-11-02	initial version (parts removed from ibcp.h)

#ifndef TOKEN_H
#define TOKEN_H

#include <memory>
#include <string>
#include <unordered_map>

#include <QString>

#include "ibcp.h"


class Token;
using TokenUniquePtr = std::unique_ptr<Token>;
using TokenPtr = std::shared_ptr<Token>;


class Token
{
public:
	// changes to Type may require changes to initializers
	// for s_hasParen and s_precedence maps
	enum class Type
	{
		Command = 1,
		Operator,
		IntFuncN,
		IntFuncP,
		Constant,
		DefFuncN,
		DefFuncP,
		NoParen,
		Paren
	};

	explicit Token(int column = -1, int length = 1) : m_column{column},
		m_length{length}, m_type{}, m_code{Invalid_Code}, m_reference{},
		m_subCode{None_SubCode} {}

	// constructor for codes
	Token(int column, int length, Type type, DataType dataType, Code code,
		const std::string string = {}) : m_column{column}, m_length{length},
		m_type{type}, m_dataType{dataType}, m_string{string.c_str()},
		m_code{code}, m_reference{}, m_subCode{None_SubCode} {}

	// constructor for identifiers
	Token(int column, int length, Type type, DataType dataType,
		const QString &inputString) : m_column{column}, m_length{length},
		m_type{type}, m_dataType{dataType}, m_string{inputString.mid(column,
		length)}, m_code{Invalid_Code}, m_reference{}, m_subCode{None_SubCode}
		{}

	// constructor for integer constants
	Token(int column, int length, const std::string string, int value) :
		m_column{column}, m_length{length}, m_type{Token::Type::Constant},
		m_dataType{DataType::Integer}, m_string{string.c_str()},
		m_code{Invalid_Code}, m_reference{}, m_subCode{None_SubCode},
		m_valueInt{value}
	{
		m_value = value;  // convert to double in case needed
	}

	// constructor for double constants
	Token(int column, int length, const std::string string, double value,
		bool decimal);

	// constructor for string constants
	Token(int column, int length, const std::string string) : m_column{column},
		m_length{length}, m_type{Token::Type::Constant},
		m_dataType{DataType::String}, m_string{string.c_str()},
		m_code{Invalid_Code}, m_reference{}, m_subCode{None_SubCode} {}

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
	void addLengthToColumn(void)
	{
		m_column += m_length;
	}

	// type access functions
	Type type(void) const
	{
		return m_type;
	}
	void setType(Type type)
	{
		m_type = type;
	}
	bool isType(Type type) const
	{
		return type == m_type;
	}

	// data type access functions
	DataType dataType(bool actual = false) const
	{
		return !actual && hasSubCode(Double_SubCode)
			? DataType::Double : m_dataType;
	}
	void setDataType(DataType dataType)
	{
		m_dataType = dataType;
	}
	bool isDataType(DataType dataType) const
	{
		return dataType == m_dataType;
	}
	void setDataType(void);  // set default data type if not already set

	// string access function
	QString string(void) const
	{
		return m_string;
	}
	void setString(const QString &string)
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
	Code nextCode(void)
	{
		return ++m_code;
	}
	bool isCode(Code code) const
	{
		return code == m_code;
	}
	bool hasValidCode(void) const
	{
		return m_code != Invalid_Code;
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
	bool hasSubCode(int subCode) const
	{
		return m_subCode & subCode;
	}
	void addSubCode(int subCode)
	{
		m_subCode |= subCode;
	}
	void removeSubCode(int subCode)
	{
		m_subCode &= ~subCode;
	}
	int subCodes(void) const
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
	bool hasParen(void) const
	{
		return s_hasParen[m_type];
	}
	int precedence(void) const
	{
		return s_precendence[m_type];
	}
	bool isNull(void) const
	{
		return m_code == Null_Code;
	}

	// set length to include second token
	void setThrough(const TokenPtr &token2)
	{
		m_length = token2->m_column - m_column + token2->m_length;
	}

	// other functions
	Code convertCode(DataType toDataType) const;

private:
	// static members
	static std::unordered_map<Type, bool, EnumClassHash> s_hasParen;
	static std::unordered_map<Type, int, EnumClassHash> s_precendence;

	// instance members
	int m_column;			// start column of token
	int m_length;			// length of token
	Type m_type;			// type of the token
	DataType m_dataType;	// data type of token
	QString m_string;		// pointer to string of token
	Code m_code;			// internal code of token (index of TableEntry)
	bool m_reference;		// token is a reference flag
	int m_subCode;			// sub-code flags of token
	double m_value;			// double value for constant token
	int m_valueInt;			// integer value for constant token (if possible)
	int m_index;			// index within encoded program code line
};


#endif  // TOKEN_H
