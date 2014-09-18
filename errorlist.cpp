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


// function to look for an error by line number
//
//   - uses a binary search to find the line number
//   - returns the index of the line number in the error list
//   - returns the index of the closet error if the line number
//     specified does not have an error
//   - returns one past end of list if line number higher than last line number

int ErrorList::find(int lineNumber) const
{
	int lo {0};
	int hi {count()};
	int mid;

	while ((mid = lo + (hi - lo) / 2) < hi)
	{
		int match {lineNumber - at(mid).lineNumber()};
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


// function to find the index of the error for a line number
//
//   - returns index of error in list for line number
//   - returns -1 if the line number does not have an error

int ErrorList::findIndex(int lineNumber) const
{
	int errIndex {find(lineNumber)};
	return errIndex < size() && lineNumber == at(errIndex).lineNumber()
		? errIndex : -1;
}


// overloaded function for inserting an error into the list
void ErrorList::insert(int index, const ErrorItem &value)
{
	QList<ErrorItem>::insert(index, value);
	m_changed = true;
}


// overloaded function for removing an error from the list
void ErrorList::removeAt(int index)
{
	QList<ErrorItem>::removeAt(index);
	m_changed = true;
}


// overloaded function for replacing an error in the list
void ErrorList::replace(int index, const ErrorItem &value)
{
	QList<ErrorItem>::replace(index, value);
	m_changed = true;
}


// function to increment line number of an error in the list
void ErrorList::incrementLineNumber(int index)
{
	(*this)[index].incrementLineNumber();
	m_changed = true;
}


// function to decrement line number of an error in the list
void ErrorList::decrementLineNumber(int index)
{
	(*this)[index].decrementLineNumber();
	m_changed = true;
}


// function to move the error due to inserted or deleted characters
void ErrorList::moveColumn(int index, int chars)
{
	(*this)[index].moveColumn(chars);
	m_changed = true;
}


// end: errorlist.cpp
