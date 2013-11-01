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

#include "rpnlist.h"
#include "table.h"


RpnList::~RpnList(void)
{
	clear();
}


// function to recreate text (abbreviated contents) of item
QString RpnItem::text(bool withIndexes)
{
	QString string = token()->text(withIndexes);
	if (attachedCount() > 0)
	{
		QChar separator('[');
		for (int i = 0; i < attachedCount(); i++)
		{
			string += separator + QString("%1:%2").arg(attached(i)->index())
				.arg(attached(i)->token()->text());
			separator = ',';
		}
		string += ']';
	}
	return string;
}


// function to clear all of the RPN items from the list
void RpnList::clear(void)
{
	while (!isEmpty())
	{
		// delete to free the RPN item that was in the list
		delete takeLast();
	}
}


// function to recreate text (abbreviated contents) of item
QString RpnList::text(bool withIndexes)
{
	QString string;

	for (int i = 0; i < count(); i++)
	{
		if (i > 0)
		{
			string += ' ';
		}
		string += at(i)->text(withIndexes);
	}
	return string;
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

RpnItem *RpnList::append(Token *token, int attachedCount, RpnItem **attached)
{
	token->removeSubCode(UnUsed_SubCode);  // mark as used
	RpnItem *rpnItem = new RpnItem(token, attachedCount, attached);
	rpnItem->setIndex(count());
	QList<RpnItem *>::append(rpnItem);
	return rpnItem;
}

// function to create an rpn item for a token and insert it into the list
//
//   - the indexes of all items after the insertion point are incremented

void RpnList::insert(int index, Token *token)
{
	QList<RpnItem *>::insert(index, new RpnItem(token));
	// update indexes of all list items after insert point
	while (++index < count())
	{
		at(index)->incrementIndex();
	}
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
