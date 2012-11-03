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

#ifndef TOKEN_H
#define TOKEN_H

#include <QString>

#include "ibcp.h"


// changes to TokenType may require changes to ibcp.cpp: Token::initialize()
enum TokenType {
	Command_TokenType,
	Operator_TokenType,
	IntFuncN_TokenType,  // 2010-03-11: replaces IntFunc_TokenType
	IntFuncP_TokenType,  // 2010-03-11: replaces IntFunc_TokenType
	Remark_TokenType,
	Constant_TokenType,
	DefFuncN_TokenType,  // 2010-03-06: replaces DefFunc_TokenType
	DefFuncP_TokenType,  // 2010-03-06: replaces DefFunc_TokenType
	NoParen_TokenType,
	Paren_TokenType,
	Error_TokenType,
	sizeof_TokenType
};


struct Token {
	int column;				// start column of token
	int length;				// length of token (2011-01-11: moved from union)
	TokenType type;			// type of the token
	DataType datatype;		// data type of token
	QString string;			// pointer to string of token
	Code code;	 			// internal code of token (index of TableEntry)
	bool reference;			// token is a reference flag
	int subcode;			// sub-code flags of token
	union {
		double dbl_value;	// value for double constant token
		int int_value;		// value for integer constant token
	};
	Token(int col = -1)
	{
		column = col;
		length = 1;
		reference = false;
		subcode = None_SubCode;
	}
	~Token(void)
	{
	}
	void set_error(const QString &msg)
	{
		length = 1;
		type = Error_TokenType;
		datatype = None_DataType;
		string = msg;
	}
	void set_error(int col, const QString  &msg)
	{
		column = col;
		length = 1;
		type = Error_TokenType;
		datatype = None_DataType;
		string = msg;
	}
	void set_error(const QString &msg, int len)
	{
		length = len;
		type = Error_TokenType;
		datatype = None_DataType;
		string = msg;
	}
	void set_error(int col, const QString &msg, int len)
	{
		column = col;
		length = len;
		type = Error_TokenType;
		datatype = None_DataType;
		string = msg;
	}
	bool is_operator(void)
	{
		return op[type];
	}
	bool has_paren(void)
	{
		return paren[type];
	}
	int precedence(void)
	{
		return prec[type];
	}
	int table_entry(void)
	{
		return table[type];
	}
	Token *through(Token *token2)
	{
		length = token2->column - column + token2->length;
		return this;
	}
	bool isNull(void)
	{
		return table_entry() && code == Null_Code;
	}

	// static members
	static bool paren[sizeof_TokenType];
	static bool op[sizeof_TokenType];
	static int prec[sizeof_TokenType];
	static bool table[sizeof_TokenType];
	static const QString message_array[sizeof_TokenStatus];

	static void initialize(void);
	static const QString message(TokenStatus status)
	{
		return message_array[status];
	}
};


#endif  // TOKEN_H
