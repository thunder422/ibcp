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


RpnList::~RpnList(void)
{
	clear();
}


// function to recreate text (abbreviated contents) of item
QString RpnItem::text(void)
{
	QString string = token()->text();
	if (nOperands() > 0)
	{
		QChar separator('[');
		for (int i = 0; i < nOperands(); i++)
		{
			string += separator + QString("%1:%2").arg(operand(i)->index())
				.arg(operand(i)->token()->text());
			separator = ',';
		}
		string += ']';
	}
	return string;
}


// function to overload the comparison operator
bool RpnItem::operator==(const RpnItem &other) const
{
	if (*m_token != *other.m_token || m_nOperands != other.m_nOperands)
	{
		return false;
	}
	for (int i = 0; i < m_nOperands; i++)
	{
		if (*m_operand[i]->token() != *other.m_operand[i]->token())
		{
			return false;
		}
	}
	return true;
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
QString RpnList::text(void)
{
	QString string;

	for (int i = 0; i < count(); i++)
	{
		if (i > 0)
		{
			string += ' ';
		}
		string += at(i)->text();
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


// end: rpnlist.cpp
