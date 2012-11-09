// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: token.h - token class definitions file
//	Copyright (C) 2012  Thunder422
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
//	2012-11-03	changed to class, renamed variables and functions to Qt style

#ifndef TOKEN_H
#define TOKEN_H

#include <QString>

#include "ibcp.h"


// changes to TokenType may require changes to ibcp.cpp: Token::initialize()
enum TokenType {
	Command_TokenType,
	Operator_TokenType,
	IntFuncN_TokenType,
	IntFuncP_TokenType,
	Remark_TokenType,
	Constant_TokenType,
	DefFuncN_TokenType,
	DefFuncP_TokenType,
	NoParen_TokenType,
	Paren_TokenType,
	Error_TokenType,
	sizeof_TokenType
};


class Token {
	int m_column;			// start column of token
	int m_length;			// length of token
	TokenType m_type;		// type of the token
	DataType m_dataType;	// data type of token
	QString m_string;		// pointer to string of token
	Code m_code;			// internal code of token (index of TableEntry)
	bool m_reference;		// token is a reference flag
	int m_subCode;			// sub-code flags of token
	union {
		double m_valueDbl;	// value for double constant token
		int m_valueInt;		// value for integer constant token
	};

public:
	Token(int column = -1)
	{
		m_column = column;
		m_length = 1;
		m_reference = false;
		m_subCode = None_SubCode;
	}
	~Token(void)
	{
	}

	// column and length access functions
	int column(void)
	{
		return m_column;
	}
	int length(void)
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
	TokenType type(void)
	{
		return m_type;
	}
	void setType(TokenType type)
	{
		m_type = type;
	}
	bool isType(TokenType type)
	{
		return type == m_type;
	}

	// data type access functions
	DataType dataType(void)
	{
		return m_dataType;
	}
	void setDataType(DataType dataType)
	{
		m_dataType = dataType;
	}
	bool isDataType(DataType dataType)
	{
		return dataType == m_dataType;
	}
	void setDataType(void);  // set default data type if not already set

	// string access function
	QString string(void)
	{
		return m_string;
	}
	void setString(const QString &string)
	{
		m_string = string;
	}
	void setString(int pos, QChar character)
	{
		m_string[pos] = character;
	}
	int stringLength(void)
	{
		return m_string.length();
	}

	// code access functions
	Code code(void)
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
	bool isCode(Code code)
	{
		return code == m_code;
	}

	// reference access functions
	bool reference(void)
	{
		return m_reference;
	}
	void setReference(bool reference = true)
	{
		m_reference = reference;
	}

	// sub-code access functions
	int subCode(void)
	{
		return m_subCode;
	}
	int isSubCode(int subCode)
	{
		return m_subCode & subCode;
	}
	void setSubCode(int subCode)
	{
		m_subCode = subCode;
	}
	void setSubCodeMask(int subCode)
	{
		m_subCode |= subCode;
	}
	void clearSubCodeMask(int subCode)
	{
		m_subCode &= ~subCode;
	}

	// value access functions
	double valueDbl(void)
	{
		return m_valueDbl;
	}
	int valueInt(void)
	{
		return m_valueInt;
	}
	void setValue(double value)
	{
		m_valueDbl = value;
	}
	void setValue(int value)
	{
		m_valueInt = value;
	}

	// set error functions
	void setError(const QString &msg)
	{
		m_length = 1;
		m_type = Error_TokenType;
		m_dataType = None_DataType;
		m_string = msg;
	}
	void setError(int column, const QString  &msg)
	{
		m_column = column;
		m_length = 1;
		m_type = Error_TokenType;
		m_dataType = None_DataType;
		m_string = msg;
	}
	void setError(const QString &msg, int len)
	{
		m_length = len;
		m_type = Error_TokenType;
		m_dataType = None_DataType;
		m_string = msg;
	}
	void setError(int column, const QString &msg, int len)
	{
		m_column = column;
		m_length = len;
		m_type = Error_TokenType;
		m_dataType = None_DataType;
		m_string = msg;
	}

	// token information functions
	bool isOperator(void)
	{
		return op[m_type];
	}
	bool hasParen(void)
	{
		return paren[m_type];
	}
	int precedence(void)
	{
		return prec[m_type];
	}
	int hasTableEntry(void)
	{
		return table[m_type];
	}
	bool isNull(void)
	{
		return hasTableEntry() && m_code == Null_Code;
	}

	// set length to include second token
	Token *setThrough(Token *token2)
	{
		m_length = token2->m_column - m_column + token2->m_length;
		return this;
	}

private:
	// static members
	static bool paren[sizeof_TokenType];
	static bool op[sizeof_TokenType];
	static int prec[sizeof_TokenType];
	static bool table[sizeof_TokenType];
	static const QString messageArray[sizeof_TokenStatus];

public:
	// static member functions
	static void initialize(void);
	static const QString message(TokenStatus status)
	{
		return messageArray[status];
	}
};


#endif  // TOKEN_H
