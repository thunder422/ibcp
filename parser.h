// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: parser.h - parser class header file
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

#ifndef PARSER_H
#define PARSER_H

#include <QString>

#include "token.h"

class Table;


class Parser
{
public:
	explicit Parser(const QString &input);
	enum class Number {
		No,
		Yes
	};
	TokenPtr operator()(Number state);
	Status errorStatus()
	{
		return m_errorStatus;
	}

private:
	// main functions
	bool getCommand(void);
	bool getIdentifier(void);
	bool getNumber(void);
	bool getString(void);
	bool getOperator(void);

	// set token error function
	void setError(Status status)
	{
		m_errorStatus = status;
		m_token->setType(Token::Type::Error);
		m_token->setLength(1);
	}
	void setErrorColumn(Status status, int column)
	{
		m_errorStatus = status;
		m_token->setType(Token::Type::Error);
		// assumes length=1, specifies alternate column
		m_token->setLength(-column);
	}
	void setErrorLength(Status status, int len)
	{
		m_errorStatus = status;
		m_token->setType(Token::Type::Error);
		m_token->setLength(len);
	}

	// support functions
	void skipWhitespace();
	int scanWord(int pos, DataType &datatype, bool &paren);

	Table &m_table;			// pointer to the table object
	QString m_input;		// input line being parsed
	int m_pos;				// index to current position in input string
	TokenPtr m_token;		// pointer to working token (to be returned)
	Status m_errorStatus;	// status code of last detected error
};


#endif  // PARSER_H
