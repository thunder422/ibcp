// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: errorlist.h - error classes source file
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
//	2013-04-10	initial version

#include "errorlist.h"
#include "rpnlist.h"


// constructor function to set error item
ErrorItem::ErrorItem(Type type, int lineNumber, RpnList *rpnList)
{
	m_type = type;
	m_lineNumber = lineNumber;
	m_column = rpnList->errorColumn();
	m_length = rpnList->errorLength();
	m_message = rpnList->errorMessage();
}


ErrorList::ErrorList(void)
{
}


// function to look for an error by line number
//
//   - uses a binary search to find the line number
//   - returns the index of the line number in the error list
//   - returns the index of the closet error if the line number
//     specified does not have an error
//   - returns one past end of list if line number higher than last line number

int ErrorList::find(int lineNumber) const
{
	int lo = 0;
	int hi = count();
	int mid;

	while ((mid = lo + (hi - lo) / 2) < hi)
	{
		int match = lineNumber - at(mid).lineNumber();
		if (match == 0)
		{
			break;
		}
		else if (match < 0)
		{
			hi = mid;
		}
		else
		{
			lo = mid + 1;
		}
	}
	return mid;
}


// overloaded function for inserting an error into the list
void ErrorList::insert(int index, const ErrorItem &value)
{
	QList<ErrorItem>::insert(index, value);
	setChangeIndex(index);
}


// overloaded function for removing an error from the list
void ErrorList::removeAt(int index)
{
	QList<ErrorItem>::removeAt(index);
	setChangeIndex(index);
}


// overloaded function for replacing an error in the list
void ErrorList::replace(int index, const ErrorItem &value)
{
	QList<ErrorItem>::replace(index, value);
	setChangeIndex(index);
}


// function to increment line number of an error in the list
void ErrorList::incrementLineNumber(int index)
{
	(*this)[index].incrementLineNumber();
	setChangeIndex(index);
}


// function to decrement line number of an error in the list
void ErrorList::decrementLineNumber(int index)
{
	(*this)[index].decrementLineNumber();
	setChangeIndex(index);
}


// function to the change index if not already set
void ErrorList::setChangeIndex(int index)
{
	if (m_changeIndexStart == -1)
	{
		m_changeIndexStart = m_changeIndexEnd = index;
	}
	else if (m_changeIndexStart > index)
	{
		m_changeIndexStart = index;
	}
	else if (m_changeIndexEnd < index)
	{
		m_changeIndexEnd = index;
	}
}


// end: errorlist.cpp
