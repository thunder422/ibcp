// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: rpnlist.h - reverse polish notation list definitions file
//	Copyright (C) 2013  Thunder422
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
//	2013-03-26	initial version (parts removed from translator.h)

#ifndef RPNLIST_H
#define RPNLIST_H

#include <QList>

#include "token.h"


// class for holding an item in the RPN output list
class RpnItem
{
	Token *m_token;				// pointer to token
	int m_nOperands;			// number of operands
	RpnItem **m_operand;		// array of operand pointers

public:
	RpnItem(Token *token, int nOperands = 0, RpnItem **operand = NULL)
	{
		m_token = token;
		m_nOperands = nOperands;
		m_operand = operand;
	}
	~RpnItem()
	{
		delete m_token;
		if (m_nOperands > 0)
		{
			delete[] m_operand;
		}
	}

	// access functions
	Token *token(void)
	{
		return m_token;
	}
	void setToken(Token *token)
	{
		m_token = token;
	}

	int nOperands(void)
	{
		return m_nOperands;
	}
	void setNOperands(int nOperands)
	{
		m_nOperands = nOperands;
	}

	RpnItem **operand(void)
	{
		return m_operand;
	}
	void setOperand(RpnItem **operand)
	{
		m_operand = operand;
	}
	RpnItem *operand(int index)
	{
		return m_operand[index];
	}
	void setOperand(int index, RpnItem *operand)
	{
		m_operand[index] = operand;
	}
	QString text(void);

	// function to set operands without allocating a new array
	void set(int nOperands, RpnItem **operand)
	{
		m_nOperands = nOperands;
		m_operand = operand;
	}
};


// class for holding a list of RPN items
class RpnList : public QList<RpnItem *>
{
public:
	RpnList(void) : m_errorToken(NULL) {}
	~RpnList(void);
	void clear(void);
	QString text(void);

	void setErrorToken(Token *errorToken)
	{
		delete m_errorToken;
		m_errorToken = errorToken;
	}
	Token *errorToken(void) const
	{
		return m_errorToken;
	}
	bool hasError(void) const
	{
		return m_errorToken != NULL;
	}

	void setErrorMessage(QString errorMessage)
	{
		m_errorMessage = errorMessage;
	}
	QString errorMessage(void) const
	{
		return m_errorMessage;
	}

private:
	Token *m_errorToken;			// token when error occurred
	QString m_errorMessage;			// message of error that occurred
};


#endif // RPNLIST_H
