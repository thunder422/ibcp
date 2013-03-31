// vim:ts=4:sw=4:

//	Interactive BASIC Compiler Project
//	File: test_ibcp.cpp - contains code for testing
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


// function to recreate text (abbreviated contents) of item
QString RpnItem::text(void)
{
	QString string = token()->text();
	if (nOperands() > 0)
	{
		QChar separator('[');
		for (int i = 0; i < nOperands(); i++)
		{
			string += separator + operand(i)->token()->text();
			separator = ',';
		}
		string += ']';
	}
	return string;
}


RpnList::~RpnList(void)
{
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
	bool started = false;

	foreach (RpnItem *rpnItem, *this)
	{
		if (started)
		{
			string += ' ';
		}
		else
		{
			started = true;
		}
		string += rpnItem->text();
	}
	return string;
}


// end: rpn_list.cpp
