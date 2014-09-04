// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: rpnlist.cpp - reverse polish notation classes source file
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
//	2013-03-30	initial version

#include <sstream>
#include <unordered_map>

#include "rpnlist.h"
#include "table.h"


RpnList::~RpnList(void)
{
	clear();
}


// function to recreate text (abbreviated contents) of list
QString RpnList::text()
{
	std::stringstream ss;
	std::unordered_map<RpnItemPtr, int> itemIndex;
	int index {};

	for (int i = 0; i < count(); i++)
	{
		if (i > 0)
		{
			ss << ' ';
		}
		RpnItemPtr rpnItem = at(i);
		itemIndex[rpnItem] = index++;
		ss << rpnItem->token()->text();
		if (rpnItem->attachedCount() > 0)
		{
			char separator {'['};
			for (auto item : rpnItem->attached())
			{
				if (item)
				{
					ss << separator << itemIndex[item] << ':'
						<< item->token()->text();
					separator = ',';
				}
			}
			if (separator != '[')
			{
				ss << ']';
			}
		}
	}
	return QString::fromStdString(ss.str());
}

// function to overload the comparison operator
bool RpnList::operator==(const RpnList &other) const
{
	if (hasError() || other.hasError())
	{
		return false;  // miscompare if either list has an error
	}
	if (count() != other.count())
	{
		return false;  // miscompare if lists are different sizes
	}
	for (int i = 0; i < count(); i++)
	{
		if (*at(i) != *other.at(i))
		{
			return false;
		}
	}
	return true;
}


// function to create an rpn item for a token and append it to the list

RpnItemPtr RpnList::append(Token *token, RpnItemPtrVector attached)
{
	token->removeSubCode(UnUsed_SubCode);  // mark as used
	RpnItemPtr rpnItem = RpnItemPtr{new RpnItem(token, attached)};
	QList<RpnItemPtr>::append(rpnItem);
	return rpnItem;
}

// function to create an rpn item for a token and insert it into the list
//
//   - the indexes of all items after the insertion point are incremented

void RpnList::insert(int index, Token *token)
{
	QList<RpnItemPtr>::insert(index, RpnItemPtr(new RpnItem(token)));
}


// function to set program code size
//
//   - assigns position indexes to each token
//   - sets code size required for encoded line
//   - upon error returns token of code not yet implemented

bool RpnList::setCodeSize(Table &table, Token *&token)
{
	// count number of program words needed
	m_codeSize = 0;
	for (int i = 0; i < count(); i++)
	{
		// assign position index to tokens (increment count for instruction)
		token = at(i)->token();
		if (!token->hasValidCode())
		{
			return false;
		}
		token->setIndex(m_codeSize++);
		if (table.hasOperand(token->code()))
		{
			m_codeSize++;  // increment count for operand
		}
	}
	return true;
}


// end: rpnlist.cpp
