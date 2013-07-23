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

#include <QStack>

class RpnItem;
class Token;


class DoneItem
{
public:
	RpnItem *rpnItem;			// pointer to RPN item
	Token *first;				// operator token's first operand pointer
	Token *last;				// operator token's last operand pointer

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

	// replace first token (delete old token first if open parentheses)
	void replaceFirst(Token *token)
	{
		deleteOpenParen(first);
		first = token;
	}

	// replace last token (delete old token first if close parentheses)
	void replaceLast(Token *token)
	{
		deleteCloseParen(last);
		last = token;
	}
};


class DoneStack : public QStack<DoneItem>
{
public:
	// push new item with rpn item, first and last tokens
	void push(RpnItem *rpnItem, Token *first = NULL, Token *last = NULL)
	{
		resize(size() + 1);
		top().rpnItem = rpnItem;
		top().first = first;
		top().last = last;
	}

	// pop the top item on stack (return its rpn item)
	// (delete any parentheses that are present in the first/last tokens)
	RpnItem *pop(void)
	{
		top().deleteOpenParen();
		top().deleteCloseParen();
		return QStack::pop().rpnItem;
	}

	// drop the top item on stack (pop with no return)
	// (delete any parentheses that are present in the first/last tokens)
	void drop(void)
	{
		top().deleteOpenParen();
		top().deleteCloseParen();
		resize(size() - 1);
	}

	// replace the top item's first and last operand token
	void replaceTopFirstLast(Token *first, Token *last)
	{
		top().replaceFirst(first);
		top().replaceLast(last);
	}
};


#endif  // DONESTACK_H
