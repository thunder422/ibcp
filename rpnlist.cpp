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
}


// function to recreate text (abbreviated contents) of list
QString RpnList::text()
{
	std::stringstream ss;
	std::unordered_map<RpnItemPtr, int> itemIndex;
	int index {};

	for (RpnItemPtr rpnItem : *this)
	{
		if (ss.tellp() > 0)
		{
			ss << ' ';
		}
		itemIndex[rpnItem] = index++;
		ss << rpnItem->token()->text();
		if (rpnItem->attachedCount() > 0)
		{
			char separator {'['};
			for (RpnItemPtr item : rpnItem->attached())
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
	auto otherIterator = other.begin();
	for (RpnItemPtr rpnItem : *this)
	{
		if (*rpnItem != **otherIterator)
		{
			return false;
		}
		++otherIterator;
	}
	return true;
}


// function to set program code size
//
//   - assigns position indexes to each token
//   - sets code size required for encoded line
//   - upon error returns token of code not yet implemented

bool RpnList::setCodeSize(Table &table, TokenPtr &token)
{
	// count number of program words needed
	m_codeSize = 0;
	for (RpnItemPtr rpnItem : *this)
	{
		// assign position index to tokens (increment count for instruction)
		token = rpnItem->token();
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
