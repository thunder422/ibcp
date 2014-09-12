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

#include <list>
#include <memory>
#include <vector>

#include "token.h"

class Table;
class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;
using RpnItemVector = std::vector<RpnItemPtr>;
using RpnItemList = std::list<RpnItemPtr>;


// class for holding an item in the RPN output list
class RpnItem
{
public:
	RpnItem(TokenPtr token, RpnItemVector attached = RpnItemVector{}) :
		m_token{token}, m_attached{attached}
	{
	}
	~RpnItem()
	{
		delete m_token;
	}

	// access functions
	TokenPtr token(void)
	{
		return m_token;
	}
	void setToken(TokenPtr token)
	{
		m_token = token;
	}

	int attachedCount(void) const
	{
		return m_attached.size();
	}
	const RpnItemVector attached() const
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
	TokenPtr m_token;				// pointer to token
	RpnItemVector m_attached;		// array of attached item pointers
};


// class for holding a list of RPN items
class RpnList
{
public:
	RpnList(void) : m_errorColumn{-1}, m_errorLength{-1} {}
	~RpnList(void);
	RpnList(RpnList &&other) :
		m_list{other.m_list},
		m_codeSize{other.m_codeSize},
		m_errorColumn{other.m_errorColumn},
		m_errorLength{other.m_errorLength},
		m_errorMessage{other.m_errorMessage}
	{
		other.m_list.clear();
		other.m_codeSize = 0;
		other.m_errorColumn = -1;
		other.m_errorLength = -1;
		other.m_errorMessage.clear();
	}
	RpnList &operator=(RpnList &&other)
	{
		std::swap(m_list, other.m_list);
		std::swap(m_codeSize, other.m_codeSize);
		std::swap(m_errorColumn, other.m_errorColumn);
		std::swap(m_errorLength, other.m_errorLength);
		std::swap(m_errorMessage, other.m_errorMessage);
		return *this;
	}

	QString text();
	bool operator==(const RpnList &other) const;
	bool operator!=(const RpnList &other) const
	{
		return !(*this == other);
	}

	RpnItemList::const_iterator begin() const
	{
		return m_list.begin();
	}
	RpnItemList::const_iterator end() const
	{
		return m_list.end();
	}
	int count() const
	{
		return m_list.size();
	}
	int empty() const
	{
		return m_list.empty();
	}
	TokenPtr lastToken(void) const
	{
		return m_list.back()->token();
	}
	void clear()
	{
		m_list.clear();
	}

	RpnItemPtr append(TokenPtr token, RpnItemVector attached = RpnItemVector{});
	RpnItemList::iterator insert(RpnItemList::iterator iterator, TokenPtr token)
	{
		return m_list.emplace(iterator, RpnItemPtr{new RpnItem{token}});
	}
	RpnItemList::iterator appendIterator(TokenPtr token)
	{
		token->removeSubCode(UnUsed_SubCode);  // mark as used
		return insert(m_list.end(), token);
	}
	int codeSize(void) const
	{
		return m_codeSize;
	}
	bool setCodeSize(Table &table, TokenPtr &token);

	void setError(TokenPtr errorToken)
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
	RpnItemList m_list;				// list of rpn items
	int m_codeSize;					// size of code required for list
	int m_errorColumn;				// column of error that occurred
	int m_errorLength;				// length of error that occurred
	QString m_errorMessage;			// message of error that occurred
};


#endif // RPNLIST_H
