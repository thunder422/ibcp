// vim:ts=4:sw=4:
//
//	Interactive BASIC Compiler Project
//	File: donestack.h - translator done stack class header file
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
//	2013-07-21	initial version (parts removed from translator.h)

#ifndef DONESTACK_H
#define DONESTACK_H

#include <memory>

#include <QStack>

class RpnItem;
using RpnItemPtr = std::shared_ptr<RpnItem>;
class Token;


class DoneItem
{
public:
	RpnItemPtr rpnItem;			// pointer to RPN item
	Token *first;				// operator token's first operand pointer
	Token *last;				// operator token's last operand pointer

	~DoneItem()
	{
		deleteOpenParen(first);
		deleteCloseParen(last);
	}

	// delete token if is an open parentheses
	static void deleteOpenParen(Token *token);
	void deleteOpenParen(void)
	{
		deleteOpenParen(first);
	}

	// delete token if is a close parentheses
	static void deleteCloseParen(Token *token);
	void deleteCloseParen(void)
	{
		deleteCloseParen(last);
	}

	// replace the item's first and last operand token
	void replaceFirstLast(Token *_first, Token *_last)
	{
		deleteOpenParen(first);
		deleteCloseParen(last);
		first = _first;
		last = _last;
	}
};


class DoneStack : public QStack<DoneItem>
{
public:
	// push new item with rpn item, first and last tokens
	void push(RpnItemPtr rpnItem, Token *first = NULL, Token *last = NULL)
	{
		resize(size() + 1);
		top().rpnItem = rpnItem;
		top().first = first;
		top().last = last;
	}

	// pop the top item on stack (return its rpn item)
	// (delete any parentheses that are present in the first/last tokens)
	RpnItemPtr pop(void)
	{
		RpnItemPtr rpnItem = top().rpnItem;
		drop();
		return rpnItem;
	}

	// drop the top item on stack (pop with no return)
	// (delete any parentheses that are present in the first/last tokens)
	void drop(void)
	{
		resize(size() - 1);
	}
};


#endif  // DONESTACK_H
