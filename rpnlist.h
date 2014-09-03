// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: rpnlist.h - reverse polish notation classes header file
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

#include <memory>
#include <vector>

#include <QList>

#include "token.h"

class Table;
class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;
using RpnItemPtrVector = std::vector<RpnItemPtr>;


// class for holding an item in the RPN output list
class RpnItem
{
public:
	RpnItem(Token *token, RpnItemPtrVector attached = RpnItemPtrVector{}) :
		m_token{token}, m_attached{attached}
	{
	}
	~RpnItem()
	{
		delete m_token;
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

	int attachedCount(void) const
	{
		return m_attached.size();
	}
	const RpnItemPtrVector attached() const
	{
		return m_attached;
	}

	bool operator==(const RpnItem &other) const
	{
		return *m_token == *other.m_token
			&& m_attached.size() == other.m_attached.size();
	}
	bool operator!=(const RpnItem &other) const
	{
		return !(*this == other);
	}

private:
	Token *m_token;					// pointer to token
	RpnItemPtrVector m_attached;	// array of attached item pointers
};


// class for holding a list of RPN items
class RpnList : public QList<RpnItemPtr>
{
public:
	RpnList(void) : m_errorColumn(-1), m_errorLength(-1) {}
	~RpnList(void);
	QString text();
	bool operator==(const RpnList &other) const;
	bool operator!=(const RpnList &other) const
	{
		return !(*this == other);
	}

	RpnItemPtr append(Token *token,
		RpnItemPtrVector attached = RpnItemPtrVector{});
	void insert(int index, Token *token);
	int codeSize(void)
	{
		return m_codeSize;
	}
	bool setCodeSize(Table &table, Token *&token);

	void setError(Token *errorToken)
	{
		m_errorColumn = errorToken->column();
		m_errorLength = errorToken->length();
	}
	bool hasError(void) const
	{
		return m_errorColumn != -1;
	}
	int errorColumn(void) const
	{
		return m_errorColumn;
	}
	int errorLength(void) const
	{
		return m_errorLength;
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
	int m_codeSize;					// size of code required for list
	int m_errorColumn;				// column of error that occurred
	int m_errorLength;				// length of error that occurred
	QString m_errorMessage;			// message of error that occurred
};


#endif // RPNLIST_H
